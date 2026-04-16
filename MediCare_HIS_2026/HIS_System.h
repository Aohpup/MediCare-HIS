#pragma once
#ifndef HIS_SYSTEM_H
#define HIS_SYSTEM_H
#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include"DayTimeUtils.h"
#include"ProjectLimits.h"

//测试信息
extern bool TEST_SYSTEM_DEBUG;		//是否启用测试（true启用，false禁用）

//S链表存储数据
//S.1药品
typedef struct Drug {
	char drugId[ID_LEN];			//商品编号
	char drugGbCode[16];		//国标码(国家药品本位码 共14位)
	char genericName[STR_LEN];		//通用名
	char tradeName[STR_LEN];		//商品名
	char alias[STR_LEN];			//别称/俗名
	int stock;						//剩余库存
	double price;					//价格(元)
	struct Drug* next;
}Drug;

//S.2医生&科室
	//S.2.1医生
	
//医生排班信息结构体
typedef struct DoctorSchedule {
	char day[DATE_STR_LEN];				//排班日期 (格式: XXXX-XX-XX)
	int quota[SLOT_COUNT + 1];			//排班时间段预约限额，0表示该时段未排班，正整数表示该时段的最大挂号数，默认为0、最大值为MAX_APP（每时段最多5人）
	int bookedCount[SLOT_COUNT + 1];	//已预约挂号数量
	struct DoctorSchedule* next;
} DoctorSchedule;

//医生信息结构体
typedef struct Docter {
	char docterId[ID_LEN];			//医生编号
	char docterName[STR_LEN];		//医生姓名
	char department[STR_LEN];		//所属一级科室名称
	char subDeptId[ID_LEN];		//所属诊室编号(二级科室唯一编码)

	DoctorSchedule* scheduleHead;	//医生排班链表头

	struct Docter* next;
}Docter;
	//S.2.2科室 (修改为一级科类->二级科室层级结构)
typedef struct SubDepartment {
	char subDeptName[STR_LEN];     // 二级科室名称 (具体科室名字，如：心内科)
	char subDeptId[ID_LEN];        // 具体诊室的编号
	struct SubDepartment* next;
}SubDepartment;

typedef struct Department {
	char categoryName[STR_LEN];    // 一级科室名称 (科室类名字，如：内科)
	char categoryId[ID_LEN];       // 一级科室代码
	SubDepartment* subDeptHead;    // 该大类下的具体科室链表
	struct Department* next;
}Department;

//S.3病床&病房
	//S.3.1病床
	typedef struct Bed {
		char bedId[BED_ID_LEN];			//病床编号 (病房内唯一编号，格式可自定义，如：Z15101，其中Z151表示病房编号，01表示床位编号)
		bool isOccupied;			//是否被使用
		char patient[ID_LEN];		//患者编号
		struct Bed* next;
	}Bed;

	//S.3.2病房种类
	typedef enum {
		WARD_NORMAL = 1,			// 普通病房 (设为1便于在终端上调用)
		WARD_VIP,					// VIP病房
		WARD_ICU					// ICU病房
	} WardType;

	//S.3.3病房
	typedef struct Ward {
		char wardId[ID_LEN];		//病房编号
		WardType type;				//病房种类
		char department[STR_LEN];	//病房所属科室
		Bed* bedListHead;			//病房内床位数
		struct Ward* next;
	} Ward;

//S.4医疗记录
	//挂号、看诊、检查、住院
typedef enum {
	REC_REG = 1,    // 挂号 Registration
	REC_VIEW,       // 看诊 Consultation
	REC_EXAM,       // 检查 Examination
	REC_STAY        // 住院 Hospitalization
} RecordType;

typedef enum {
	PATIENT_GENERAL = 1,	// 普通患者
	PATIENT_VIP,			// VIP患者
	PATIENT_EMERGENCY		// 急诊患者
} PatientType;

//初步患者编号生成规则,TODO: 之后根据实际需求调整生成规则，增加日期前缀、患者类别等
#define STARTING_PATIENT_ID 10000000	//患者编号起始值
extern int currentPatientId;			//当前患者编号计数器（全局变量，初始值为STARTING_PATIENT_ID）

typedef struct RegistrationRecord {
	char recordId[ID_LEN];			// 挂号记录编号
	char department[STR_LEN];		// 挂号科室
	char doctorId[ID_LEN];			// 挂号医生编号
	char date[ID_LEN];				// 挂号日期
	char time[ID_LEN];				// 挂号时间
	//RegistrationRecord* currRegTail;//当前患者挂号记录链表的末尾指针（仅在加载数据时使用，其他时候保持为NULL）
	struct RegistrationRecord* next;
} RegistrationRecord;

typedef struct ConsultationRecord {
	char recordId[ID_LEN];			// 医疗记录编号
	RecordType record;				// 记录类型（看诊 / 检查）
	char details[512];				//报告细节内容
	char date[ID_LEN];				//日期
	char doctorId[ID_LEN];			//所属医生编号
	struct ConsultationRecord* next;
} ConsultationRecord;

typedef struct StayRecord {
	char recordId[ID_LEN];			// 住院记录编号
	char details[512];				// 住院细节内容
	char startDate[ID_LEN];			// 住院开始日期
	char duration[ID_LEN];			// 住院时长
	char endDate[ID_LEN];			// 住院结束日期，如果仍在住院则为"未出院"或类似标识
	char doctorId[ID_LEN];			// 所属医生编号
	char wardId[ID_LEN];			// 关联病房编号
	struct StayRecord* next;
} StayRecord;

//S.5患者节点
typedef struct Patient {
	char patientId[ID_LEN];			//患者编号
	char name[STR_LEN];				//患者姓名
	char phone[ID_LEN];				//患者电话
	char idCard[18 + 3];			//患者身份证号
	char gender[STR_LEN];			//患者性别
	PatientType type;				//患者类别 (普通/VIP/急诊)

	RegistrationRecord* regHead;		// 挂号记录链表头
	ConsultationRecord* viewHead;		// 看诊记录链表头
	ConsultationRecord* examHead;		// 检查记录链表头
	StayRecord* stayHead;				// 住院记录链表头

	RegistrationRecord* currRegTail;			// 当前患者挂号记录链表的末尾指针
	ConsultationRecord* currViewTail;		// 当前患者看诊记录链表的末尾指针
	ConsultationRecord* currExamTail;		// 当前患者检查记录链表的末尾指针
	StayRecord* currStayTail;				// 当前患者住院记录链表的末尾指针

	struct Patient* next;
} Patient;


extern const char* slot_names[SLOT_COUNT];		// 挂号预约时间段列表

typedef enum {
	SLOT_INVALID = 0,	// 无效时间段
	SLOT_0800_0830, SLOT_0830_0900, SLOT_0900_0930, SLOT_0930_1000 = 4,
	SLOT_1000_1030, SLOT_1030_1100, SLOT_1100_1130 = 7,
	SLOT_1330_1400, SLOT_1400_1430, SLOT_1430_1500 = 10,
	SLOT_1500_1530, SLOT_1530_1600, SLOT_1600_1630 = 13
} TimeSlot;


//extern Patient* appointmentSlots[13 + 1][MAX_APP];	// 预约挂号表，第一维为时间段，第二维为该时段的挂号列表


//患者排队状态枚举
typedef enum {
	STATUS_WAITING,    // 等待叫号
	STATUS_CALLED,     // 已叫号（但未进入诊室）
	STATUS_IN_ROOM,    // 就诊中
	STATUS_FINISHED,   // 就诊结束
	STATUS_MISSED,     // 过号未应
	STATUS_CANCELLED   // 爽约/取消
} PatientStatus;

typedef struct HIS_System {
	Drug* drugHead;
	Drug* drugDisplayHead;
	Docter* docHead;
	SubDepartment* subDeptHead;
	Department* deptHead;
	Ward* wardHead;
	Patient* patientHead;
	Patient* patientTail;
} HIS_System;


// 初始化医疗管理系统底座
void initSystem(HIS_System* sys);
//Warning: 添加数据前请务必先调用 initSystem() 初始化系统底座，否则可能会导致严重错误！！！


//保存系统数据到文件
void saveSystemData(HIS_System* sys);

//释放系统内存（当前至少释放药品原始链表和显示链表）
void cleanupSystemMemory(HIS_System* sys);

#endif // HIS_SYSTEM_H