#pragma once
#ifndef EXAMFILEMANAGE_H
#define EXAMFILEMANAGE_H

#include"HIS_System.h"
//检查项目字典文件加载状态标志
extern bool is_Exam_Item_File_Loaded;

//检查申请/结果文件加载状态标志
extern bool is_Exam_Order_File_Loaded;

//检查项目字典文件加载函数
void loadExamItemData(HIS_System* sys);

//检查项目字典文件保存函数
void saveExamItemData(HIS_System* sys);

//检查申请/结果文件加载函数
void loadExamOrderData(HIS_System* sys);

//检查申请/结果文件保存函数
void saveExamOrderData(HIS_System* sys);

#endif // !EXAMFILEMANAGE_H
