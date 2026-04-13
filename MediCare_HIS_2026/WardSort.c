#define _CRT_SECURE_NO_WARNINGS
#include "WardSort.h"
#include "InputUtils.h"
#include <string.h>

static void swapWards(Ward* a, Ward* b) {
	Ward temp = *a;
	*a = *b;
	*b = temp;
	// 交换后需要修正next指针
	Ward* tempNext = a->next;
	a->next = b->next;
	b->next = tempNext;
}

static int countBeds(Ward* ward) {
	int c = 0; Bed* b = ward->bedListHead;
	while (b) { c++; b = b->next; }
	return c;
}
static int countOccupiedBeds(Ward* ward) {
	int c = 0; Bed* b = ward->bedListHead;
	while (b) { if (b->isOccupied) c++; b = b->next; }
	return c;
}

static int needSwap(Ward* a, Ward* b, int choice, int order) {
	int cmp = 0;
	switch (choice) {
	case WARD_SORT_BY_ID:
		cmp = strcmp(a->wardId, b->wardId); break;
	case WARD_SORT_BY_TYPE:
		cmp = (a->type > b->type) - (a->type < b->type); break;
	case WARD_SORT_BY_DEPT:
		cmp = strcmp(a->department, b->department); break;
	case WARD_SORT_BY_BED_TOTAL:
		cmp = countBeds(a) - countBeds(b); break;
	case WARD_SORT_BY_BED_FREE:
		cmp = (countBeds(a) - countOccupiedBeds(a)) - (countBeds(b) - countOccupiedBeds(b));
		break;
	case WARD_SORT_BY_BED_USED:
		cmp = countOccupiedBeds(a) - countOccupiedBeds(b); break;
	default:
		return 0;
	}
	return (order == WARD_ORDER_ASC) ? (cmp > 0) : (cmp < 0);	// 升序时a>b需要交换，降序时a<b需要交换
}

// 使用冒泡排序算法对病房链表进行排序
static void swapWardList(Ward* head, Ward* tail, int choice, int order) {
	if (head == NULL) {
		printf("\n>>> 系统内没有病房数据！\n");
		return;
	}
	else if(head->next == NULL) {
		return; // 只有一个节点，无需排序
	}
	bool swapped;
	do {
		swapped = false;
		Ward* curr = head;
		while (curr->next != tail) {
			if (needSwap(curr, curr->next, choice, order)) {
				swapWards(curr, curr->next);
				swapped = true;
			}
			curr = curr->next; // 只有不交换时才移动到下一个节点
		}
		tail = curr; // 每次冒泡后，最后一个节点是最大/最小的，不需要再比较
	} while (swapped);
}

void wardSortMenu(HIS_System* sys) {
	if (sys->wardHead == NULL) {
		printf(">>> 系统内没有病房数据！\n");
		return;
	}
	int choice, order;
	while (1) {
		printf("\n--- 病房信息排序 ---\n");
		printf("1. 按病房编号\n");
		printf("2. 按病房种类\n");
		printf("3. 按所属科室\n");
		printf("4. 按床位总数\n");
		printf("5. 按剩余床位数\n");
		printf("6. 按已使用床位数\n");
		printf("0. 取消排序\n");
		choice = safeGetInt("请选择排序方式: ");
		if (choice == WARD_SORT_EXIT) return;
		if (choice < WARD_SORT_BY_ID || choice > WARD_SORT_BY_BED_USED) {
			printf(">>> 无效选择，请重试！\n");
			continue;
		}
		printf("1. 升序\n2. 降序\n0. 取消排序\n");
		order = safeGetInt("请选择排序顺序: ");
		if (order == WARD_ORDER_EXIT) return;
		if (order != WARD_ORDER_ASC && order != WARD_ORDER_DESC) {
			printf(">>> 无效选择，请重试！\n");
			continue;
		}

		const char* options[] = { "按病房编号", "按病房种类", "按所属科室", "按床位总数", "按剩余床位数", "按已使用床位数" };
		printf("\n>>> 已选择排序方式：%s\n", options[choice - 1]);
		printf(">>> 已选择排序顺序：%s\n\n", (order == WARD_ORDER_ASC) ? "升序" : "降序");

		swapWardList(sys->wardHead, NULL, choice, order);

		printf(">>> 病房排序完成！\n");
		return;
	}
}
