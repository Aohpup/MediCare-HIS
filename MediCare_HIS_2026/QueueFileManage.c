#define _CRT_SECURE_NO_WARNINGS
#include"QueueFileManage.h"
#include"QueueManage.h"
#include<string.h>

bool is_Queue_Ticket_File_Loaded = false; //标记是否加载过排队挂号数据

// 从txt文件加载排队挂号数据
void loadQueueTicketData(HIS_System* sys) {
	if (is_Queue_Ticket_File_Loaded) {
		return;
	}
	FILE* fp = fopen(QUEUE_TICKET_FILE, "r");
	if (!fp) {
		printf(">>> 警告: 找不到 %s，将跳过排队挂号数据加载。\n", QUEUE_TICKET_FILE);
		// 文件不存在时仍需标记已处理，确保运行中产生的挂号记录在退出时能被保存
		is_Queue_Ticket_File_Loaded = true;
		return;
	}
	char line[512];
	if (fgets(line, sizeof(line), fp) != NULL) {
		if (line[0] != '#') {
			// 如果第一行不是注释，说明文件格式不正确，直接跳过加载
			fseek(fp, 0, SEEK_SET);
		}
	}
	int maxSeq = 0;
	while (fgets(line, sizeof(line), fp) != NULL) {
		if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
			continue;
		}
		char patientId[ID_LEN] = { 0 };
		char doctorId[ID_LEN] = { 0 };
		char date[DATE_STR_LEN] = { 0 };
		int slotNo = 0;
		int isOnsite = 0;
		int checkedIn = 0;
		int signSeq = 0;
		int lateMinutes = 0;
		int status = 0;
		if (sscanf(line, "T %24s %24s %19s %d %d %d %d %d %d", patientId, doctorId, date, &slotNo, &isOnsite, &checkedIn, &signSeq, &lateMinutes, &status) != 9) {
			continue;
		}
		Patient* patient = findPatientByIdInQueue(sys, patientId);
		doctor* doc = findDoctorByIdInQueue(sys, doctorId);
		if (patient == NULL || doc == NULL || slotNo <= SLOT_INVALID || slotNo > SLOT_COUNT) {
			continue;
		}
		QueueTicket* ticket = (QueueTicket*)malloc(sizeof(QueueTicket));
		if (ticket == NULL) {
			break;
		}
		memset(ticket, 0, sizeof(QueueTicket));
		ticket->patient = patient;
		ticket->doctor = doc;
		strcpy(ticket->date, date);
		ticket->slot = (TimeSlot)slotNo;
		ticket->isOnsite = (isOnsite != 0);
		ticket->checkedIn = (checkedIn != 0);
		ticket->signSeq = signSeq;
		ticket->lateMinutes = lateMinutes;
		ticket->status = (PatientStatus)status;
		queueAddTicket(ticket);
		if (signSeq > maxSeq) {
			maxSeq = signSeq;
		}
	}
	fclose(fp);
	queueUpdateSignSeq(maxSeq);
	is_Queue_Ticket_File_Loaded = true;
	printf(">>> 排队挂号数据加载完成！\n");

	// 加载完挂号单数据后，立即重建所有候诊队列
	// 确保医生叫号功能在系统重启后可以正常使用
	queueRebuildAll();
}

// 将排队挂号数据保存到txt文件
void saveQueueTicketData(HIS_System* sys) {
	(void)sys;
	FILE* fp = fopen(QUEUE_TICKET_FILE, "w");
	if (!fp) {
		printf(">>> 错误: 无法打开 %s 进行写入！\n", QUEUE_TICKET_FILE);
		return;
	}
	fprintf(fp, "# HIS QUEUE TICKET DATA FILE\n");
	fprintf(fp, "# 每行格式：T patientId doctorId date slot isOnsite checkedIn signSeq lateMinutes status\n");
	fprintf(fp, "# 中文是：T 挂号单数据行标识 患者编号 医生编号 预约日期 预约时段 是否当场挂号 是否已签到 签到顺序 迟到分钟数 患者状态\n");
	QueueTicket* curr = queueGetTicketHead();
	while (curr != NULL) {
		// 每行格式：T patientId doctorId date slot isOnsite checkedIn signSeq lateMinutes status
		// 中文是：T 挂号单数据行标识 患者编号 医生编号 预约日期 预约时段 是否当场挂号 是否已签到 签到顺序 迟到分钟数 患者状态
		fprintf(fp, "T %s %s %s %d %d %d %d %d %d\n",
			curr->patient->patientId,
			curr->doctor->doctorId,
			curr->date,
			(int)curr->slot,
			curr->isOnsite ? 1 : 0,
			curr->checkedIn ? 1 : 0,
			curr->signSeq,
			curr->lateMinutes,
			(int)curr->status);
		curr = curr->next;
	}
	fclose(fp);
	printf(">>> 排队挂号数据保存完成！\n");
}
