#pragma once
#ifndef PATIENTMANAGE_H
#define PATIENTMANAGE_H
#include"HIS_System.h"
#include"stdbool.h"
#include"string.h"

//标记患者是否已登录
extern bool is_Patient_Logged_In;

//获取当前登录患者的编号
const char* getCurrentPatientId(void);

//判断患者是否已登录
bool isPatientLoggedIn(void);

//根据患者编号查找患者信息
Patient* findPatientById(HIS_System* sys, const char* patientId);

//患者管理模块，包含患者注册、登录、挂号预约等功能
void loadFileAllData(HIS_System* sys);

//患者注册
void registerPatient(HIS_System* sys, const char* remainIdCard);

//患者登录
void logInPatient(HIS_System* sys);

//患者退出登录
void logOutPatient(void);

//挂号预约
void registerAppointment(HIS_System* sys);

//查看病例信息（患者视角）
void viewMedicalRecordPat(HIS_System* sys, const char* patientId);

//查看病例信息（医生视角）
void viewMedicalRecordDoc(HIS_System* sys, const char* doctorId);

//医生写入病例信息（看诊记录）
void writeMedicalRecord(HIS_System* sys, const char* doctorId);

//医生开具检查单
void issueExaminationOrder(HIS_System* sys, const char* doctorId);

//医生结束看诊，将患者挂号单状态从IN_ROOM推进为FINISHED（自动结诊，供医生登录出时调用）
int autoEndCurrentConsultation(HIS_System* sys, const char* doctorId);

//医生结束看诊，将患者挂号单状态从IN_ROOM推进为FINISHED
void endConsultation(HIS_System* sys, const char* doctorId);

//医生查看就诊历史记录，列出所有曾看诊过的患者
void viewConsultationHistory(HIS_System* sys, const char* doctorId);

//写入看诊病例（医生开具）
bool appendViewMedicalRecord(HIS_System* sys, const char* patientId, const char* doctorId, const char* details, const char* date);

//写入住院病例（新增 deptInfo、bedId 参数，details 由函数写入"入院"）
bool appendStayMedicalRecord(HIS_System* sys, const char* patientId, const char* doctorId, const char* deptInfo, const char* bedId, const char* startDate, const char* duration, const char* endDate, const char* wardId);

//更新住院记录的出院日期与时长（出院时调用）
bool updateStayRecordEnd(HIS_System* sys, const char* patientId, const char* wardId, const char* endDate, const char* duration);

//执行出院核心流程（释放床位、更新记录、回写病历）
void executeDischargePatient(HIS_System* sys, const char* patientId, const char* wardId, const char* bedId);

//患者端办理出院手续
void patientDischargeCheckout(HIS_System* sys, const char* patientId);


//患者个人信息查询与修改菜单
void patientInfoMenu(HIS_System* sys, const char* patientId);

//通用余额充值函数
//余额操作辅助函数
double getTotalBalance(const Patient* patient);
void addRealBalance(Patient* patient, double amount);
void addBonusBalance(Patient* patient, double amount);
double deductBalance(Patient* patient, double amount);

//患者余额充值菜单（需先登录，内部获取当前患者）
void patientRechargeMenu(HIS_System* sys);

//指定患者的充值菜单（锁定跳转用，不要求已登录）
void patientRechargeMenuForPatient(HIS_System* sys, Patient* patient);

//晚间急诊挂号（独立于白天挂号）
void registerNightEmergency(HIS_System* sys);

#endif // !PATIENTMANAGE_H