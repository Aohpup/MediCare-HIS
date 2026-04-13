#define _CRT_SECURE_NO_WARNINGS
#include"QueueManage.h"
#include<string.h>

typedef struct QueueTicket {
	Patient* patient;
	Docter* doctor;
	char date[DATE_STR_LEN];
	TimeSlot slot;
	bool isOnsite;
	bool checkedIn;
	int signSeq;
	int lateMinutes;
	PatientStatus status;
	struct QueueTicket* next;
} QueueTicket;

typedef struct DoctorDaySchedule {
	char doctorId[ID_LEN];
	char date[DATE_STR_LEN];
	bool openSlots[SLOT_COUNT + 1];
	int bookingCount[SLOT_COUNT + 1];
	struct DoctorDaySchedule* next;
} DoctorDaySchedule;

static DoctorDaySchedule* g_scheduleHead = NULL;
static QueueTicket* g_ticketHead = NULL;
static WaitingQueue* g_waitingQueueHead = NULL;
static int g_signSeq = 1;

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

static int compareSignOnly(const void* lhs, const void* rhs) {
	const QueueTicket* a = *(const QueueTicket**)lhs;
	const QueueTicket* b = *(const QueueTicket**)rhs;
	return a->signSeq - b->signSeq;
}

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

static int parseMinuteOfDay(const char* timeStr) {
	if (timeStr == NULL || strlen(timeStr) < 5) {
		return -1;
	}
	int hour = 0;
	int min = 0;
	if (sscanf(timeStr, "%d:%d", &hour, &min) != 2) {
		return -1;
	}
	if (hour < 0 || hour > 23 || min < 0 || min > 59) {
		return -1;
	}
	return hour * 60 + min;
}

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

static WaitingQueue* getWaitingQueue(const char* doctorId, const char* date, TimeSlot slot, bool createIfMissing) {
	WaitingQueue* curr = g_waitingQueueHead;
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

static QueueTicket* findTicket(const char* patientId, const char* doctorId, const char* date, TimeSlot slot) {
	QueueTicket* curr = g_ticketHead;
	while (curr != NULL) {
		if (
			strcmp(curr->patient->patientId, patientId) == 0 &&
			strcmp(curr->doctor->docterId, doctorId) == 0 &&
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
	if (!newNode) {
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

bool bookQueueTicket(Patient* patient, Docter* doctor, const char* date, TimeSlot slot, bool isOnsite) {
	if (patient == NULL || doctor == NULL || date == NULL || slot <= SLOT_INVALID || slot > SLOT_COUNT) {
		return false;
	}
	DoctorDaySchedule* schedule = getSchedule(doctor->docterId, date, false);
	if (schedule == NULL || !schedule->openSlots[slot]) {
		printf(">>> 该医生该时段尚未排班，无法挂号。\n");
		return false;
	}
	if (schedule->bookingCount[slot] >= MAX_APP) {
		printf(">>> 该医生该时段已满号（每时段最多5人）。\n");
		return false;
	}
	if (findTicket(patient->patientId, doctor->docterId, date, slot) != NULL) {
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
	if (ticket->checkedIn) {
		printf(">>> 该挂号记录已完成签到。\n");
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
			ticket->lateMinutes = 120;
			printf(">>> 迟到超过60分钟，已顺延至下一班次末尾。\n");
		}
		else {
			ticket->lateMinutes = 120;
			printf(">>> 迟到超过60分钟，当前已是末班次，将排在本班次末尾。\n");
		}
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
			strcmp(curr->doctor->docterId, doctorId) == 0 &&
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
	WaitingQueue* waiting = getWaitingQueue(doctorId, date, slot, false);
	if (waiting == NULL) {
		printf(">>> 当前时段暂无排队队列。\n");
		return NULL;
	}
	refreshSlotQueue(doctorId, date, slot);
	if (waiting->queue.front == NULL) {
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
	WaitingQueue* waiting = getWaitingQueue(doctorId, date, slot, false);
	if (waiting == NULL) {
		printf(">>> 当前时段还没有排队数据。\n");
		return;
	}
	refreshSlotQueue(doctorId, date, slot);
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

void printAllTimeSlots(void) {
	for (int i = 0; i < SLOT_COUNT; ++i) {
		printf("%d. %s\n", i + 1, slot_names[i]);
	}
}


