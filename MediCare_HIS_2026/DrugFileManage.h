#pragma once
#ifndef FILEMANAGE_H
#define FILEMANAGE_H
#include "HIS_System.h"

// 从txt文件加载药物系统数据
void loadDrugSystemData(HIS_System* sys);

// 将药物系统数据保存到txt文件
void saveDrugSystemData(HIS_System* sys);

#endif // FILEMANAGE_H