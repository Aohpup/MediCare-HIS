#pragma once
#ifndef DOCTERMANAGE_H
#define DOCTERMANAGE_H
#include "HIS_System.h"

//检查医生编号是否存在
bool isDoctorIdExist(Docter* head, const char* id);

//检查医生姓名是否存在
bool isDoctorNameExist(Docter* head, const char* name);

//医生管理菜单界面
void doctorManageMenu(HIS_System* sys);

//录入新医生
void addDoctor(HIS_System* sys);

//打印单条医生详细信息
void printDoctorInfo(Docter* doctor);

//查询医生信息
void queryDoctor(HIS_System* sys);

//修改医生信息
void modifyDoctor(HIS_System* sys);

//排序医生数据
//排序功能在DocterSort.h中实现

//删除医生记录
void deleteDoctor(HIS_System** sys);

//根据删除方式和查询字符串的删除工具
void deleteDoctorFunc(Docter** head, const char* queryStr, int mode);

//显示所有医生信息
void displayAllDoctors(HIS_System* sys);


#endif // !DOCTERMANAGE_H
