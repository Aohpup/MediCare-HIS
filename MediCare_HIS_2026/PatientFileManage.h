#pragma once
#ifndef PATIENTFILEMANAGE_H
#define PATIENTFILEMANAGE_H
#include"HIS_System.h"

extern bool is_Patient_File_Loaded;	//标记是否加载过患者数据

//从系统内患者文件加载系统数据
void loadPatientsSystemData(HIS_System* sys);

//保存患者数据到文件
void savePatientsSystemData(HIS_System* sys);

#endif // !PATIENTFILEMANAGE_H