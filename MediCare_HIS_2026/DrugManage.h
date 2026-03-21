#pragma once
#ifndef DRUGMANAGE_H
#define DRUGMANAGE_H
#include "HIS_System.h"
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

// 录入新药品
void addDrug(HIS_System* sys);

#endif // DRUGMANAGE_H
