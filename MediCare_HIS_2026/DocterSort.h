#pragma once
#ifndef DOCTERSORT_H
#define DOCTERSORT_H
#include "HIS_System.h"

// 排序方式常量定义
#define SORT_EXIT        0		//退出排序
#define SORT_BY_ID			1		//根据医生编号排序
#define SORT_BY_NAME		2		//根据医生姓名排序
#define SORT_BY_DEPT		3		//根据医生所在科室排序
#define SORT_BY_CONSULT		4		//根据医生诊号数量排序

// 排序顺序常量定义
#define ORDER_EXIT 0			//退出排序
#define ORDER_ASC  1			//升序
#define ORDER_DESC 2			//降序

//确定是否需要交换两个医生节点以满足用户选择的排序方式和顺序
bool needToSortDoctor(Docter* a, Docter* b, int choice, int order);

//交换数据
void swapDoctors(Docter* a, Docter* b);

//根据用户选择的排序方式和顺序，对医生链表进行排序
void sortDoctorList(Docter* head, Docter* tail, int choice, int order);

//排序菜单界面
void doctorSortMenu(HIS_System* sys);

#endif // !DOCTERSORT_H
