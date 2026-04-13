#pragma once
#ifndef PATIENTFILEMANAGE_H
#define PATIENTFILEMANAGE_H
#include"HIS_System.h"

//从系统内患者文件加载系统数据
void loadPatientsFromFile(HIS_System* sys);

//保存患者数据到文件
void savePatientsToFile(HIS_System* sys);

#endif // !PATIENTFILEMANAGE_H