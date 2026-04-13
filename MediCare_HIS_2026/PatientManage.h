#pragma once
#ifndef PATIENTMANAGE_H
#define PATIENTMANAGE_H
#include"HIS_System.h"
#include"stdbool.h"
#include"string.h"

//患者注册
void registerPatient(HIS_System* sys, const char* remainIdCard);

//挂号预约
void registerAppointment(HIS_System* sys);

//患者服务台
void patientManageMenu(HIS_System* sys);

#endif // !PATIENTMANAGE_H