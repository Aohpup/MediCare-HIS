#pragma once
#ifndef DEPARTMENTSORT
#define DEPARTMENTSORT
#include"HIS_System.h"

// 排序方式常量定义
#define SORT_EXIT			0		//退出排序
#define SORT_BY_CATEGORY	1		//根据一级科室名称排序
#define SORT_BY_CATEGORY_ID	2		//根据一级科室代码排序
#define SORT_BY_SUBDEPT		3		//根据二级科室名称排序
#define SORT_BY_ID			4		//根据科室编号排序

// 排序顺序常量定义
#define ORDER_EXIT 0			//退出排序
#define ORDER_ASC  1			//升序
#define ORDER_DESC 2			//降序

// 对病床链表按床位编号排序（升序）
void sortBedList(Bed** head);

// 排序方式常量定义
bool needToSortDepartment(Department* a, Department* b, int choice, int order);

// 交换数据
void swapDepartments(Department* a, Department* b);

// 根据用户选择的排序方式和顺序，对科室链表进行排序
void sortDepartmentList(Department* head, Department* tail, int choice, int order);

// 排序菜单界面
void departmentSortMenu(HIS_System* sys);
#endif // !DEPARTMENTSORT
