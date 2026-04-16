#pragma once
#ifndef QUEUEMANAGE_H
#define QUEUEMANAGE_H
//排队管理模块头文件
#include"HIS_System.h"
#include"DayTimeUtils.h"

//排队节点结构体
typedef struct QueueNode {
	Patient* patient;			//排队患者信息指针
	int pority;				//优先级（根据患者类别设置，急诊 > VIP > 普通）
	struct QueueNode* next;
} QueueNode;

//队列结构体
typedef struct Queue {
	QueueNode* front;			//队列头指针
	QueueNode* rear;			//队列尾指针
	int size;					//队列当前长度
} Queue;

//医生排队信息结构体
typedef struct WaitingQueue {
	char doctorId[ID_LEN];		//对应的医生编号
	char subDeptId[ID_LEN];		//对应的诊室编号
	char date[DATE_STR_LEN];		//对应的排队日期
	TimeSlot slot;				//对应的时段
	Queue queue;				//排队队列
	struct WaitingQueue* next;
} WaitingQueue;

//排队挂号信息结构体
typedef struct QueueTicket {
	Patient* patient;
	Docter* doctor;					
	char date[DATE_STR_LEN];		//预约的日期
	TimeSlot slot;					//预约的时段
	bool isOnsite;					//是否当场挂号
	bool checkedIn;					//是否已签到
	int signSeq;					//签到顺序（同一时段内，签到越早的患者signSeq越小）
	int lateMinutes;				//迟到分钟数（0/30/60/120，分别对应准时/迟到30分钟内/迟到30-60分钟/迟到超过60分钟）
	PatientStatus status;			//患者状态
	struct QueueTicket* next;
} QueueTicket;

//医生排班信息结构体
typedef struct DoctorDaySchedule {
	char doctorId[ID_LEN];
	char date[DATE_STR_LEN];
	bool openSlots[SLOT_COUNT + 1];		//排班时间段标记，true表示该时段有排班，false表示无排班
	int bookingCount[SLOT_COUNT + 1];
	struct DoctorDaySchedule* next;
} DoctorDaySchedule;

//初始化队列
void initQueue(Queue* q);

//患者入队
void enqueue(Queue* q, Patient* patient);	

//患者出队(医生叫号)
void dequeue(Queue* q);			

// 获取当前时间所在的时段
int getCurrentTimeSlot(void);

// 医生排班：开启某位医生某天某个时段的接诊
bool openDoctorScheduleSlot(const char* doctorId, const char* date, TimeSlot slot);

// 医生排班：取消某位医生某天某个时段的接诊
bool cancelDoctorScheduleSlot(const char* doctorId, const char* date, TimeSlot slot);

// 查询某位医生某天某个时段剩余可挂号数量
int getDoctorSlotRemain(const char* doctorId, const char* date, TimeSlot slot);

// 查询某位医生某天某个时段是否已排班
bool isDoctorSlotOpen(const char* doctorId, const char* date, TimeSlot slot);

// 查询某位医生某天某个时段已挂号数量
int getDoctorSlotBooked(const char* doctorId, const char* date, TimeSlot slot);

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

// 打印医生某日排班表
void printDoctorScheduleTable(const char* doctorId, const char* date);

// 将某位医生排班写入已打开文件
void exportDoctorSchedules(FILE* fp, const char* doctorId);

// 从文件恢复某位医生某日时段排班与挂号计数
void importDoctorSchedule(const char* doctorId, const char* date, TimeSlot slot, int bookingCount);

// 时间段展示工具
void printAllTimeSlots(void);


#endif // !QUEUEMANAGE_H
