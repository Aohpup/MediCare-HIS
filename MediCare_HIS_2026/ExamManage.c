#define _CRT_SECURE_NO_WARNINGS
#include"ExamManage.h"
#include"ExamFileManage.h"
#include"InputUtils.h"
#include"ConfirmFunc.h"
#include<string.h>

static ExamItem* findExamItem(HIS_System* sys, const char* itemId) {
	ExamItem* curr = sys->examItemHead;
	while (curr != NULL) {
		if (strcmp(curr->itemId, itemId) == 0) {
			return curr;
		}
		curr = curr->next;
	}
	return NULL;
}

static ExamOrder* findExamOrder(HIS_System* sys, const char* orderId) {
	ExamOrder* curr = sys->examOrderHead;
	while (curr != NULL) {
		if (strcmp(curr->orderId, orderId) == 0) {
			return curr;
		}
		curr = curr->next;
	}
	return NULL;
}

static bool hasPendingItems(const ExamOrder* order) {
	ExamOrderItem* item = order->itemHead;
	while (item != NULL) {
		if (!item->finished) {
			return true;
		}
		item = item->next;
	}
	return false;
}

static void updateOrderStatus(ExamOrder* order) {
	if (order == NULL) {
		return;
	}
	if (hasPendingItems(order)) {
		strcpy(order->status, "Pending");
	}
	else {
		strcpy(order->status, "Finished");
	}
}

static void generateExamOrderId(char* outId, const char* patientId) {
	const char* suffix = patientId;
	int len = (int)strlen(patientId);
	if (len > 4) {
		suffix = patientId + (len - 4);
	}
	sprintf(outId, "X%s%03d", suffix, rand() % 1000);
}

bool createExamOrder(HIS_System* sys, const char* doctorId, const char* patientId) {
	if (sys == NULL || doctorId == NULL) {
		return false;
	}
	if (sys->examItemHead == NULL) {
		printf(">>> 当前没有检查项目字典，请先维护检查项目。\n");
		return false;
	}
	char selectedPatientId[ID_LEN];
	if (patientId == NULL) {
		safeGetString(">>> 请输入患者编号(输入 -1 取消): ", selectedPatientId, ID_LEN);
		if (strcmp(selectedPatientId, "-1") == 0) {
			printf(">>> 已取消开具检查单。\n");
			return false;
		}
	}
	else {
		strcpy(selectedPatientId, patientId);
	}

	ExamOrder* order = (ExamOrder*)malloc(sizeof(ExamOrder));
	if (order == NULL) {
		printf(">>> 内存不足，无法创建检查单。\n");
		return false;
	}
	memset(order, 0, sizeof(ExamOrder));
	generateExamOrderId(order->orderId, selectedPatientId);
	strcpy(order->patientId, selectedPatientId);
	strcpy(order->doctorId, doctorId);
	strcpy(order->date, getCurrentDateStr());
	strcpy(order->status, "Pending");
	order->itemHead = NULL;
	order->next = NULL;

	printf(">>> 请选择检查项目(输入 -1 结束)：\n");
	while (1) {
		char itemId[ID_LEN];
		safeGetString(">>> 项目编号: ", itemId, ID_LEN);
		if (strcmp(itemId, "-1") == 0) {
			break;
		}
		ExamItem* dictItem = findExamItem(sys, itemId);
		if (dictItem == NULL) {
			printf(">>> 未找到项目编号 %s，请重试。\n", itemId);
			continue;
		}
		ExamOrderItem* newItem = (ExamOrderItem*)malloc(sizeof(ExamOrderItem));
		if (newItem == NULL) {
			printf(">>> 内存不足，无法添加项目。\n");
			break;
		}
		memset(newItem, 0, sizeof(ExamOrderItem));
		strcpy(newItem->itemId, dictItem->itemId);
		strcpy(newItem->itemName, dictItem->itemName);
		newItem->price = dictItem->price;
		newItem->finished = false;
		newItem->next = NULL;
		if (order->itemHead == NULL) {
			order->itemHead = newItem;
		}
		else {
			ExamOrderItem* tail = order->itemHead;
			while (tail->next != NULL) {
				tail = tail->next;
			}
			tail->next = newItem;
		}
		printf(">>> 已添加项目 [%s] %s\n", dictItem->itemId, dictItem->itemName);
	}

	if (order->itemHead == NULL) {
		printf(">>> 未选择任何检查项目，已取消开单。\n");
		free(order);
		return false;
	}

	if (sys->examOrderHead == NULL) {
		sys->examOrderHead = order;
	}
	else {
		ExamOrder* tail = sys->examOrderHead;
		while (tail->next != NULL) {
			tail = tail->next;
		}
		tail->next = order;
	}

	saveExamOrderData(sys);
	printf(">>> 检查单 %s 已开具。\n", order->orderId);
	return true;
}

void listPendingExamOrders(HIS_System* sys) {
	if (sys == NULL) {
		return;
	}
	ExamOrder* order = sys->examOrderHead;
	bool found = false;
	while (order != NULL) {
		if (hasPendingItems(order)) {
			printExamOrderDetail(order);
			found = true;
		}
		order = order->next;
	}
	if (!found) {
		printf(">>> 当前没有待执行检查单。\n");
	}
}

bool fillExamResult(HIS_System* sys, const char* orderId, const char* itemId, const char* result) {
	if (sys == NULL || orderId == NULL || itemId == NULL || result == NULL) {
		return false;
	}
	ExamOrder* order = findExamOrder(sys, orderId);
	if (order == NULL) {
		printf(">>> 未找到检查单 %s。\n", orderId);
		return false;
	}
	ExamOrderItem* item = order->itemHead;
	while (item != NULL) {
		if (strcmp(item->itemId, itemId) == 0) {
			strncpy(item->result, result, sizeof(item->result) - 1);
			item->result[sizeof(item->result) - 1] = '\0';
			item->finished = true;
			updateOrderStatus(order);
			saveExamOrderData(sys);
			printf(">>> 已保存检查结果。\n");
			return true;
		}
		item = item->next;
	}
	printf(">>> 检查单中未找到项目编号 %s。\n", itemId);
	return false;
}

void queryExamOrdersByDoctor(HIS_System* sys, const char* doctorId) {
	if (sys == NULL || doctorId == NULL) {
		return;
	}
	ExamOrder* order = sys->examOrderHead;
	bool found = false;
	while (order != NULL) {
		if (strcmp(order->doctorId, doctorId) == 0) {
			printExamOrderDetail(order);
			found = true;
		}
		order = order->next;
	}
	if (!found) {
		printf(">>> 暂无该医生开具的检查单。\n");
	}
}

void queryExamOrdersByPatient(HIS_System* sys, const char* patientId) {
	if (sys == NULL || patientId == NULL) {
		return;
	}
	ExamOrder* order = sys->examOrderHead;
	bool found = false;
	while (order != NULL) {
		if (strcmp(order->patientId, patientId) == 0) {
			printExamOrderDetail(order);
			found = true;
		}
		order = order->next;
	}
	if (!found) {
		printf(">>> 暂无该患者的检查单。\n");
	}
}

void printExamOrderDetail(const ExamOrder* order) {
	if (order == NULL) {
		return;
	}
	printf("\n--- 检查单 [%s] 状态:%s ---\n", order->orderId, order->status);
	printf("患者:%s 医生:%s 日期:%s\n", order->patientId, order->doctorId, order->date);
	ExamOrderItem* item = order->itemHead;
	int idx = 1;
	while (item != NULL) {
		printf("%d) %s %s | 结果:%s | 完成:%s\n", idx++, item->itemId, item->itemName,
			item->result[0] == '\0' ? "(未填写)" : item->result,
			item->finished ? "是" : "否");
		item = item->next;
	}
}
