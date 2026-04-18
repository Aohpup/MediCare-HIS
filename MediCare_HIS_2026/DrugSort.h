#pragma once
#ifndef DRUGSORT_H
#define DRUGSORT_H
#include "HIS_System.h"

// 排序方式常量定义
#define SORT_EXIT         0		//退出排序
#define SORT_BY_ID        1		//根据药品编号排序
#define SORT_BY_GBCODE    2		//根据国标码排序
#define SORT_BY_GENNAME   3		//根据通用名排序
#define SORT_BY_TRADENAME 4		//根据商品名排序
#define SORT_BY_ALIAS     5		//根据别名排序
#define SORT_BY_STOCK     6		//根据库存排序
#define SORT_BY_PRICE     7		//根据价格排序

// 排序顺序常量定义
#define ORDER_EXIT 0			//退出排序
#define ORDER_ASC  1			//升序
#define ORDER_DESC 2			//降序

//排序界面菜单
void drugSortMenu(HIS_System* sys);

////排序界面菜单（医生视角）
void drugSortMenuDoc(HIS_System* sys);

//确定是否需要交换两个药品节点以满足用户选择的排序方式和顺序
bool needToSortDrug(Drug* a, Drug* b, int choice, int order);

//交换数据
void swapDrugs(Drug* a, Drug* b);

//根据用户选择的排序方式和顺序，对药品链表进行排序
void sortDrugList(Drug* head, Drug* tail, int choice, int order);

#endif // DRUGSORT_H