#pragma once
#ifndef doctorMANAGE_H
#define doctorMANAGE_H
#include "HIS_System.h"

//医生登录
bool logInDoctor(HIS_System* sys);

//获取当前登录医生的编号
char* getCurrentDoctorId();

//检查医生编号是否存在
bool isDoctorIdExist(doctor* head, const char* id);

//检查医生姓名是否存在
bool isDoctorNameExist(doctor* head, const char* name);

//患者端医生查询二级菜单（按科室/编号/姓名）
void queryDoctorPat(HIS_System* sys, const char* patientId);

//医生管理菜单界面
void doctorManageMenu(HIS_System* sys);

//医生管理菜单界面（医生视角）
void doctorManageMenuDoc(HIS_System* sys, const char* doctorId);

//医生界面（患者视角，展示医生公示信息）
void doctorManageMenuPat(HIS_System* sys, const char* patientId);

//录入新医生
void addDoctor(HIS_System* sys);

//打印单条医生详细信息
void printDoctorInfo(doctor* doctor);

//查询医生信息
void queryDoctor(HIS_System* sys, const char* currDocterId);

//修改医生信息
void modifyDoctor(HIS_System* sys);

//排序医生数据
//排序功能在doctorSort.h中实现

//删除医生记录
void deleteDoctor(HIS_System** sys);

//根据删除方式和查询字符串的删除工具
void deleteDoctorFunc(doctor** head, const char* queryStr, int mode);

//显示所有医生信息
void displayAllDoctors(HIS_System* sys);

//医生排班管理
void doctorScheduleMenu(HIS_System* sys, const char* currentDoctorId);

//医生叫号
void doctorCallQueueMenu(HIS_System* sys, const char* currentDoctorId);

//查看医生排班表与候诊信息
void doctorViewScheduleBoardMenu(HIS_System* sys);

//查看患者病历信息（医生视角）
//void writeMedicalRecord(HIS_System* sys, const char* doctorId);

// 医生端：查看患者住院信息（当前叫号患者 / 手动指定）
void doctorViewStayInfo(HIS_System* sys, const char* doctorId);

// 医生端：安排患者住院（分配病房和床位）
void doctorArrangeWard(HIS_System* sys, const char* doctorId);

// 医生端：住院管理二级菜单（分配/查看/出院）
void doctorWardMenu(HIS_System* sys, const char* doctorId);

#endif // !doctorMANAGE_H
