#define _CRT_SECURE_NO_WARNINGS
#include"HIS_System.h"
#include"QueueManage.h"
#include"ConfirmFunc.h"
#include"InputUtils.h"
#include"PauseUtil.h"
#include"DayTimeUtils.h"
#include"PrintFormattedStr.h"
#include<string.h>

// 全局链表：医生排班、挂号单、候诊队列
static DoctorDaySchedule* g_scheduleHead = NULL;
static QueueTicket* g_ticketHead = NULL;

// 供外部判断全局排班数据是否已就绪
bool hasScheduleData(void) {
	return g_scheduleHead != NULL;
}
static WaitingQueue* g_waitingQueueHead = NULL;
static int g_signSeq = 1;
static int g_priorityCounter = 0;	// 医生优先标记自增序号

// 根据患者类别返回优先级数值，急诊最高，VIP与普通同级
static int patientPriority(const Patient* patient) {
	if (patient == NULL) {
		return 0;
	}
	if (patient->type == PATIENT_EMERGENCY) {
		return 2;
	}
	return 1;	// VIP 与普通患者权重相同
}

Patient* findPatientByIdInQueue(HIS_System* sys, const char* patientId) {
	Patient* curr = sys->patientHead;
	while (curr != NULL) {
		if (strcmp(curr->patientId, patientId) == 0) {
			return curr;
		}
		curr = curr->next;
	}
	return NULL;
}

doctor* findDoctorByIdInQueue(HIS_System* sys, const char* doctorId) {
	doctor* curr = sys->docHead;
	while (curr != NULL) {
		if (strcmp(curr->doctorId, doctorId) == 0) {
			return curr;
		}
		curr = curr->next;
	}
	return NULL;
}

// 比较函数：先按优先级比较，优先级相同则按签到顺序比较（签到越早signSeq越小）
static int comparePriorityThenSign(const void* lhs, const void* rhs) {
	const QueueTicket* a = *(const QueueTicket**)lhs;
	const QueueTicket* b = *(const QueueTicket**)rhs;
	int pa = patientPriority(a->patient);
	int pb = patientPriority(b->patient);
	if (pa != pb) {
		return pb - pa;
	}
	return a->signSeq - b->signSeq;
}

// 比较函数：仅按签到顺序比较（签到越早signSeq越小）
static int compareSignOnly(const void* lhs, const void* rhs) {
	const QueueTicket* a = *(const QueueTicket**)lhs;
	const QueueTicket* b = *(const QueueTicket**)rhs;
	return a->signSeq - b->signSeq;
}

// 三级比较函数（供就诊中患者列表使用）：
// 第一级：患者类型（急诊 > VIP > 普通）降序
// 第二级：priorityOrder 降序（越大越靠前，0=从未标记排末尾）
// 第三级：signSeq 升序（先签到在前）
static int compareConsultingPriority(const void* lhs, const void* rhs) {
	const QueueTicket* a = *(const QueueTicket**)lhs;
	const QueueTicket* b = *(const QueueTicket**)rhs;
	int pa = patientPriority(a->patient);
	int pb = patientPriority(b->patient);
	if (pa != pb) return pb - pa;
	if (a->priorityOrder != b->priorityOrder) return b->priorityOrder - a->priorityOrder;
	return a->signSeq - b->signSeq;
}

// 重建所有候诊队列：遍历全部挂号单，对每个有效的(医生,日期,时段)组合调用refreshSlotQueue
// 确保系统重启后医生叫号功能可以正常使用
static void rebuildAllWaitingQueues(void) {
	// 用于记录已经重建过的 (doctorId, date, slot) 组合，避免重复调用refreshSlotQueue
	typedef struct {
		char doctorId[ID_LEN];
		char date[DATE_STR_LEN];
		TimeSlot slot;
	} RebuildKey;
	RebuildKey rebuilt[256];
	int rebuiltCount = 0;

	QueueTicket* curr = g_ticketHead;
	while (curr != NULL) {
		// 只重建有已签到且状态为等待中或已叫号患者的队列
		if (curr->checkedIn && (curr->status == STATUS_WAITING || curr->status == STATUS_CALLED)) {
			bool alreadyRebuilt = false;
			for (int i = 0; i < rebuiltCount; ++i) {
				if (strcmp(rebuilt[i].doctorId, curr->doctor->doctorId) == 0 &&
					strcmp(rebuilt[i].date, curr->date) == 0 &&
					rebuilt[i].slot == curr->slot) {
					alreadyRebuilt = true;
					break;
				}
			}
			if (!alreadyRebuilt && rebuiltCount < 256) {
				strcpy(rebuilt[rebuiltCount].doctorId, curr->doctor->doctorId);
				strcpy(rebuilt[rebuiltCount].date, curr->date);
				rebuilt[rebuiltCount].slot = curr->slot;
				rebuiltCount++;
				refreshSlotQueue(curr->doctor->doctorId, curr->date, curr->slot);
			}
		}
		curr = curr->next;
	}
	if (rebuiltCount > 0 && TEST_SYSTEM_DEBUG) {
		printf(">>> 候诊队列重建完成，共重建 %d 个队列。\n", rebuiltCount);
	}
}

// 获取时间段的开始分钟数（相对于当天0点），如8:00对应480，8:30对应510，以此类推
static int slotStartMinute(TimeSlot slot) {
	static const int starts[SLOT_COUNT + 2] = {
		0,
		8 * 60 + 0,    // SLOT_0800_0830 = 1
		8 * 60 + 30,   // SLOT_0830_0900 = 2
		9 * 60 + 0,    // SLOT_0900_0930 = 3
		9 * 60 + 30,   // SLOT_0930_1000 = 4
		10 * 60 + 0,   // SLOT_1000_1030 = 5
		10 * 60 + 30,  // SLOT_1030_1100 = 6
		11 * 60 + 0,   // SLOT_1100_1130 = 7
		11 * 60 + 30,  // SLOT_1130_1200 = 8
		12 * 60 + 0,   // SLOT_1200_1230 = 9
		12 * 60 + 30,  // SLOT_1230_1300 = 10
		13 * 60 + 0,   // SLOT_1300_1330 = 11
		13 * 60 + 30,  // SLOT_1330_1400 = 12
		14 * 60 + 0,   // SLOT_1400_1430 = 13
		14 * 60 + 30,  // SLOT_1430_1500 = 14
		15 * 60 + 0,   // SLOT_1500_1530 = 15
		15 * 60 + 30,  // SLOT_1530_1600 = 16
		16 * 60 + 0,   // SLOT_1600_1630 = 17
		16 * 60 + 30   // SLOT_NIGHT = 18
	};
	if (slot <= SLOT_INVALID || slot > SLOT_NIGHT) {
		return -1;
	}
	return starts[slot];
}

// 将时间字符串（格式为"HH:MM"）转换为当天的分钟数，返回-1表示格式错误或时间无效
static int parseMinuteOfDay(const char* timeStr) {
	if (timeStr == NULL || strlen(timeStr) < 5) {
		return -1;
	}
	int hour = -1;
	int min = -1;
	if (sscanf(timeStr, "%d:%d", &hour, &min) != 2) {
		return -1;
	}
	if (hour < 0 || hour > 23 || min < 0 || min > 59) {
		return -1;
	}
	return hour * 60 + min;
}

// 获取医生某天的排班信息，如果createIfMissing为true且不存在则创建新记录
static DoctorDaySchedule* getSchedule(const char* doctorId, const char* date, bool createIfMissing) {
	DoctorDaySchedule* curr = g_scheduleHead;
	while (curr != NULL) {
		if (strcmp(curr->doctorId, doctorId) == 0 && strcmp(curr->date, date) == 0) {
			return curr;
		}
		curr = curr->next;
	}
	if (!createIfMissing) {
		return NULL;
	}
	DoctorDaySchedule* created = (DoctorDaySchedule*)malloc(sizeof(DoctorDaySchedule));
	if (created == NULL) {
		return NULL;
	}
	strcpy(created->doctorId, doctorId);
	strcpy(created->date, date);
	for (int i = 0; i <= SLOT_COUNT + 1; ++i) {
		created->openSlots[i] = false;
		created->bookingCount[i] = 0;
	}
	created->next = g_scheduleHead;
	g_scheduleHead = created;
	return created;
}

// 获取医生某天某时段的候诊队列，如果createIfMissing为true且不存在则创建新记录
static WaitingQueue* getWaitingQueue(const char* doctorId, const char* date, TimeSlot slot, bool createIfMissing) {
	WaitingQueue* curr = g_waitingQueueHead;	//遍历链表查找匹配的候诊队列
	while (curr != NULL) {
		if (strcmp(curr->doctorId, doctorId) == 0 && strcmp(curr->date, date) == 0 && curr->slot == slot) {
			return curr;
		}
		curr = curr->next;
	}
	if (!createIfMissing) {
		return NULL;
	}
	WaitingQueue* created = (WaitingQueue*)malloc(sizeof(WaitingQueue));
	if (created == NULL) {
		return NULL;
	}
	strcpy(created->doctorId, doctorId);
	created->subDeptId[0] = '\0';
	strcpy(created->date, date);
	created->slot = slot;
	initQueue(&created->queue);
	created->next = g_waitingQueueHead;
	g_waitingQueueHead = created;
	return created;
}

// 查找患者在某医生某天某时段的挂号记录，返回指向该挂号记录的指针，如果未找到或已取消则返回NULL
static QueueTicket* findTicket(const char* patientId, const char* doctorId, const char* date, TimeSlot slot) {
	QueueTicket* curr = g_ticketHead;
	while (curr != NULL) {
		if (
			strcmp(curr->patient->patientId, patientId) == 0 &&
			strcmp(curr->doctor->doctorId, doctorId) == 0 &&
			strcmp(curr->date, date) == 0 &&
			curr->slot == slot &&
			curr->status != STATUS_CANCELLED
		) {
			return curr;
		}
		curr = curr->next;
	}
	return NULL;
}

// 释放排队队列中的所有节点内存，并重置队列状态
static void clearQueueNodes(Queue* q) {
	QueueNode* curr = q->front;
	while (curr != NULL) {
		QueueNode* next = curr->next;
		free(curr);
		curr = next;
	}
	q->front = NULL;
	q->rear = NULL;
	q->size = 0;
}

void initQueue(Queue* q) {
	q->front = NULL;
	q->rear = NULL;
	q->size = 0;
}

void enqueue(Queue* q, Patient* patient) {
	QueueNode* newNode = (QueueNode*)malloc(sizeof(QueueNode));
	if (newNode == NULL) {
		printf(">>> 内存分配失败，无法将患者加入排队队列！\n");
		return;
	}
	newNode->patient = patient;
	newNode->pority = patientPriority(patient);
	newNode->next = NULL;
	if (q->rear == NULL) {
		q->front = newNode;
		q->rear = newNode;
	}
	else {
		q->rear->next = newNode;
		q->rear = newNode;
	}
	q->size++;
}

void dequeue(Queue* q) {
	if (q->front == NULL) {
		printf(">>> 当前排队队列为空，无患者可叫号！\n");
		return;
	}
	QueueNode* temp = q->front;
	q->front = q->front->next;
	if (q->front == NULL) {
		q->rear = NULL;
	}
	free(temp);
	q->size--;
}

bool openDoctorScheduleSlot(const char* doctorId, const char* date, TimeSlot slot) {
	if (doctorId == NULL || date == NULL || slot <= SLOT_INVALID || (slot > SLOT_COUNT && slot != SLOT_NIGHT)) {
		return false;
	}
	DoctorDaySchedule* schedule = getSchedule(doctorId, date, true);
	if (schedule == NULL) {
		return false;
	}
	// 晚间排班与白天排班互斥：排晚班的医生不得有白天排班，反之亦然
	if (slot == SLOT_NIGHT) {
		for (int i = 1; i <= SLOT_COUNT; ++i) {
			if (schedule->openSlots[i]) {
				printf(">>> 该医生已有白天排班，不可同时排晚间急诊班次。\n");
				return false;
			}
		}
	}
	else {
		if (schedule->openSlots[SLOT_NIGHT]) {
			printf(">>> 该医生已有晚间急诊班次，不可同时排白天班次。\n");
			return false;
		}
	}
	schedule->openSlots[slot] = true;
	return true;
}

bool cancelDoctorScheduleSlot(const char* doctorId, const char* date, TimeSlot slot) {
	if (doctorId == NULL || date == NULL || slot <= SLOT_INVALID || (slot > SLOT_COUNT && slot != SLOT_NIGHT)) {
		return false;
	}
	DoctorDaySchedule* schedule = getSchedule(doctorId, date, false);
	if (schedule == NULL || !schedule->openSlots[slot]) {
		return false;
	}
	if (schedule->bookingCount[slot] > 0) {
		printf(">>> 当前时段已有挂号记录，无法直接取消排班。\n");
		return false;
	}
	schedule->openSlots[slot] = false;
	schedule->bookingCount[slot] = 0;
	return true;
}

int getDoctorSlotRemain(const char* doctorId, const char* date, TimeSlot slot) {
	DoctorDaySchedule* schedule = getSchedule(doctorId, date, false);
	if (schedule == NULL || !schedule->openSlots[slot]) {
		return 0;
	}
	// 晚间急诊不限额
	if (slot == SLOT_NIGHT) {
		return 999;
	}
	if (schedule->bookingCount[slot] >= MAX_APP) {
		return 0;
	}
	return MAX_APP - schedule->bookingCount[slot];
}

bool isDoctorSlotOpen(const char* doctorId, const char* date, TimeSlot slot) {
	if (slot <= SLOT_INVALID || (slot > SLOT_COUNT && slot != SLOT_NIGHT)) {
		return false;
	}
	DoctorDaySchedule* schedule = getSchedule(doctorId, date, false);
	if (schedule == NULL) {
		return false;
	}
	return schedule->openSlots[slot];
}

int getDoctorSlotBooked(const char* doctorId, const char* date, TimeSlot slot) {
	if (slot <= SLOT_INVALID || (slot > SLOT_COUNT && slot != SLOT_NIGHT)) {
		return 0;
	}
	DoctorDaySchedule* schedule = getSchedule(doctorId, date, false);
	if (schedule == NULL || !schedule->openSlots[slot]) {
		return 0;
	}
	return schedule->bookingCount[slot];
}

bool bookQueueTicket(Patient* patient, doctor* doctor, const char* date, TimeSlot slot, bool isOnsite) {
	if (patient == NULL || doctor == NULL || date == NULL || slot <= SLOT_INVALID || slot > SLOT_COUNT) {
		return false;
	}
	if (isNoonSlot(slot)) {
		printf(">>> 午休时段（11:30-13:30）暂不开放看诊，请选择其他时段。\n");
		return false;
	}
	DoctorDaySchedule* schedule = getSchedule(doctor->doctorId, date, false);
	if (schedule == NULL || !schedule->openSlots[slot]) {
		printf(">>> 该医生该时段尚未排班，无法挂号。\n");
		return false;
	}
	if (schedule->bookingCount[slot] >= MAX_APP) {
		printf(">>> 该医生该时段已满号（每时段最多5人）。\n");
		return false;
	}
	if (findTicket(patient->patientId, doctor->doctorId, date, slot) != NULL) {
		printf(">>> 您在该时段已有挂号记录，请勿重复挂号。\n");
		return false;
	}

	QueueTicket* ticket = (QueueTicket*)malloc(sizeof(QueueTicket));
	if (ticket == NULL) {
		return false;
	}
	ticket->patient = patient;
	ticket->doctor = doctor;
	strcpy(ticket->date, date);
	ticket->slot = slot;
	ticket->isOnsite = isOnsite;
	ticket->checkedIn = false;
	ticket->signSeq = 0;
	ticket->lateMinutes = 0;
	ticket->status = STATUS_WAITING;
	ticket->priorityOrder = 0;
	ticket->next = g_ticketHead;
	g_ticketHead = ticket;

	schedule->bookingCount[slot]++;
	return true;
}

bool checkInQueueTicket(const char* patientId, const char* doctorId, const char* date, TimeSlot slot, const char* signInTime) {
	QueueTicket* ticket = findTicket(patientId, doctorId, date, slot);
	if (ticket == NULL) {
		printf(">>> 未找到对应挂号记录，签到失败。\n");
		return false;
	}
	if (ticket->checkedIn) {
		printf(">>> 该挂号记录已完成签到，请勿重复签到。\n");
		return true;
	}

	int slotMinute = slotStartMinute(slot);
	int signMinute = parseMinuteOfDay(signInTime);
	if (slotMinute < 0 || signMinute < 0) {
		printf(">>> 签到时间格式错误，应为 HH:MM。\n");
		return false;
	}

	int delta = signMinute - slotMinute;

	// 不可提前超过5分钟签到
	if (delta < -5) {
		printf(">>> 不可签到，未到签到时间（最早可提前5分钟签到）。\n");
		return false;
	}

	// 获取当前系统时间所在时段
	int sH = 0, sM = 0;
	char slotTime[TIME_STR_LEN];
	if (sscanf(signInTime, "%d:%d", &sH, &sM) >= 2)
		snprintf(slotTime, sizeof(slotTime), "%02d:%02d:00", sH, sM);
	else
		snprintf(slotTime, sizeof(slotTime), "%s", signInTime);
	TimeSlot currentSlot = (TimeSlot)changeTimeToSlot(slotTime);
	TimeSlot DetailCurrentSlot = (TimeSlot)changeTimeToSlotInAll(slotTime);	// 获取更精细的时段（包含午休时段），用于判断是否在午休时段内提前签到
	if (DetailCurrentSlot == SLOT_1300_1330 && delta < 0) {
		// 在午休时段内提前签到，视为准时签到
	}
	else if (currentSlot == SLOT_INVALID) {
		if(DetailCurrentSlot == SLOT_1130_1200 || DetailCurrentSlot == SLOT_1200_1230 || DetailCurrentSlot == SLOT_1230_1300 || DetailCurrentSlot == SLOT_1300_1330)
			printf(">>> 当前不是门诊时段(午休)，无法签到。\n");
		else if (DetailCurrentSlot == SLOT_NIGHT)
			printf(">>> 当前不是门诊时段(晚间急诊)，无法签到。\n");
		else
			printf(">>> 当前不是门诊时段，无法签到。\n");
		return false;
	}

	// 时段最后3分钟禁止签到（已预约但未签到的视为迟到，不可再签到）
	int currentSlotStart = slotStartMinute(currentSlot);
	if (currentSlotStart >= 0) {
		int offsetInSlot = signMinute - currentSlotStart;
		if (offsetInSlot >= 27) {
			printf(">>> 当前时段已过签到截止时间（最后3分钟），无法签到。\n");
			return false;
		}
	}

	TimeSlot originalSlot = ticket->slot;

	ticket->checkedIn = true;
	ticket->signSeq = g_signSeq++;
	ticket->status = STATUS_WAITING;

	// 提前签到（delta < 0）或准时签到（同一时段内），均按准时处理
	if (delta < 0 || currentSlot == originalSlot) {
		ticket->lateMinutes = 0;
		refreshSlotQueue(doctorId, date, ticket->slot);
		if (delta < 0) {
			printf(">>> 提前签到成功（距离开诊还有 %d 分钟）。\n", -delta);
		}
		return true;
	}

	// 迟到签到：检查是否在允许范围内（预约时段后最多2个时段=1小时）
	if (currentSlot > originalSlot + 2) {
		printf(">>> 已超过最晚签到时间（预约时段后1小时内），无法签到。\n");
		// 回滚签到状态，该记录仍可后续使用
		ticket->checkedIn = false;
		ticket->status = STATUS_CANCELLED;
		return false;
	}

	// 迟到签到：寻找可用时段
	TimeSlot targetSlot = currentSlot;
	int slotsLate = 0;
	while (targetSlot <= SLOT_COUNT) {
		if (isDoctorSlotOpen(doctorId, date, targetSlot) && getDoctorSlotRemain(doctorId, date, targetSlot) > 0) {
			break;
		}
		targetSlot = (TimeSlot)(targetSlot + 1);
		slotsLate++;
	}
	if (targetSlot > SLOT_COUNT) {
		printf(">>> 所有后续时段已满，该挂号记录已取消。\n");
		ticket->status = STATUS_CANCELLED;
		ticket->checkedIn = false;
		return false;
	}

	ticket->slot = targetSlot;
	ticket->lateMinutes = (targetSlot - originalSlot) * 30;
	printf(">>> 迟到签到，已排至 %s 时段末尾。\n", slot_names[targetSlot - 1]);

	refreshSlotQueue(doctorId, date, ticket->slot);
	return true;
}

void refreshSlotQueue(const char* doctorId, const char* date, TimeSlot slot) {
	WaitingQueue* waiting = getWaitingQueue(doctorId, date, slot, true);
	if (waiting == NULL) {
		return;
	}
	clearQueueNodes(&waiting->queue);

	QueueTicket* onTime[128] = { 0 };
	QueueTicket* late30[128] = { 0 };
	QueueTicket* late60[128] = { 0 };
	QueueTicket* over60[128] = { 0 };
	int onCount = 0;
	int late30Count = 0;
	int late60Count = 0;
	int over60Count = 0;

	QueueTicket* curr = g_ticketHead;
	while (curr != NULL) {
		if (
			curr->checkedIn &&
			curr->status == STATUS_WAITING &&
			strcmp(curr->doctor->doctorId, doctorId) == 0 &&
			strcmp(curr->date, date) == 0 &&
			curr->slot == slot
		) {
			if (curr->lateMinutes == 0 && onCount < 128) {
				onTime[onCount++] = curr;
			}
			else if (curr->lateMinutes == 30 && late30Count < 128) {
				late30[late30Count++] = curr;
			}
			else if (curr->lateMinutes == 60 && late60Count < 128) {
				late60[late60Count++] = curr;
			}
			else if (over60Count < 128) {
				over60[over60Count++] = curr;
			}
		}
		curr = curr->next;
	}

	qsort(onTime, onCount, sizeof(QueueTicket*), comparePriorityThenSign);
	qsort(late30, late30Count, sizeof(QueueTicket*), compareSignOnly);
	qsort(late60, late60Count, sizeof(QueueTicket*), compareSignOnly);
	qsort(over60, over60Count, sizeof(QueueTicket*), compareSignOnly);

	QueueTicket* ordered[512] = { 0 };
	int size = 0;
	for (int i = 0; i < onCount; ++i) {
		ordered[size++] = onTime[i];
	}
	for (int i = 0; i < late30Count; ++i) {
		int insertPos = (size < 3) ? size : 3;
		for (int j = size; j > insertPos; --j) {
			ordered[j] = ordered[j - 1];
		}
		ordered[insertPos] = late30[i];
		++size;
	}
	for (int i = 0; i < late60Count; ++i) {
		int insertPos = (size < 6) ? size : 6;
		for (int j = size; j > insertPos; --j) {
			ordered[j] = ordered[j - 1];
		}
		ordered[insertPos] = late60[i];
		++size;
	}
	for (int i = 0; i < over60Count; ++i) {
		ordered[size++] = over60[i];
	}

	for (int i = 0; i < size; ++i) {
		enqueue(&waiting->queue, ordered[i]->patient);
	}
}

Patient* callNextPatient(const char* doctorId, const char* date, TimeSlot slot) {
	// 先刷新队列（refreshSlotQueue内部会按需创建WaitingQueue）
	refreshSlotQueue(doctorId, date, slot);
	WaitingQueue* waiting = getWaitingQueue(doctorId, date, slot, false);
	if (waiting == NULL || waiting->queue.front == NULL) {
		printf(">>> 当前时段暂无可叫号患者。\n");
		return NULL;
	}

	Patient* nextPatient = waiting->queue.front->patient;
	QueueTicket* ticket = findTicket(nextPatient->patientId, doctorId, date, slot);
	if (ticket != NULL) {
		ticket->status = STATUS_CALLED;
	}
	dequeue(&waiting->queue);
	refreshSlotQueue(doctorId, date, slot);
	return nextPatient;
}

void printSlotQueue(const char* doctorId, const char* date, TimeSlot slot) {
	if (TEST_SYSTEM_DEBUG) {
		if (confirmFunc("使用", "自定义时间")) {
			char timeStr[16];
			safeGetString("请输入时间（格式 HH:MM）：", timeStr, sizeof(timeStr));
			strcat(timeStr, ":00");
			slot = changeTimeToSlot(timeStr);
		}
		if(confirmFunc("使用", "自定义日期")) {
			char dateStr[DATE_STR_LEN];
			safeGetString("请输入日期（格式 YYYY-MM-DD）：", dateStr, sizeof(dateStr));
			strcpy(date, dateStr);
		}
	}
	// 先刷新队列（refreshSlotQueue内部会按需创建WaitingQueue）
	refreshSlotQueue(doctorId, date, slot);
	WaitingQueue* waiting = getWaitingQueue(doctorId, date, slot, false);
	if (waiting == NULL) {
		printf(">>> 当前时段还没有排队数据。\n");
		return;
	}
	printf("\n--- 排队队列 [医生:%s 日期:%s 时段:%s] ---\n", doctorId, date, slot_names[slot - 1]);
	QueueNode* curr = waiting->queue.front;
	int idx = 1;
	while (curr != NULL) {
		printf("%d) %s (%s)\n", idx++, curr->patient->name, curr->patient->patientId);
		curr = curr->next;
	}
	if (idx == 1) {
		printf(">>> 当前时段暂无已签到患者。\n");
	}
	pressEnterToContinue();	
}

void printDoctorScheduleTable(HIS_System* sys, const char* doctorId, const char* date) {
	doctor* targetDoctor = findDoctorByIdInQueue(sys, doctorId);
	const char* doctorName = (targetDoctor != NULL && targetDoctor->doctorName[0] != '\0') ? targetDoctor->doctorName : "未知";
	printf("\n========== 医生排班表 ==========\n医生: %s\n编号: %s\n日期: %s\n--------------------------------\n", doctorName, doctorId, date);
	for (int i = 1; i <= SLOT_COUNT; ++i) {
		if (!isDoctorSlotOpen(doctorId, date, (TimeSlot)i)) {
			printf("[%s] %s\n", slot_names[i - 1],
				isNoonSlot((TimeSlot)i) ? "午休" : "未排班");
		}
		else {
			int booked = getDoctorSlotBooked(doctorId, date, (TimeSlot)i);
			printf("[%s] 已挂号:%d 剩余:%d\n", slot_names[i - 1], booked, MAX_APP - booked);
		}
	}
	// 晚间急诊单独显示
	if (isDoctorSlotOpen(doctorId, date, SLOT_NIGHT)) {
		int booked = getDoctorSlotBooked(doctorId, date, SLOT_NIGHT);
		printf("[晚间急诊] 已挂号:%d 剩余:不限额\n", booked);
	}
	else {
		printf("[晚间急诊] 未排班\n");
	}
	printf("================================\n");
	pressEnterToContinue();
}

void exportDoctorSchedules(FILE* fp, const char* doctorId) {
	if (fp == NULL || doctorId == NULL) {
		return;
	}
	DoctorDaySchedule* curr = g_scheduleHead;
	while (curr != NULL) {
		if (strcmp(curr->doctorId, doctorId) == 0) {
			for (int slot = 1; slot <= SLOT_COUNT; ++slot) {
				if (curr->openSlots[slot]) {
					fprintf(fp, "S %s %d %d\n", curr->date, slot, curr->bookingCount[slot]);
				}
			}
			// 导出晚间急诊排班
			if (curr->openSlots[SLOT_NIGHT]) {
				fprintf(fp, "S %s %d %d\n", curr->date, (int)SLOT_NIGHT, curr->bookingCount[SLOT_NIGHT]);
			}
		}
		curr = curr->next;
	}
}

void importDoctorSchedule(const char* doctorId, const char* date, TimeSlot slot, int bookingCount) {
	// 晚间急诊时段需在枚举值范围内（SLOT_NIGHT = 18，通过 slot > SLOT_COUNT && slot != SLOT_NIGHT 排除非法值）
	if (doctorId == NULL || date == NULL || slot <= SLOT_INVALID || (slot > SLOT_COUNT && slot != SLOT_NIGHT)) {
		printf(">>> 导入数据格式错误，无法设置医生排班。\n");
		return;
	}
	DoctorDaySchedule* schedule = getSchedule(doctorId, date, true);
	if (schedule == NULL) {
		printf(">>> 内存分配失败，无法设置医生排班。\n");
		return;
	}
	// 导入时默认将该时段设置为已排班，并根据bookingCount设置已挂号数量
	schedule->openSlots[slot] = true;
	if (bookingCount < 0) {
		bookingCount = 0;
	}
	if (bookingCount > MAX_APP) {
		printf(">>> 导入数据中挂号数量超过每时段最大值，已自动调整为%d。\n", MAX_APP);
		bookingCount = MAX_APP;
	}
	schedule->bookingCount[slot] = bookingCount;
}

// 判断时段是否为午休时段（11:30-13:30），午休时段暂不开放看诊
bool isNoonSlot(TimeSlot slot) {
	return (slot >= SLOT_1130_1200 && slot <= SLOT_1300_1330);
}

void printAllTimeSlots(void) {
	int prefixTargetWidth;
	// 1. 根据 SLOT_COUNT 决定前缀宽度，确保所有时段的 XX:XX 部分自然对齐
	if (SLOT_COUNT < 10) {
		prefixTargetWidth = 3;
	}
	else if (SLOT_COUNT < 100) {
		prefixTargetWidth = 4;
	}
	else {
		prefixTargetWidth = 5;
	}

	for (int i = 0; i < SLOT_COUNT; ++i) {
		char prefix[16];
		sprintf(prefix, "%d.", i + 1);   // 构造 "1."、"13." 等前缀

		// 打印前缀并自动补齐空格（纯 ASCII，printFormattedStr 完全适用）
		printFormattedStr(prefix, prefixTargetWidth);

		//打印时间段，此时所有 XX:XX 已自然对齐
		printf("%s", slot_names[i]);

		if (isNoonSlot((TimeSlot)(i + 1))) {
			printf(" (午休)");
		}
		printf("\n");
	}
}

// ========== 跨模块辅助函数（供QueueFileManage调用） ==========

// 将挂号单添加到全局链表头部
void queueAddTicket(QueueTicket* ticket) {
	if (ticket == NULL) {
		return;
	}
	ticket->next = g_ticketHead;
	g_ticketHead = ticket;
}

// 获取全局挂号单链表头指针
QueueTicket* queueGetTicketHead(void) {
	return g_ticketHead;
}

// 更新全局签到序列号
void queueUpdateSignSeq(int maxSeq) {
	if (maxSeq >= g_signSeq) {
		g_signSeq = maxSeq + 1;
	}
}

// 重建所有候诊队列
void queueRebuildAll(void) {
	rebuildAllWaitingQueues();
}

// ========== 跨模块状态查询与流转函数（供PatientManage使用） ==========

// 根据患者编号和医生编号查找对应的挂号记录（排除已取消的，有多个时取signSeq最大的）
QueueTicket* findTicketByDoctorPatient(const char* doctorId, const char* patientId) {
	if (doctorId == NULL || patientId == NULL) {
		return NULL;
	}
	QueueTicket* best = NULL;
	int bestSeq = -1;
	QueueTicket* curr = g_ticketHead;
	while (curr != NULL) {
		if (curr->patient != NULL && curr->doctor != NULL &&
			strcmp(curr->patient->patientId, patientId) == 0 &&
			strcmp(curr->doctor->doctorId, doctorId) == 0 &&
			curr->status != STATUS_CANCELLED) {
			if (curr->signSeq > bestSeq) {
				best = curr;
				bestSeq = curr->signSeq;
			}
		}
		curr = curr->next;
	}
	return best;
}

// 检查某患者是否已被某医生叫号（状态为STATUS_CALLED或STATUS_IN_ROOM）
bool isPatientCalledByDoctor(const char* patientId, const char* doctorId) {
	QueueTicket* ticket = findTicketByDoctorPatient(doctorId, patientId);
	if (ticket == NULL) {
		return false;
	}
	return (ticket->status == STATUS_CALLED || ticket->status == STATUS_IN_ROOM);
}

// 检查患者是否处于就诊中状态（STATUS_IN_ROOM），用于结束看诊等操作
bool isPatientInRoomByDoctor(const char* patientId, const char* doctorId) {
	QueueTicket* ticket = findTicketByDoctorPatient(doctorId, patientId);
	if (ticket == NULL) {
		return false;
	}
	return (ticket->status == STATUS_IN_ROOM);
}

// 检查医生是否曾经叫号过某患者（含已结束看诊、过号），用于出院管理等历史关联操作
bool hasPatientCalledByDoctor(const char* patientId, const char* doctorId) {
	QueueTicket* ticket = findTicketByDoctorPatient(doctorId, patientId);
	if (ticket == NULL) {
		return false;
	}
	return (ticket->status == STATUS_CALLED
		|| ticket->status == STATUS_IN_ROOM
		|| ticket->status == STATUS_FINISHED
		|| ticket->status == STATUS_MISSED);
}

// 将患者挂号单状态从STATUS_CALLED推进为STATUS_IN_ROOM（表示已进入诊室就诊）
bool markTicketAsInRoom(const char* patientId, const char* doctorId) {
	QueueTicket* ticket = findTicketByDoctorPatient(doctorId, patientId);
	if (ticket == NULL) {
		return false;
	}
	if (ticket->status != STATUS_CALLED) {
		return false;
	}
	ticket->status = STATUS_IN_ROOM;
	return true;
}

// 将患者挂号单状态推进为STATUS_FINISHED（表示看诊结束）
bool markTicketAsFinished(const char* patientId, const char* doctorId) {
	QueueTicket* ticket = findTicketByDoctorPatient(doctorId, patientId);
	if (ticket == NULL) {
		return false;
	}
	if (ticket->status != STATUS_IN_ROOM && ticket->status != STATUS_CALLED) {
		printf(">>> 该患者当前不在就诊中状态，无法结束看诊。\n");
		return false;
	}
	ticket->status = STATUS_FINISHED;
	return true;
}

// 根据医生编号查找当前已叫号（STATUS_CALLED 或 STATUS_IN_ROOM）的患者编号
// 使用三级排序逻辑找最优患者（急诊 > priorityOrder > signSeq）
const char* findCalledPatientIdByDoctor(const char* doctorId) {
	if (doctorId == NULL) {
		return NULL;
	}
	QueueTicket* best = NULL;
	for (QueueTicket* curr = g_ticketHead; curr != NULL; curr = curr->next) {
		if (curr->doctor == NULL || curr->patient == NULL)
			continue;
		if (strcmp(curr->doctor->doctorId, doctorId) != 0)
			continue;
		if (curr->status != STATUS_CALLED && curr->status != STATUS_IN_ROOM)
			continue;
		if (best == NULL) {
			best = curr;
			continue;
		}
		// 三级比较：患者类型 > priorityOrder > signSeq
		int pa = patientPriority(curr->patient);
		int pb = patientPriority(best->patient);
		if (pa != pb) {
			if (pa > pb) best = curr;
			continue;
		}
		if (curr->priorityOrder != best->priorityOrder) {
			if (curr->priorityOrder > best->priorityOrder) best = curr;
			continue;
		}
		if (curr->signSeq < best->signSeq) {
			best = curr;
		}
	}
	return (best != NULL) ? best->patient->patientId : NULL;
}

// ========== 候诊与就诊管理（医生工作站子菜单） ==========

// 列出某医生当前所有已叫号/就诊中的患者，三级排序：急诊 > 优先标记 > 签到顺序
void printConsultingPatientsByDoctor(const char* doctorId) {
	if (doctorId == NULL) return;

	// 收集所有 CALLED/IN_ROOM 的患者
	QueueTicket* collected[256];
	int count = 0;
	for (QueueTicket* curr = g_ticketHead; curr != NULL; curr = curr->next) {
		if (curr->doctor == NULL || curr->patient == NULL) continue;
		if (strcmp(curr->doctor->doctorId, doctorId) != 0) continue;
		if (curr->status != STATUS_CALLED && curr->status != STATUS_IN_ROOM) continue;
		if (count < 256) {
			collected[count++] = curr;
		}
	}

	if (count == 0) {
		printf(">>> 当前没有已叫号/就诊中的患者。\n");
		return;
	}

	qsort(collected, count, sizeof(QueueTicket*), compareConsultingPriority);

	printf("\n========== 当前就诊患者列表 ==========\n");
	printFormattedStr("排序", 4);
	printFormattedStr("患者编号", 14);
	printFormattedStr("患者姓名", 12);
	printFormattedStr("患者类型", 10);
	printFormattedStr("状态", 8);
	printFormattedStr("优先标记", 10);
	printf("\n");
	printFormattedStr("----", 4);
	printFormattedStr("--------------", 14);
	printFormattedStr("------------", 12);
	printFormattedStr("----------", 10);
	printFormattedStr("--------", 8);
	printFormattedStr("----------", 10);
	printf("\n");

	for (int i = 0; i < count; ++i) {
		QueueTicket* t = collected[i];
		char idx[16], patId[32], patName[32], typeStr[16], statusStr[16], priorityStr[16];
		snprintf(idx, sizeof(idx), "%d", i + 1);
		snprintf(patId, sizeof(patId), "%s", t->patient->patientId);
		snprintf(patName, sizeof(patName), "%s", t->patient->name);
		if (t->patient->type == PATIENT_EMERGENCY)
			snprintf(typeStr, sizeof(typeStr), "急诊");
		else if (t->patient->type == PATIENT_VIP)
			snprintf(typeStr, sizeof(typeStr), "VIP");
		else
			snprintf(typeStr, sizeof(typeStr), "普通");
		snprintf(statusStr, sizeof(statusStr), "%s", t->status == STATUS_IN_ROOM ? "就诊中" : "已叫号");
		snprintf(priorityStr, sizeof(priorityStr), "%s", t->priorityOrder > 0 ? "优先" : "");
		printFormattedStr(idx, 4);
		printFormattedStr(patId, 14);
		printFormattedStr(patName, 12);
		printFormattedStr(typeStr, 10);
		printFormattedStr(statusStr, 8);
		printFormattedStr(priorityStr, 10);
		printf("\n");
	}
	printf("------------------------------------------\n");
}

// 医生手动设定优先看诊患者：清除本医生所有旧优先标记，在目标患者上设新标记
static void setDoctorPriorityPatient(const char* doctorId) {
	if (doctorId == NULL) return;

	// 收集该医生所有 CALLED/IN_ROOM 的患者
	QueueTicket* collected[256];
	int count = 0;
	for (QueueTicket* curr = g_ticketHead; curr != NULL; curr = curr->next) {
		if (curr->doctor == NULL || curr->patient == NULL) continue;
		if (strcmp(curr->doctor->doctorId, doctorId) != 0) continue;
		if (curr->status != STATUS_CALLED && curr->status != STATUS_IN_ROOM) continue;
		if (count < 256) {
			collected[count++] = curr;
		}
	}

	if (count == 0) {
		printf(">>> 当前没有已叫号/就诊中的患者，无法设置优先。\n");
		return;
	}

	// 排序后展示
	qsort(collected, count, sizeof(QueueTicket*), compareConsultingPriority);
	printConsultingPatientsByDoctor(doctorId);

	printf("\n>>> 请选择要设为优先的患者（输入序号，输入 0 返回）: ");
	int choice = safeGetInt("");
	if (choice == 0) return;
	if (choice < 1 || choice > count) {
		printf(">>> 无效的选择。\n");
		return;
	}

	QueueTicket* target = collected[choice - 1];

	// 清除该医生所有旧优先标记
	for (int i = 0; i < count; ++i) {
		collected[i]->priorityOrder = 0;
	}

	// 对目标患者设置新的优先标记
	target->priorityOrder = ++g_priorityCounter;
	printf(">>> 已将患者 %s (%s) 设为优先看诊对象。\n", target->patient->name, target->patient->patientId);
}

// 医生工作站「候诊与就诊管理」二级菜单
void doctorConsultationMenu(HIS_System* sys, const char* doctorId) {
	if (sys == NULL || doctorId == NULL) {
		printf(">>> 医生身份无效。\n");
		return;
	}

	int choice = -1;
	while (1) {
		if (isNightTime()) {
			// 夜班：仅显示挂号队列，不涉及优先级管理
			printf("\n========== 候诊与就诊管理 ==========\n");
			printf("1. 打印当前挂号队列\n");
			printf("0. 返回上一级菜单\n");
			printf("======================================\n");
			choice = safeGetInt("请选择操作: ");

			switch (choice) {
			case 1: printNightQueue(doctorId, getCurrentDateStr()); break;
			case 0: return;
			default: printf(">>> 无效的选择。\n"); break;
			}
		} else {
			printf("\n========== 候诊与就诊管理 ==========\n");
			printf("1. 打印当前挂号队列\n");
			printf("2. 查看当前就诊患者\n");
			printf("3. 指定优先看诊患者\n");
			printf("0. 返回上一级菜单\n");
			printf("======================================\n");
			choice = safeGetInt("请选择操作: ");

			switch (choice) {
			case 1: {
				TimeSlot curSlot = (TimeSlot)changeTimeToSlot(getCurrentTimeStr());
				if (curSlot == SLOT_INVALID) {
					printf(">>> 当前不是门诊时段，无法查看队列。\n");
				} else {
					printSlotQueue(doctorId, getCurrentDateStr(), curSlot);
				}
				break;
			}
			case 2: printConsultingPatientsByDoctor(doctorId); break;
			case 3: setDoctorPriorityPatient(doctorId); break;
			case 0: return;
			default: printf(">>> 无效的选择。\n"); break;
			}
		}
	}
}

// ========== 晚间急诊模块 ==========

// 获取当前系统时间对应的时段（晚间返回SLOT_NIGHT，其他委托给changeTimeToSlot）
TimeSlot getCurrentSlot(void) {
	char* timeStr = getCurrentTimeStr();
	if (isNightTime()) {
		return SLOT_NIGHT;
	}
	return (TimeSlot)changeTimeToSlot(timeStr);
}

// 列出当天所有排了晚间急诊的医生
void printNightDoctors(const char* date) {
	printf("\n========== 晚间急诊医生列表 ==========\n");
	printf("日期: %s\n", date);
	printf("--------------------------------------\n");

	bool foundAny = false;
	DoctorDaySchedule* curr = g_scheduleHead;
	while (curr != NULL) {
		if (strcmp(curr->date, date) == 0 && curr->openSlots[SLOT_NIGHT]) {
			// 找到对应的医生信息
			doctor* doc = NULL;
			for (QueueTicket* t = g_ticketHead; t != NULL; t = t->next) {
				if (t->doctor != NULL) {
					doc = t->doctor;
					if (strcmp(doc->doctorId, curr->doctorId) == 0) break;
				}
			}
			// 如果ticket链表中没有，需要通过外部查找；这里使用标识
			printf("医生编号: %s\n", curr->doctorId);
			foundAny = true;
		}
		curr = curr->next;
	}
	if (!foundAny) {
		printf(">>> 当天暂无晚间急诊值班医生。\n");
	}
	printf("======================================\n");
}

// 晚间急诊挂号
bool bookNightEmergencyTicket(Patient* patient, doctor* doc, const char* date) {
	if (patient == NULL || doc == NULL || date == NULL) {
		return false;
	}
	// 检查医生是否有晚间排班
	DoctorDaySchedule* schedule = getSchedule(doc->doctorId, date, false);
	if (schedule == NULL || !schedule->openSlots[SLOT_NIGHT]) {
		printf(">>> 该医生今日无晚间急诊排班，无法挂号。\n");
		return false;
	}

	// 检查是否已挂过该医生的晚间急诊
	QueueTicket* t = g_ticketHead;
	while (t != NULL) {
		if (strcmp(t->patient->patientId, patient->patientId) == 0 &&
			strcmp(t->doctor->doctorId, doc->doctorId) == 0 &&
			strcmp(t->date, date) == 0 &&
			t->slot == SLOT_NIGHT &&
			t->status != STATUS_CANCELLED && t->status != STATUS_FINISHED) {
			printf(">>> 您已挂过该医生的晚间急诊，请勿重复挂号。\n");
			return false;
		}
		t = t->next;
	}

	QueueTicket* ticket = (QueueTicket*)malloc(sizeof(QueueTicket));
	if (ticket == NULL) {
		return false;
	}
	ticket->patient = patient;
	ticket->doctor = doc;
	strcpy(ticket->date, date);
	ticket->slot = SLOT_NIGHT;
	ticket->isOnsite = true;       // 当场挂号
	ticket->checkedIn = true;      // 自动签到
	ticket->signSeq = g_signSeq++;
	ticket->lateMinutes = 0;       // 不适用迟到规则
	ticket->status = STATUS_WAITING;
	ticket->priorityOrder = 0;
	ticket->next = g_ticketHead;
	g_ticketHead = ticket;

	schedule->bookingCount[SLOT_NIGHT]++;
	printf(">>> 晚间急诊挂号成功！签到序号: %d\n", ticket->signSeq);
	return true;
}

// 比较函数：仅按signSeq升序（用于晚间队列）
static int compareNightSign(const void* lhs, const void* rhs) {
	const QueueTicket* a = *(const QueueTicket**)lhs;
	const QueueTicket* b = *(const QueueTicket**)rhs;
	return a->signSeq - b->signSeq;
}

// 刷新晚间急诊队列（仅按signSeq升序，无分桶/无优先级）
void refreshNightQueue(const char* doctorId, const char* date) {
	WaitingQueue* waiting = getWaitingQueue(doctorId, date, SLOT_NIGHT, true);
	if (waiting == NULL) {
		return;
	}
	clearQueueNodes(&waiting->queue);

	QueueTicket* nightTickets[256] = { 0 };
	int nightCount = 0;

	QueueTicket* curr = g_ticketHead;
	while (curr != NULL) {
		if (curr->checkedIn &&
			curr->status == STATUS_WAITING &&
			strcmp(curr->doctor->doctorId, doctorId) == 0 &&
			strcmp(curr->date, date) == 0 &&
			curr->slot == SLOT_NIGHT &&
			nightCount < 256) {
			nightTickets[nightCount++] = curr;
		}
		curr = curr->next;
	}

	qsort(nightTickets, nightCount, sizeof(QueueTicket*), compareNightSign);

	for (int i = 0; i < nightCount; ++i) {
		enqueue(&waiting->queue, nightTickets[i]->patient);
	}
}

// 晚间叫号
Patient* callNextNightPatient(const char* doctorId, const char* date) {
	if (TEST_NIGHT_TIME) {
		char dateStr[DATE_STR_LEN];
		if (confirmFunc("使用", "自定义日期")) {
			safeGetString("请输入日期（格式 YYYY-MM-DD）：", dateStr, sizeof(dateStr));
		}
		if (isValidDate(dateStr)) {
			printf(">>> 已设置测试日期为 %s。\n", dateStr);
			strcpy(date, dateStr);
		}
		else {
			printf(">>> 无效的日期格式，已取消测试日期设置。\n");
		}
	}
	refreshNightQueue(doctorId, date);
	WaitingQueue* waiting = getWaitingQueue(doctorId, date, SLOT_NIGHT, false);
	if (waiting == NULL || waiting->queue.front == NULL) {
		printf(">>> 晚间急诊暂无候诊患者。\n");
		return NULL;
	}

	Patient* nextPatient = waiting->queue.front->patient;
	QueueTicket* ticket = findTicket(nextPatient->patientId, doctorId, date, SLOT_NIGHT);
	if (ticket != NULL) {
		ticket->status = STATUS_IN_ROOM;
	}
	dequeue(&waiting->queue);
	refreshNightQueue(doctorId, date);
	printf(">>> 叫号成功: %s (%s) — 已进入诊室就诊\n", nextPatient->name, nextPatient->patientId);
	return nextPatient;
}

// 打印晚间急诊队列
void printNightQueue(const char* doctorId, const char* date) {
	if (TEST_NIGHT_TIME) {
		char dateStr[DATE_STR_LEN];
		if(confirmFunc("使用", "自定义日期")) {
			safeGetString("请输入日期（格式 YYYY-MM-DD）：", dateStr, sizeof(dateStr));
		}
		if(isValidDate(dateStr)) {
			printf(">>> 已设置测试日期为 %s。\n", dateStr);
			strcpy(date, dateStr);
		}
		else {
			printf(">>> 无效的日期格式，已取消测试日期设置。\n");
		}
	}
	refreshNightQueue(doctorId, date);
	WaitingQueue* waiting = getWaitingQueue(doctorId, date, SLOT_NIGHT, false);

	printf("\n========== 晚间急诊候诊队列 ==========\n");
	printf("日期: %s  时段: 晚间急诊\n", date);
	printf("医生编号: %s\n", doctorId);
	printf("--------------------------------------\n");

	if (waiting == NULL || waiting->queue.front == NULL) {
		printf(">>> 暂无候诊患者。\n");
		printf("======================================\n");
		pressEnterToContinue();
		return;
	}

	int idx = 1;
	QueueNode* node = waiting->queue.front;
	while (node != NULL) {
		Patient* p = node->patient;
		printf("%d) %s (%s)\n", idx, p->name, p->patientId);
		node = node->next;
		++idx;
	}
	printf("======================================\n");
	pressEnterToContinue();
}
