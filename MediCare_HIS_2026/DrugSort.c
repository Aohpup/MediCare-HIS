#define _CRT_SECURE_NO_WARNINGS

#include"DrugSort.h"
#include"HIS_System.h"
#include <string.h>
// 交换两个药品节点的数据和next指针
static void swapDrugs(Drug* a, Drug* b) {
	Drug temp = *a;
	*a = *b;
	*b = temp;

	Drug* tempNext = a->next;
	a->next = b->next;
	b->next = tempNext;
}

bool needToSortDrug(Drug* a, Drug* b, int choice, int order) {
	int cmpResult = 0;
	switch (choice) {
	case SORT_BY_ID:		//根据药品编号排序
		cmpResult = strcmp(a->drugId, b->drugId);
		break;
	case SORT_BY_GBCODE:	//根据国标码排序
		cmpResult = strcmp(a->drugGbCode, b->drugGbCode);
		break;
	case SORT_BY_GENNAME:	//根据通用名排序
		cmpResult = strcmp(a->genericName, b->genericName);
		break;
	case SORT_BY_TRADENAME:	//根据商品名排序
		cmpResult = strcmp(a->tradeName, b->tradeName);
		break;
	case SORT_BY_ALIAS:		//根据别名排序
		cmpResult = strcmp(a->alias, b->alias);
		break;
	case SORT_BY_STOCK:		//根据库存排序
		cmpResult = (a->stock > b->stock) - (a->stock < b->stock); 
		break;
	case SORT_BY_PRICE:		//根据价格排序
		cmpResult = (a->price > b->price) - (a->price < b->price); 
		break;

	default:
		printf(">>> 无效的排序方式选择！\n");
		return false; 
	}

	if ((order == ORDER_ASC && cmpResult > 0) ||	// 升序时a应该在b后面
		(order == ORDER_DESC && cmpResult < 0)) {	// 降序时a应该在b前面
		return true;	// 需要交换
	} 
	else {
		return false;	// 不需要交换
	}
}

// 根据用户选择的排序方式和顺序，对药品链表进行冒泡排序
void sortDrugList(Drug* head, Drug* tail, int choice, int order) {
	if (head == NULL || head->next == NULL) {
		return; // 链表为空或只有一个节点，无需排序
	}
	bool swapped;
	do {
		swapped = false;
		Drug* current = head;
		while (current->next != tail) {
			if (needToSortDrug(current, current->next, choice, order)) {
				swapDrugs(current, current->next);
				swapped = true;
			}
			current = current->next;
		}
		tail = current; // 缩小冒泡范围(最后的节点已经排序完毕)
	} while (swapped);
}

//排序界面菜单
void drugSortMenu(HIS_System* sys) {
	if(sys->drugHead == NULL) {
		printf(">>> 系统内没有药品数据！\n");
		return;
	}
	int sortFieldChoice = -1, sortOrderChoice = -1;
	while (1) {
		printf("输入对应序号选择排序方式:\n");
		printf("1. 根据药品编号排序\n");
		printf("2. 根据国标码排序\n");
		printf("3. 根据通用名排序\n");
		printf("4. 根据商品名排序\n");
		printf("5. 根据别名排序\n");
		printf("6. 根据库存排序\n");
		printf("7. 根据价格排序\n");
		printf("0. 取消排序\n");
		sortFieldChoice = safeGetInt(">>> 请输入排序方式: \n");
		if (sortFieldChoice == SORT_EXIT) {
			printf(">>> 已取消排序。\n");
			break;
		}
		else if (sortFieldChoice < SORT_BY_ID || sortFieldChoice > SORT_BY_PRICE) {
			printf(">>> 无效选择，请重新输入！\n");
			continue;
		}
		
		printf("请输入对应序号选择排序顺序:\n");
		printf("1. 升序\n");
		printf("2. 降序\n");
		printf("0. 取消排序\n");
		sortOrderChoice = safeGetInt(">>> 请输入排序顺序: \n");
		if (sortOrderChoice == ORDER_EXIT) {
			printf(">>> 已取消排序。\n");
			break;
		}
		else if (sortOrderChoice < ORDER_ASC || sortOrderChoice > ORDER_DESC) {
			printf(">>> 无效选择，请重新输入！\n");
			continue;
		}

		const char* sortOptions[] = { "根据药品编号排序", "根据国标码排序", "根据通用名排序", "根据商品名排序", "根据别名排序", "根据库存排序", "根据价格排序" };
		printf("\n>>> 已选择排序方式: %s\n", sortOptions[sortFieldChoice - 1]);
		printf(">>> 已选择排序顺序: %s\n\n", sortOrderChoice == ORDER_ASC ? "升序" : "降序");

		sortDrugList(sys->drugHead, NULL, sortFieldChoice, sortOrderChoice);
		printf(">>> 药品列表排序完成！\n");
		break;
	}
}