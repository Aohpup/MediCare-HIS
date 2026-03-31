#define _CRT_SECURE_NO_WARNINGS
#include"DepartmentSort.h"
#include<string.h>
#include"InputUtils.h"

// 根据用户选择的排序方式和顺序，确定是否需要交换两个科室节点
bool needToSortDepartment(Department* a, Department* b, int choice, int order) {
	if (choice == SORT_BY_CATEGORY) {
		int cmpResult = strcmp(a->categoryName, b->categoryName);
		return (order == ORDER_ASC) ? (cmpResult > 0) : (cmpResult < 0);
	}
	else if (choice == SORT_BY_CATEGORY_ID) {
		int cmpResult = strcmp(a->categoryId, b->categoryId);
		return (order == ORDER_ASC) ? (cmpResult > 0) : (cmpResult < 0);
	}
	else if (choice == SORT_BY_SUBDEPT) {
		char* subA = a->subDeptHead ? a->subDeptHead->subDeptName : "";	//如果没有二级科室，视为最小值
		char* subB = b->subDeptHead ? b->subDeptHead->subDeptName : "";
		int cmpResult = strcmp(subA, subB);
		return (order == ORDER_ASC) ? (cmpResult > 0) : (cmpResult < 0);
	}
	else if (choice == SORT_BY_ID) {
		char* idA = a->subDeptHead ? a->subDeptHead->subDeptId : "";	// 如果没有二级科室，则视为ID为空字符串，排在前面
		char* idB = b->subDeptHead ? b->subDeptHead->subDeptId : "";
		int cmpResult = strcmp(idA, idB);
		return (order == ORDER_ASC) ? (cmpResult > 0) : (cmpResult < 0);
	}
	return false; // 默认不交换
}

// 交换两个科室节点的数据
void swapDepartments(Department* a, Department* b) {
	Department temp = *a;
	*a = *b;
	*b = temp;
	// 交换后需要修正next指针
	Department* tempNext = a->next;
	a->next = b->next;
	b->next = tempNext;
}

// 根据用户选择的排序方式和顺序，确定是否需要交换两个二级科室节点
static bool needToSortSubDepartment(SubDepartment* a, SubDepartment* b, int choice, int order) {
	int cmpResult = 0;
	if (choice == SORT_BY_SUBDEPT) {
		cmpResult = strcmp(a->subDeptName, b->subDeptName);
	}
	else if (choice == SORT_BY_ID) {
		cmpResult = strcmp(a->subDeptId, b->subDeptId);
	}
	else {
		return false;
	}
	return (order == ORDER_ASC) ? (cmpResult > 0) : (cmpResult < 0);
}

// 交换两个二级科室节点的数据和next指针
static void swapSubDepartments(SubDepartment* a, SubDepartment* b) {
	SubDepartment temp = *a;
	*a = *b;
	*b = temp;

	SubDepartment* tempNext = a->next;
	a->next = b->next;
	b->next = tempNext;
}

// 根据用户选择的排序方式和顺序，对二级科室链表进行冒泡排序
static void sortSubDepartmentList(SubDepartment* head, SubDepartment* tail, int choice, int order) {
	if (head == NULL || head->next == NULL) {
		return;
	}
	bool swapped;
	do {
		swapped = false;
		SubDepartment* curr = head;
		while (curr->next != tail) {
			if (needToSortSubDepartment(curr, curr->next, choice, order)) {
				swapSubDepartments(curr, curr->next);
				swapped = true;
			}
			curr = curr->next;
		}
		tail = curr;
	} while (swapped);
}

// 根据用户选择的排序方式和顺序，对科室链表进行冒泡排序
// 并在需要时增加二级科室链表排序
void sortDepartmentList(Department* head, Department* tail, int choice, int order) {
	if (head == NULL) {
		return;
	}

	if (choice == SORT_BY_SUBDEPT || choice == SORT_BY_ID) {
		Department* deptCurr = head;
		while (deptCurr != NULL) {
			sortSubDepartmentList(deptCurr->subDeptHead, NULL, choice, order);
			deptCurr = deptCurr->next;
		}
	}

	if (head->next == NULL) {
		return;
	}
	bool swapped;
	do {
		swapped = false;
		Department* curr = head;
		while (curr->next != tail) {
			if (needToSortDepartment(curr, curr->next, choice, order)) {
				swapDepartments(curr, curr->next);
				swapped = true;
			}
			curr = curr->next;
		}
		tail = curr;
	} while (swapped);
}

// 排序菜单界面
void departmentSortMenu(HIS_System* sys) {
	if (sys->deptHead == NULL) {
		printf("\n>>> 系统内没有科室数据！\n");
		return;
	}
	int choice, order;
	while (1) {
		printf("\n--- 科室信息排序 ---\n");
		printf("1. 按一级科室名称排序\n");
		printf("2. 按一级科室代码排序\n");
		printf("3. 按二级科室名称排序\n");
		printf("4. 按诊室编号排序\n");
		printf("0. 取消排序\n");
		choice = safeGetInt("请选择排序方式: ");
		if (choice == SORT_EXIT) break;
		if (choice != SORT_BY_CATEGORY && choice != SORT_BY_CATEGORY_ID && choice != SORT_BY_SUBDEPT && choice != SORT_BY_ID) {
			printf(">>> 无效选择，请重新输入！\n");
			continue;
		}
		printf("请选择排序顺序:\n");
		printf("1. 升序\n");
		printf("2. 降序\n");
		printf("0. 取消排序\n");
		order = safeGetInt("请输入您的选择: ");
		if (order == ORDER_EXIT) break;
		if (order != ORDER_ASC && order != ORDER_DESC) {
			printf(">>> 无效选择，请重新输入！\n");
			continue;
		}

		const char* options[] = { "按一级科室名称", "按一级科室代码", "按二级科室名称", "按科室编号" };
		printf("\n>>> 已选择排序方式: %s\n", options[choice - 1]);
		printf(">>> 已选择排序顺序: %s\n\n", (order == ORDER_ASC) ? "升序" : "降序");

		sortDepartmentList(sys->deptHead, NULL, choice, order);
		printf("\n>>> 科室信息排序完成！\n");
		break;
	}
}