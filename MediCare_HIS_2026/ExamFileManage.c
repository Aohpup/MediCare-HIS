#define _CRT_SECURE_NO_WARNINGS
#include"ExamFileManage.h"
#include"HIS_System.h"
#include<string.h>

//检查项目字典文件加载状态标志
bool is_Exam_Item_File_Loaded = false;
//检查申请/结果文件加载状态标志
bool is_Exam_Order_File_Loaded = false;

// 辅助函数：追加检查项目到系统链表
static ExamItem* appendExamItem(HIS_System* sys, const char* id, const char* name, double price, const char* department) {
	ExamItem* item = (ExamItem*)malloc(sizeof(ExamItem));
	if (item == NULL) {
		return NULL;
	}
	memset(item, 0, sizeof(ExamItem));
	strcpy(item->itemId, id);
	strcpy(item->itemName, name);
	item->price = price;
	if (department != NULL) {
		strcpy(item->department, department);
	}
	item->next = NULL;

	if (sys->examItemHead == NULL) {
		sys->examItemHead = item;
		return item;
	}
	ExamItem* tail = sys->examItemHead;
	while (tail->next != NULL) {
		tail = tail->next;
	}
	tail->next = item;
	return item;
}

// 辅助函数：追加检查申请到系统链表
static ExamOrder* appendExamOrder(HIS_System* sys, const ExamOrder* source) {
	ExamOrder* order = (ExamOrder*)malloc(sizeof(ExamOrder));
	if (order == NULL) {
		return NULL;
	}
	memset(order, 0, sizeof(ExamOrder));
	strcpy(order->orderId, source->orderId);
	strcpy(order->patientId, source->patientId);
	strcpy(order->doctorId, source->doctorId);
	strcpy(order->date, source->date);
	strcpy(order->status, source->status);
	order->itemHead = NULL;
	order->next = NULL;

	if (sys->examOrderHead == NULL) {
		sys->examOrderHead = order;
		return order;
	}
	ExamOrder* tail = sys->examOrderHead;
	while (tail->next != NULL) {
		tail = tail->next;
	}
	tail->next = order;
	return order;
}

// 辅助函数：追加检查申请项目到申请单链表
static ExamOrderItem* appendExamOrderItem(ExamOrder* order, const char* itemId, const char* itemName, double price, bool finished) {
	ExamOrderItem* item = (ExamOrderItem*)malloc(sizeof(ExamOrderItem));
	if (item == NULL) {
		return NULL;
	}
	memset(item, 0, sizeof(ExamOrderItem));
	strcpy(item->itemId, itemId);
	strcpy(item->itemName, itemName);
	item->price = price;
	item->finished = finished;
	item->next = NULL;

	if (order->itemHead == NULL) {
		order->itemHead = item;
		return item;
	}
	ExamOrderItem* tail = order->itemHead;
	while (tail->next != NULL) {
		tail = tail->next;
	}
	tail->next = item;
	return item;
}

//检查项目字典文件加载函数
void loadExamItemData(HIS_System* sys) {
	if (is_Exam_Item_File_Loaded) {
		return;
	}
	FILE* fp = fopen(EXAM_ITEM_FILE, "r");
	if (!fp) {
		printf(">>> 警告: 找不到 %s，将跳过检查项目加载。\n", EXAM_ITEM_FILE);
		return;
	}
	char line[512];
	if (fgets(line, sizeof(line), fp) != NULL) {
		if (line[0] != '#') {
			fseek(fp, 0, SEEK_SET);
		}
	}
	while (fgets(line, sizeof(line), fp) != NULL) {
		if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
			continue;
		}
		char id[ID_LEN] = { 0 };
		char name[STR_LEN] = { 0 };
		double price = 0.0;
		char dept[STR_LEN] = { 0 };
		if (sscanf(line, "I %24s %49s %lf %49s", id, name, &price, dept) >= 3) {
			appendExamItem(sys, id, name, price, dept);
		}
	}
	fclose(fp);
	is_Exam_Item_File_Loaded = true;
	printf(">>> 检查项目字典加载完成！\n");
}

//检查项目字典文件保存函数
void saveExamItemData(HIS_System* sys) {
	FILE* fp = fopen(EXAM_ITEM_FILE, "w");
	if (!fp) {
		printf(">>> 错误: 无法打开 %s 进行写入！\n", EXAM_ITEM_FILE);
		return;
	}
	fprintf(fp, "# HIS EXAM ITEM DATA FILE\n");
	ExamItem* curr = sys->examItemHead;
	while (curr != NULL) {
		fprintf(fp, "I %s %s %.2f %s\n", curr->itemId, curr->itemName, curr->price, curr->department);
		curr = curr->next;
	}
	fclose(fp);
	printf(">>> 检查项目字典保存完成！\n");
}

//检查申请/结果文件加载函数
void loadExamOrderData(HIS_System* sys) {
	if (is_Exam_Order_File_Loaded) {
		return;
	}
	FILE* fp = fopen(EXAM_ORDER_FILE, "r");
	if (!fp) {
		printf(">>> 警告: 找不到 %s，将跳过检查申请加载。\n", EXAM_ORDER_FILE);
		return;
	}
	char line[1024];
	if (fgets(line, sizeof(line), fp) != NULL) {
		if (line[0] != '#') {
			fseek(fp, 0, SEEK_SET);
		}
	}
	ExamOrder* currentOrder = NULL;
	ExamOrderItem* lastItem = NULL;
	while (fgets(line, sizeof(line), fp) != NULL) {
		if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
			continue;
		}
		if (strncmp(line, "O ", 2) == 0) {
			ExamOrder tmp;
			memset(&tmp, 0, sizeof(tmp));
			if (sscanf(line + 2, "%24s %24s %24s %19s %49s", tmp.orderId, tmp.patientId, tmp.doctorId, tmp.date, tmp.status) == 5) {
				currentOrder = appendExamOrder(sys, &tmp);
				lastItem = NULL;
			}
			continue;
		}
		if (strncmp(line, "D ", 2) == 0) {
			if (currentOrder == NULL) {
				continue;
			}
			char itemId[ID_LEN] = { 0 };
			char itemName[STR_LEN] = { 0 };
			double price = 0.0;
			int finished = 0;
			if (sscanf(line + 2, "%24s %49s %lf %d", itemId, itemName, &price, &finished) >= 3) {
				lastItem = appendExamOrderItem(currentOrder, itemId, itemName, price, finished != 0);
			}
			continue;
		}
		if (strncmp(line, "R ", 2) == 0) {
			if (lastItem != NULL) {
				char* result = line + 2;
				result[strcspn(result, "\r\n")] = '\0';
				strncpy(lastItem->result, result, sizeof(lastItem->result) - 1);
				lastItem->result[sizeof(lastItem->result) - 1] = '\0';
			}
			continue;
		}
		if (strncmp(line, "END", 3) == 0) {
			currentOrder = NULL;
			lastItem = NULL;
			continue;
		}
	}
	fclose(fp);
	is_Exam_Order_File_Loaded = true;
	printf(">>> 检查申请数据加载完成！\n");
}

//检查申请/结果文件保存函数
void saveExamOrderData(HIS_System* sys) {
	FILE* fp = fopen(EXAM_ORDER_FILE, "w");
	if (!fp) {
		printf(">>> 错误: 无法打开 %s 进行写入！\n", EXAM_ORDER_FILE);
		return;
	}
	fprintf(fp, "# HIS EXAM ORDER DATA FILE\n");
	ExamOrder* order = sys->examOrderHead;
	while (order != NULL) {
		fprintf(fp, "O %s %s %s %s %s\n", order->orderId, order->patientId, order->doctorId, order->date, order->status);
		ExamOrderItem* item = order->itemHead;
		while (item != NULL) {
			fprintf(fp, "D %s %s %.2f %d\n", item->itemId, item->itemName, item->price, item->finished ? 1 : 0);
			fprintf(fp, "R %s\n", item->result);
			item = item->next;
		}
		fprintf(fp, "END\n");
		order = order->next;
	}
	fclose(fp);
	printf(">>> 检查申请数据保存完成！\n");
}
