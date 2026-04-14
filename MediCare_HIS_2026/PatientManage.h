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

#endif // !PATIENTMANAGE_H