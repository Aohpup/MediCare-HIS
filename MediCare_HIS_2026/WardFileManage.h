#pragma once
#ifndef WARDFILEMANAGE_H
#define WARDFILEMANAGE_H
#include "HIS_System.h"

extern bool is_Ward_File_Loaded;	//标记是否加载过病房数据

//保存病房数据到文件
void saveWardSystemData(HIS_System* sys);

//从文件加载病房数据到系统
void loadWardSystemData(HIS_System* sys);

#endif // !WARDFILEMANAGE_H
