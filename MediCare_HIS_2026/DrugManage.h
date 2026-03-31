#pragma once
#ifndef DRUGMANAGE_H
#define DRUGMANAGE_H
#include "HIS_System.h"
#include <stdbool.h>
#include <string.h>

// 检查药品编号是否存在
bool isDrugIdExist(Drug* head, const char* id);

// 检查药品通用名是否存在
bool isDrugGenNameExist(Drug* head, const char* genName);

// 检查药品商品名是否存在
bool isDrugTraNameExist(Drug* head, const char* traName);

// 检查药品别名是否存在
bool isDrugAliNameExist(Drug* head, const char* aliName);

//药品管理菜单界面
void drugManageMenu(HIS_System* sys);

//录入新药品
void addDrug(HIS_System* sys);

//打印单条药品详细信息
void printDrugInfo(Drug* drug);

//查询药品信息
void queryDrug(HIS_System* sys);

//修改药品信息
void modifyDrug(HIS_System* sys);

//显示所有药品信息
void displayAllDrugs(HIS_System* sys);

//排序功能在DrugSort.h中实现

//删除药品记录功能
void deleteDrug(HIS_System** sys);

//根据删除方式和查询字符串，删除工具
void deleteDrugFunc(Drug** head, const char* queryStr, int mode);

#endif // DRUGMANAGE_H
