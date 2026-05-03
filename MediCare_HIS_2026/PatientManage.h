#pragma once
#ifndef PATIENTMANAGE_H
#define PATIENTMANAGE_H
#include"HIS_System.h"
#include"stdbool.h"
#include"string.h"

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

//计算两日期之间的天数（简易近似算法，日期格式 YYYY-MM-DD）
int daysBetweenDates(const char* start, const char* end);

#endif // !PATIENTMANAGE_H