#pragma once
#ifndef DOCTERFILEMANAGE_H
#define DOCTERFILEMANAGE_H

#include"HIS_System.h"

// 从txt文件加载医生系统数据
void loadDoctorSystemData(HIS_System* sys);

// 将医生系统数据保存到txt文件
void saveDoctorSystemData(HIS_System* sys);

#endif // !DOCTERFILEMANAGE_H