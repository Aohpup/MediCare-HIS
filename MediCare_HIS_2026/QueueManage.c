#define _CRT_SECURE_NO_WARNINGS
#include"HIS_System.h"
#include"QueueManage.h"
#include"ConfirmFunc.h"
#include"InputUtils.h"
#include<string.h>

// 全局链表：医生排班、挂号单、候诊队列
static DoctorDaySchedule* g_scheduleHead = NULL;
static QueueTicket* g_ticketHead = NULL;
static WaitingQueue* g_waitingQueueHead = NULL;
static int g_signSeq = 1;

// 根据患者类别返回优先级数值，急诊最高（3），VIP次之（2），普通最低（1）
static int patientPriority(const Patient* patient) {
	if (patient == NULL) {
		return 0;
	}
	if (patient->type == PATIENT_EMERGENCY) {
		return 3;
	}
	if (patient->type == PATIENT_VIP) {
		return 2;
	}
	return 1;
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
	if (rebuiltCount > 0) {
		printf(">>> 候诊队列重建完成，共重建 %d 个队列。\n", rebuiltCount);
	}
}

// 获取时间段的开始分钟数（相对于当天0点），如8:00对应480，8:30对应510，以此类推
static int slotStartMinute(TimeSlot slot) {
	static const int starts[SLOT_COUNT + 1] = {
		0,
		8 * 60 + 0,
		8 * 60 + 30,
		9 * 60 + 0,
		9 * 60 + 30,
		10 * 60 + 0,
		10 * 60 + 30,
		11 * 60 + 0,
		13 * 60 + 30,
		14 * 60 + 0,
		14 * 60 + 30,
		15 * 60 + 0,
		15 * 60 + 30,
		16 * 60 + 0
	};
	if (slot <= SLOT_INVALID || slot > SLOT_COUNT) {
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
	for (int i = 0; i <= SLOT_COUNT; ++i) {
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
	if (doctorId == NULL || date == NULL || slot <= SLOT_INVALID || slot > SLOT_COUNT) {
		return false;
	}
	DoctorDaySchedule* schedule = getSchedule(doctorId, date, true);
	if (schedule == NULL) {
		return false;
	}
	schedule->openSlots[slot] = true;
	return true;
}

bool cancelDoctorScheduleSlot(const char* doctorId, const char* date, TimeSlot slot) {
	if (doctorId == NULL || date == NULL || slot <= SLOT_INVALID || slot > SLOT_COUNT) {
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
	if (schedule->bookingCount[slot] >= MAX_APP) {
		return 0;
	}
	return MAX_APP - schedule->bookingCount[slot];
}

bool isDoctorSlotOpen(const char* doctorId, const char* date, TimeSlot slot) {
	if (slot <= SLOT_INVALID || slot > SLOT_COUNT) {
		return false;
	}
	DoctorDaySchedule* schedule = getSchedule(doctorId, date, false);
	if (schedule == NULL) {
		return false;
	}
	return schedule->openSlots[slot];
}

int getDoctorSlotBooked(const char* doctorId, const char* date, TimeSlot slot) {
	if (slot <= SLOT_INVALID || slot > SLOT_COUNT) {
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
	// 已签到的记录不允许重复签到
	if (ticket->checkedIn) {
		printf(">>> 该挂号记录已完成签到，请勿重复签到，注意查看叫号情况。\n");
		return true;
	}

	int slotMinute = slotStartMinute(slot);
	int signMinute = parseMinuteOfDay(signInTime);
	if (slotMinute < 0 || signMinute < 0) {
		printf(">>> 签到时间格式错误，应为 HH:MM。\n");
		return false;
	}

	int delta = signMinute - slotMinute;
	ticket->checkedIn = true;
	ticket->signSeq = g_signSeq++;
	ticket->status = STATUS_WAITING;

	if (delta <= 15) {
		ticket->lateMinutes = 0;
	}
	else if (delta <= 30) {
		ticket->lateMinutes = 30;
	}
	else if (delta <= 60) {
		ticket->lateMinutes = 60;
	}
	else {
		if (ticket->slot < SLOT_COUNT) {
			ticket->slot = (TimeSlot)(ticket->slot + 1);
			printf(">>> 迟到超过60分钟，已顺延至下一班次末尾。\n");
		}
		else {
			printf(">>> 迟到超过60分钟，当前已是末班次，将排在本班次末尾。\n");
		}
		ticket->lateMinutes = 120;
	}

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
	if(TEST_SYSTEM_DEBUG) {
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
}

void printDoctorScheduleTable(const char* doctorId, const char* date) {
	printf("\n========== 医生排班表 ==========\n医生: %s\n日期: %s\n--------------------------------\n", doctorId, date);
	for (int i = 1; i <= SLOT_COUNT; ++i) {
		if (!isDoctorSlotOpen(doctorId, date, (TimeSlot)i)) {
			printf("[%s] 未排班\n", slot_names[i - 1]);
		}
		else {
			int booked = getDoctorSlotBooked(doctorId, date, (TimeSlot)i);
			printf("[%s] 已挂号:%d 剩余:%d\n", slot_names[i - 1], booked, MAX_APP - booked);
		}
	}
	printf("================================\n");
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
		}
		curr = curr->next;
	}
}

void importDoctorSchedule(const char* doctorId, const char* date, TimeSlot slot, int bookingCount) {
	if (doctorId == NULL || date == NULL || slot <= SLOT_INVALID || slot > SLOT_COUNT) {
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

void printAllTimeSlots(void) {
	for (int i = 0; i < SLOT_COUNT; ++i) {
		printf("%d. %s\n", i + 1, slot_names[i]);
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

// 根据患者编号和医生编号查找对应的挂号记录（排除已取消的）
QueueTicket* findTicketByDoctorPatient(const char* doctorId, const char* patientId) {
	if (doctorId == NULL || patientId == NULL) {
		return NULL;
	}
	QueueTicket* curr = g_ticketHead;
	while (curr != NULL) {
		if (curr->patient != NULL && curr->doctor != NULL &&
			strcmp(curr->patient->patientId, patientId) == 0 &&
			strcmp(curr->doctor->doctorId, doctorId) == 0 &&
			curr->status != STATUS_CANCELLED) {
			return curr;
		}
		curr = curr->next;
	}
	return NULL;
}

// 检查某患者是否已被某医生叫号（状态为STATUS_CALLED或STATUS_IN_ROOM）
bool isPatientCalledByDoctor(const char* patientId, const char* doctorId) {
	QueueTicket* ticket = findTicketByDoctorPatient(doctorId, patientId);
	if (ticket == NULL) {
		return false;
	}
	return (ticket->status == STATUS_CALLED || ticket->status == STATUS_IN_ROOM);
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

// 将患者挂号单状态推进为STATUS_FINISHED（表示看诊结束），仅当状态为STATUS_IN_ROOM时生效
bool markTicketAsFinished(const char* patientId, const char* doctorId) {
	QueueTicket* ticket = findTicketByDoctorPatient(doctorId, patientId);
	if (ticket == NULL) {
		return false;
	}
	if (ticket->status != STATUS_IN_ROOM) {
		printf(">>> 该患者当前不在就诊中状态，无法结束看诊。\n");
		return false;
	}
	ticket->status = STATUS_FINISHED;
	return true;
}

// 根据医生编号查找当前已叫号（STATUS_CALLED或STATUS_IN_ROOM）的患者编号
const char* findCalledPatientIdByDoctor(const char* doctorId) {
	if (doctorId == NULL) {
		return NULL;
	}
	QueueTicket* curr = g_ticketHead;
	while (curr != NULL) {
		if (curr->doctor != NULL && curr->patient != NULL &&
			strcmp(curr->doctor->doctorId, doctorId) == 0 &&
			(curr->status == STATUS_CALLED || curr->status == STATUS_IN_ROOM)) {
			return curr->patient->patientId;
		}
		curr = curr->next;
	}
	return NULL;
}

