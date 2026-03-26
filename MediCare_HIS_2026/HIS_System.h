#pragma once
#ifndef HIS_SYSTEM_H
#define HIS_SYSTEM_H
#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include"ProjectLimits.h"

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
typedef struct Docter {
	char docterId[ID_LEN];			//医生编号
	char docterName[STR_LEN];		//医生姓名
	char department[STR_LEN];		//医生所在科室
	int consultationCount;			//诊号数量
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
	SubDepartment* subDeptHead;    // 该大类下的具体科室链表
	struct Department* next;
}Department;

//S.3病床&病房
	//S.3.1病床
typedef struct Bed {
	char bedId[ID_LEN];			//病房编号
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

typedef struct MedicalRecord {
	char recordId[ID_LEN];			// 医疗记录编号
	RecordType record;
	char details[512];				//报告细节内容
	char date[ID_LEN];				//日期
	char doctorId[ID_LEN];			//所属医生编号
	struct MedicalRecord* next;
} MedicalRecord;

//S.5患者节点 //TODO: 四项该怎么建链表?
typedef struct Patient {
	char patientId[ID_LEN];			//患者编号
	char name[STR_LEN];				//患者姓名
	char phone[ID_LEN];				//患者电话
	int type;						//患者类别
	//	MedicalRecord* recordsHead;		//患者所属医疗记录
	MedicalRecord* regHead;			// 挂号记录链表
	MedicalRecord* viewHead;		// 看诊记录链表
	MedicalRecord* examHead;		// 检查记录链表
	MedicalRecord* stayHead;		// 住院记录链表
	struct Patient* next;
} Patient;

typedef struct HIS_System {
	Drug* drugHead;
	Docter* docHead;
	SubDepartment* subDeptHead;
	Department* deptHead;
	Ward* wardHead;
	Patient* patientHead;
} HIS_System;

// 初始化医疗管理系统底座
void initSystem(HIS_System* sys);
//Warning: 添加数据前请务必先调用 initSystem() 初始化系统底座，否则可能会导致严重错误！！！

#endif // HIS_SYSTEM_H