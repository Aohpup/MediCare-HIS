#pragma once
#ifndef WARDMANAGE_H
#define WARDMANAGE_H
#include"HIS_System.h"

//检查病房编号是否存在
bool isWardIdExist(Ward* head, const char* id);

//检查病床编号是否存在
bool isBedIdExist(Bed* head, const char* id);

//病房管理菜单界面
void wardManageMenu(HIS_System* sys);

//录入新病房
void addWard(HIS_System* sys);

//打印单条病房详细信息
void printWardInfo(Ward* ward);

//查询病房信息
void queryWard(HIS_System* sys);

//修改病房信息
void modifyWard(HIS_System* sys);

//排序病房数据
//排序功能在WardSort.h中实现

//删除病房记录
void deleteWard(HIS_System** sys);

//根据删除方式和查询字符串的删除工具
void deleteWardFunc(Ward** head, const char* queryStr, int mode);

//显示所有病房信息
void displayAllWards(HIS_System* sys);

#endif // !WARDMANAGE_H
