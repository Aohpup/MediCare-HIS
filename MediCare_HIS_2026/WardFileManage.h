#pragma once
#ifndef WARDFILEMANAGE_H
#define WARDFILEMANAGE_H
#include "HIS_System.h"

//保存病房数据到文件
void saveWardData(HIS_System* sys);

//从文件加载病房数据到系统
void loadWardData(HIS_System* sys);

#endif // !WARDFILEMANAGE_H
