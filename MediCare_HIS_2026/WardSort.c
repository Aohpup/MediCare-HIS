#define _CRT_SECURE_NO_WARNINGS
#include "WardSort.h"
#include "InputUtils.h"
#include <string.h>

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
	return (order == WARD_ORDER_ASC) ? (cmp > 0) : (cmp < 0);
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

		Ward* tail = NULL;
		int swapped;
		do {
			swapped = 0;
			Ward* cur = sys->wardHead;
			while (cur && cur->next != tail) {
				if (needSwap(cur, cur->next, choice, order)) {
					Ward temp = *cur;
					*cur = *(cur->next);
					*(cur->next) = temp;
					Ward* tmpNext = cur->next->next;
					cur->next->next = cur->next;
					cur->next = tmpNext;
					swapped = 1;
				}
				cur = cur->next;
			}
			tail = cur;
		} while (swapped);

		printf(">>> 病房排序完成！\n");
		return;
	}
}
