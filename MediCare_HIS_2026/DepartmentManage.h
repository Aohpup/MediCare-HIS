#pragma once
#ifndef DEPARTMENTMANAGE_H
#define DEPARTMENTMANAGE_H
#include"HIS_System.h"

//检查科室编号是否存在
bool isDepartmentIdExist(Department* head, const char* id);

//检查科室名称是否存在
bool isDepartmentNameExist(Department* head, const char* name);

//检查二级科室名称是否存在
bool isSubDepartmentNameExist(Department* head, const char* name);

//检查一级科室代码是否存在
bool isCategoryIdExist(Department* head, const char* id);

//统计某二级科室编号下的医生数量（为可能的“多医生-单科室”扩展预留）
int countDoctorsBySubDeptId(HIS_System* sys, const char* subDeptId);

//科室管理菜单界面
void departmentManageMenu(HIS_System* sys);

//加入新科室
void addDepartment(HIS_System* sys);

//打印单条科室详细信息
void printDepartmentInfo(Department* department, SubDepartment* subDepartment);

//查询科室信息
void queryDepartment(HIS_System* sys);

//修改科室信息
void modifyDepartment(HIS_System* sys);

//排序科室数据
//排序功能在DepartmentSort.h中实现

//删除科室数据
void deleteDepartment(HIS_System** sys);

//根据删除方式和查询字符串的删除工具
void deleteDepartmentFunc(Department** head, const char* queryStr, int mode);

//显示所有科室信息
void displayAllDepartments(HIS_System* sys);


#endif // !DEPARTMENTMANAGE_H
