#pragma once
// WardSort.h
#ifndef WARDSORT_H
#define WARDSORT_H

#include "HIS_System.h"

//排序方式
typedef enum {
	WARD_SORT_EXIT = 0,         // 退出排序
	WARD_SORT_BY_ID = 1,        // 按病房编号
	WARD_SORT_BY_TYPE,          // 按病房种类
	WARD_SORT_BY_DEPT,          // 按所属科室
	WARD_SORT_BY_BED_TOTAL,     // 按床位总数
	WARD_SORT_BY_BED_FREE,		// 按剩余床位数
	WARD_SORT_BY_BED_USED		// 按已使用床位数
} WardSortChoice;

//排序顺序
typedef enum {
	WARD_ORDER_ASC = 1,			// 升序
	WARD_ORDER_DESC = 2,		// 降序
	WARD_ORDER_EXIT = 0			// 退出排序
} WardSortOrder;

//对病床链表按床位编号排序（升序）
void sortBedList(Bed** head);

//病房排序菜单界面
void wardSortMenu(HIS_System* sys);

#endif
