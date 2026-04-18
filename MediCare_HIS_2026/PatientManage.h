#pragma once
#ifndef PATIENTMANAGE_H
#define PATIENTMANAGE_H
#include"HIS_System.h"
#include"stdbool.h"
#include"string.h"

//患者管理模块，包含患者注册、登录、挂号预约等功能
void loadFileAllData(HIS_System* sys);

//患者注册
void registerPatient(HIS_System* sys, const char* remainIdCard);

//患者登录
void logInPatient(HIS_System* sys);

//挂号预约
void registerAppointment(HIS_System* sys);

//查看病例信息（患者视角）
void viewMedicalRecordPat(HIS_System* sys, const char* patientId);

//查看病例信息（医生视角）
void viewMedicalRecordDoc(HIS_System* sys, const char* doctorId);

//医生写入病例信息（看诊记录）
void writeMedicalRecord(HIS_System* sys, const char* doctorId);

//写入看诊病例（医生开具）
bool appendViewMedicalRecord(HIS_System* sys, const char* patientId, const char* doctorId, const char* details, const char* date);

//写入检查病例（预留接口）
bool appendExamMedicalRecord(HIS_System* sys, const char* patientId, const char* doctorId, const char* details, const char* date);

//写入住院病例（预留接口）
bool appendStayMedicalRecord(HIS_System* sys, const char* patientId, const char* doctorId, const char* details, const char* startDate, const char* duration, const char* endDate, const char* wardId);

#endif // !PATIENTMANAGE_H