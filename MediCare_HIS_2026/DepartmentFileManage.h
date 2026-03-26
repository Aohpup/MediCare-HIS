#pragma once
#ifndef DEPARTMENTFILEMANAGE_H
#define DEPARTMENTFILEMANAGE_H

#include"HIS_System.h"

//从txt文件加载科室系统数据
void loadDepartmentSystemData(HIS_System* sys);

//将科室系统数据保存到txt文件
void saveDepartmentSystemData(HIS_System* sys);

#endif // !DEPARTMENTFILEMANAGE_H
