#pragma once
#ifndef doctorFILEMANAGE_H
#define doctorFILEMANAGE_H

#include"HIS_System.h"

extern bool is_Doctor_File_Loaded;	//标记是否加载过医生数据

// 从txt文件加载医生系统数据
void loadDoctorSystemData(HIS_System* sys);

// 将医生系统数据保存到txt文件
void saveDoctorSystemData(HIS_System* sys);

#endif // !doctorFILEMANAGE_H