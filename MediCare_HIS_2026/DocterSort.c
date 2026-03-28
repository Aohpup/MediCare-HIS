#define _CRT_SECURE_NO_WARNINGS
#include"DocterSort.h"
#include"InputUtils.h"
#include<string.h>

// 交换两个医生节点的数据和next指针
static void swapDoctors(Docter* a, Docter* b) {
	Docter temp = *a;
	*a = *b;
	*b = temp;
	// 交换后需要修正next指针
	Docter* tempNext = a->next;
	a->next = b->next;
	b->next = tempNext;
}

bool needToSortDoctor(Docter* a, Docter* b, int choice, int order) {
	int cmpResult = 0;
	switch (choice) {
	case SORT_BY_ID:		//根据医生编号排序
		cmpResult = strcmp(a->docterId, b->docterId);
		break;
	case SORT_BY_NAME:		//根据医生姓名排序
		cmpResult = strcmp(a->docterName, b->docterName);
		break;
	case SORT_BY_DEPT:		//根据医生所在科室排序
		cmpResult = strcmp(a->department, b->department);
		break;
	case SORT_BY_CONSULT:	//根据诊号数量排序
		cmpResult = (a->consultationCount > b->consultationCount) - (a->consultationCount < b->consultationCount);	// 结果为1表示a>b，-1表示a<b，0表示相等
		break;
	default:
		printf(">>> 无效的排序方式选择！\n");
		return false; 
	}

	// 升序时a应该在b后面，降序时a应该在b前面
	if ((order == ORDER_ASC && cmpResult > 0) ||	
		(order == ORDER_DESC && cmpResult < 0)) 	
		return true;		// 需要交换
	else 
		return false;		// 不需要交换
}

void sortDoctorList(Docter* head, Docter* tail, int choice, int order) {
	if (head == NULL || head->next == NULL) {
		return; // 链表为空或只有一个节点，无需排序
	}
	bool swapped;
	do {
		swapped = false;
		Docter* curr = head;
		while (curr->next != tail) {
			if (needToSortDoctor(curr, curr->next, choice, order)) {
				swapDoctors(curr, curr->next);
				swapped = true;
			}
			curr = curr->next;
		}
		tail = curr; // 每次冒泡后，最后一个节点已经是最大/最小，不再参与下一轮比较
	} while (swapped);// 当一轮比较没有发生交换时，说明链表已经有序，可以提前结束
}

//排序菜单界面
void doctorSortMenu(HIS_System* sys) {
	if (sys->docHead == NULL) {
		printf(">>> 系统内没有医生数据！\n");
		return;
	}
	while (1) {
		int choice, order;
		printf("\n--- 医生信息排序 ---\n");
		printf("请选择排序方式:\n");
		printf("1. 按医生编号排序\n");
		printf("2. 按医生姓名排序\n");
		printf("3. 按所在科室排序\n");
		printf("4. 按诊号数量排序\n");
		printf("0. 取消排序\n");
		choice = safeGetInt("请输入选择: ");
		if (choice == SORT_EXIT) {
			printf(">>>\n已取消排序。\n");
			break;
		}
		else if (choice < SORT_BY_ID || choice > SORT_BY_CONSULT) {
			printf(">>> 无效选择，请重新输入！\n");
			return;
		}
		printf("请输入对应序号选择排序顺序:\n");
		printf("1. 升序\n");
		printf("2. 降序\n");
		printf("0. 取消排序\n");
		order = safeGetInt(">>> 请输入排序顺序: \n");
		if (order == ORDER_ASC) order = ORDER_ASC;
		else if (order == ORDER_DESC) order = ORDER_DESC;
		else if (order == ORDER_EXIT) {
			printf(">>> 已取消排序。\n");
			break;
		}
		else {
			printf(">>> 无效选择，请重新输入！\n");
			continue;
		}

		const char* sortOptions[] = { "按医生编号排序", "按医生姓名排序", "按所在科室排序", "按诊号数量排序" };
		printf("\n>>> 已选择排序方式: %s\n", sortOptions[choice - 1]);
		printf(">>> 已选择排序顺序: %s\n\n", order == ORDER_ASC ? "升序" : "降序");

		sortDoctorList(sys->docHead, NULL, choice, order);
		printf(">>> 医生信息排序完成！\n");
		break;
	}
}

