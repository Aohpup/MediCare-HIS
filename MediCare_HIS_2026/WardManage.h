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

// 病房种类转字符串
const char* wardTypeToStr(WardType type);

// 统计床位数/已占用数
int countBeds(Ward* ward);
int countOccupiedBeds(Ward* ward);

// 按床位编号查找
Bed* findBed(Ward* ward, const char* bedId);

// 患者端：查看我的住院信息（只读）
void patientViewStayInfo(HIS_System* sys, const char* patientId);

// 医生端：查看患者病房信息（只读，显示患者姓名、病房号、床位号等基本信息）
void docterViewPatientStayInfo(HIS_System* sys, const char* doctorId);

// 患者端：病房查询（仅可查看自己所住病房）
void wardQueryMenuPat(HIS_System* sys, const char* patientId);

// 自动推荐病房（按优先级规则，供医生分配时调用）
Ward* autoRecommendWard(HIS_System* sys, PatientType patientType, const char* doctorDept);

// 查找患者入住病房
Ward* findPatientWard(HIS_System* sys, const char* patientId);

#endif // !WARDMANAGE_H
