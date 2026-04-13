#pragma once
#ifndef QUEUEMANAGE_H
#define QUEUEMANAGE_H
//排队管理模块头文件
#include"HIS_System.h"

typedef struct QueueNode {
	Patient* patient;			//排队患者信息指针
	int pority;				//优先级（根据患者类别设置，急诊 > VIP > 普通）
	struct QueueNode* next;
} QueueNode;

typedef struct Queue {
	QueueNode* front;			//队列头指针
	QueueNode* rear;			//队列尾指针
	int size;					//队列当前长度
} Queue;

typedef struct WaitingQueue {
	char doctorId[ID_LEN];		//对应的医生编号
	char subDeptId[ID_LEN];		//对应的诊室编号
	char date[DATE_STR_LEN];		//对应的排队日期
	TimeSlot slot;				//对应的时段
	Queue queue;				//排队队列
	struct WaitingQueue* next;
} WaitingQueue;

void initQueue(Queue* q);			//初始化队列

void enqueue(Queue* q, Patient* patient);	//患者入队

void dequeue(Queue* q);			//患者出队(医生叫号)

// 医生排班：开启某位医生某天某个时段的接诊
bool openDoctorScheduleSlot(const char* doctorId, const char* date, TimeSlot slot);

// 查询某位医生某天某个时段剩余可挂号数量
int getDoctorSlotRemain(const char* doctorId, const char* date, TimeSlot slot);

// 预约/当场挂号
bool bookQueueTicket(Patient* patient, Docter* doctor, const char* date, TimeSlot slot, bool isOnsite);

// 签到并刷新队列。返回true表示签到成功并可参与叫号
bool checkInQueueTicket(const char* patientId, const char* doctorId, const char* date, TimeSlot slot, const char* signInTime);

// 刷新某医生某天某时段队列
void refreshSlotQueue(const char* doctorId, const char* date, TimeSlot slot);

// 医生叫号，返回被叫到的患者（若为空则表示无可叫号患者）
Patient* callNextPatient(const char* doctorId, const char* date, TimeSlot slot);

// 打印当前时段队列
void printSlotQueue(const char* doctorId, const char* date, TimeSlot slot);

// 时间段展示工具
void printAllTimeSlots(void);


#endif // !QUEUEMANAGE_H
