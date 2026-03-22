#pragma once
#ifndef DRUGSORT_H
#define DRUGSORT_H
#include "HIS_System.h"

// 排序方式常量定义
#define SORT_BY_ID        0		//根据药品编号排序
#define SORT_BY_GBCODE    1		//根据国标码排序
#define SORT_BY_GENNAME   2		//根据通用名排序
#define SORT_BY_TRADENAME 3		//根据商品名排序
#define SORT_BY_ALIAS     4		//根据别名排序
#define SORT_BY_STOCK     5		//根据库存排序
#define SORT_BY_PRICE     6		//根据价格排序
//#define SORT_BY_NONE     -1		//取消排序

// 排序顺序常量定义
#define ORDER_ASC  0			//升序
#define ORDER_DESC 1			//降序

//确定是否需要交换两个药品节点以满足用户选择的排序方式和顺序
static bool needToSort(Drug* a, Drug* b, int choice, int order);

//交换数据
void swapDrugs(Drug* a, Drug* b);

//根据用户选择的排序方式和顺序，对药品链表进行排序
void sortDrugList(Drug* head, Drug* tail, int choice, int order);

#endif // DRUGSORT_H