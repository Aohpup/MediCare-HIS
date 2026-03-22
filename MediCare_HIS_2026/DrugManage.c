#define _CRT_SECURE_NO_WARNINGS

#include"DrugManage.h"
#include"HIS_System.h"
#include"BufferClear.h"
#include"InputUtils.h"
#include"ProjectLimits.h"
#include"DrugFileManage.h"
#include"PrintFormattedStr.h"

//---------------------------------------------------
//药品编号防重复
static bool isDrugIdExist(Drug* head, const char* id) {
	Drug* curr = head;
	while (curr != NULL) {
		if (strcmp(curr->drugId, id) == 0) return true;
		curr = curr->next;
	}
	return false;
}

//药品国标码防重复
static bool isDrugGbCodeExist(Drug* head, const char* gbCode) {
	Drug* curr = head;
	while (curr != NULL) {
		if (strcmp(curr->drugGbCode, gbCode) == 0) return true;
		curr = curr->next;
	}
	return false;
}

//药品通用名防重复
static bool isDrugGenNameExist(Drug* head, const char* genName) {
	Drug* curr = head;
	while (curr != NULL) {
		if (strcmp(curr->genericName, genName) == 0) return true;
		curr = curr->next;
	}
	return false;
}
//药品商品名防重复
static bool isDrugTraNameExist(Drug* head, const char* traName) {
	Drug* curr = head;
	while (curr != NULL) {
		if (strcmp(curr->tradeName, traName) == 0) return true;
		curr = curr->next;
	}
	return false;
}
//药品别名防重复
static bool isDrugAliNameExist(Drug* head, const char* aliName) {
	Drug* curr = head;
	while (curr != NULL) {
		if (strcmp(curr->alias, aliName) == 0) return true;
		curr = curr->next;
	}
	return false;
}

//-----------------------------------------------------------------
//载入药品界面并提供管理菜单
void drugManageMenu(HIS_System* sys) {
	if(sys == NULL) {
		printf(">>> 严重错误: 系统底座未初始化！！！\n");
		return;
	}
	loadDrugSystemData(sys);   // 从文件加载数据

	int choice = -1;
	while (1) {
		printf("\n========== 药品管理中心 ==========\n");
		printf("1. 录入新药品\n");
		printf("2. 查询药品信息\n");
		printf("3. 修改药品数据\n");
		printf("4. 排序药品列表\n");
		printf("5. 删除药品记录\n");
		printf("6. 显示所有药品信息\n");
		printf("7. 保存系统数据\n");
		printf("0. 返回上一级菜单\n");
		printf("==================================\n");
		choice = safeGetInt("请选择药品管理操作: ");

		switch (choice) {
		case 1:
			addDrug(sys);
			break;
		case 2:
			queryDrug(sys);
			break;
		case 3:
			modifyDrug(sys);
			break;
		case 4:
			drugSortMenu(sys);
			break;
		case 5:
			deleteDrug(&sys);
			break;
		case 6:
			displayAllDrugs(sys);
			break;
		case 7:
			printf(">>> 正在保存药品系统数据...\n");
			saveDrugSystemData(sys);
			break;
		case 0:
			return;
		default:
			printf(">>> 无效选择，请重试。\n");
			break;
		}
	}
}

//-----------------------------------------------------------------

//录入新药品
void addDrug(HIS_System* sys) {
	while (1) {
		printf("\n--- 录入新药品 (在任意输入环节输入 '-1' 可取消本次添加) ---\n");
		Drug* newDrug = (Drug*)malloc(sizeof(Drug));
		if (!newDrug) {
			printf(">>> 错误：内存分配失败！\n");
			return;
		}

		bool hasCancelFlag = false;

		// 输入并验证药品编号
		while (1) {
			safeGetString("请输入药品编号:", newDrug->drugId, ID_LEN);
			if (strcmp(newDrug->drugId, "-1") == 0) {
				hasCancelFlag = true;
				break;
			}
			if (isDrugIdExist(sys->drugHead, newDrug->drugId)) {
				printf(">>> 错误：该药品编号已存在，请重新输入！\n");
				continue;
			}
			break;
		}
		if (hasCancelFlag) {
			free(newDrug);
			printf(">>> 已取消录入。\n");
			break; // 退出大循环，返回上一级菜单
		}

		// 输入并验证国标码
		while (1) {
			safeGetString("请输入国家药品本位码(14位国标码): ", newDrug->drugGbCode, 16);
			if (strcmp(newDrug->drugGbCode, "-1") == 0) {
				hasCancelFlag = true;
				break;
			}
			if (isDrugGbCodeExist(sys->drugHead, newDrug->drugGbCode)) {
				printf(">>> 错误：该国标码对应药品已存在，请重新输入！\n");
				continue;
			}
			break;
		}
		if (hasCancelFlag) {
			free(newDrug);
			printf(">>> 已取消录入。\n");
			break;
		}

		// 输入并验证通用名
		while (1) {
			safeGetString("请输入通用名: ", newDrug->genericName, STR_LEN);
			if (strcmp(newDrug->genericName, "-1") == 0) {
				hasCancelFlag = true;
				break;
			}
			if (isDrugGenNameExist(sys->drugHead, newDrug->genericName) ||
				isDrugTraNameExist(sys->drugHead, newDrug->genericName) ||
				isDrugAliNameExist(sys->drugHead, newDrug->genericName)) {
				printf(">>> 错误：该名称与系统内已有通用名冲突！\n");
				continue;
			}
			break;
		}
		if (hasCancelFlag) {
			free(newDrug);
			printf(">>> 已取消录入。\n");
			break;
		}

		// 输入商品名
		while (1) {
			safeGetString("请输入商品名: ", newDrug->tradeName, STR_LEN);
			if (strcmp(newDrug->tradeName, "-1") == 0) {
				hasCancelFlag = true;
				break;
			}
			if (isDrugGenNameExist(sys->drugHead, newDrug->tradeName) ||
				isDrugTraNameExist(sys->drugHead, newDrug->tradeName) ||
				isDrugAliNameExist(sys->drugHead, newDrug->tradeName)) {
				printf(">>> 错误：该名称与系统内已有商品名冲突！\n");
				continue;
			}
			break;
		}
		if (hasCancelFlag) {
			free(newDrug);
			printf(">>> 已取消录入。\n");
			break;
		}

		// 输入别名
		while (1) {
			safeGetString("请输入别名 (用于快速检索): ", newDrug->alias, STR_LEN);
			if (strcmp(newDrug->alias, "-1") == 0) {
				hasCancelFlag = true;
				break;
			}
			if (isDrugGenNameExist(sys->drugHead, newDrug->alias) ||
				isDrugTraNameExist(sys->drugHead, newDrug->alias) ||
				isDrugAliNameExist(sys->drugHead, newDrug->alias)) {
				printf(">>> 错误：该别名已存在！\n");
				continue;
			}
			break;
		}
		if (hasCancelFlag) {
			free(newDrug);
			printf(">>> 已取消录入。\n");
			break;
		}

		// 输入库存
		while(1){
			newDrug->stock = safeGetInt("请输入初始库存量: ");
			if (newDrug->stock == -1) {
				hasCancelFlag = true;
				break;
			}
			if (newDrug->stock < 0) {
				printf(">>> 错误：库存量不能为负数！\n");
				continue;
			}
			break;
		}
		if (hasCancelFlag) {
			free(newDrug);
			printf(">>> 已取消录入。\n");
			break;
		}

		// 输入单价
		while (1) {
			newDrug->price = safeGetDouble("请输入单价: ");
			if (newDrug->price <= -1.00 + 1e-6 && newDrug->price >= -1.00 - 1e-6) {
				hasCancelFlag = true;
				break;
			}
			if (newDrug->price < 0) {
				printf(">>> 错误：价格不能为负数！\n");
				continue;
			}
			break;
		}
		if (hasCancelFlag) {
			free(newDrug);
			printf(">>> 已取消录入。\n");
			break;
		}

		// 头插法
		newDrug->next = sys->drugHead;
		sys->drugHead = newDrug;

		printf(">>> 药品 <%s>(%s) 录入成功！\n", newDrug->genericName, newDrug->tradeName);
		
		// 提示是否继续添加
		printf(">>> 提示：继续添加请按回车；或直接输入 '-1' 可以在下一个编号输入时退出。\n");
	}
}

// 打印单条药品详细信息
void printDrugInfo(Drug* drug) {
	if (drug == NULL) return;
	
	char buffer[256];
	
	printf("\n----------------------------------------------------------------------------------------------------\n");
	// 表头
	printFormattedStr("药品编号", 12);
	printFormattedStr("国标码", 18);
	printFormattedStr("通用名", 18);
	printFormattedStr("商品名", 18);
	printFormattedStr("别名", 18);
	printFormattedStr("库存", 8);
	printFormattedStr("单价(元)", 10);
	printf("\n");
	
	// 分隔线
	printf("----------------------------------------------------------------------------------------------------\n");
	
	// 内容
	printFormattedStr(drug->drugId, 12);
	printFormattedStr(drug->drugGbCode, 18);
	printFormattedStr(drug->genericName, 18);
	printFormattedStr(drug->tradeName, 18);
	printFormattedStr(drug->alias, 18);
	
	// 数字转字符串后打印
	sprintf(buffer, "%d", drug->stock);
	printFormattedStr(buffer, 8);
	
	sprintf(buffer, "%.2f", drug->price);
	printFormattedStr(buffer, 10);
	
	printf("\n----------------------------------------------------------------------------------------------------\n");
}

//查询药品信息
void queryDrug(HIS_System* sys) {
	if(sys->drugHead == NULL) {
		printf(">>> 系统内没有药品数据！\n");
		return;
	}
	int choice;
	printf("\n--- 药品信息查询 ---\n");
	printf("1. 按药品编号查询\n");
	printf("2. 按国标码查询\n");
	printf("3. 按名称(通用名/商品名/别名)查询\n");
	printf("0. 返回上一级菜单\n");
	choice = safeGetInt("请选择查询方式: ");
	char queryStr[STR_LEN];
	Drug* curr = sys->drugHead;
	bool found = false;
	switch (choice) {
	case 1:
		safeGetString("请输入药品编号: ", queryStr, ID_LEN);
		while (curr != NULL) {
			if (strcmp(curr->drugId, queryStr) == 0) {
				printDrugInfo(curr);
				found = true;
				break;
			}
			curr = curr->next;
		}
		break;
	case 2:
		safeGetString("请输入国家药品本位码(16位国标码): ", queryStr, 16);
		while (curr != NULL) {
			if (strcmp(curr->drugGbCode, queryStr) == 0) {
				printDrugInfo(curr);
				found = true;
				break;
			}
			curr = curr->next;
		}
		break;
	case 3:
		safeGetString("请输入名称(通用名/商品名/别名): ", queryStr, STR_LEN);
		while (curr != NULL) {
			if (strcmp(curr->genericName, queryStr) == 0 ||
				strcmp(curr->tradeName, queryStr) == 0 ||
				strcmp(curr->alias, queryStr) == 0) {
				printDrugInfo(curr);
				found = true;
				break;
			}
			curr = curr->next;
		}
		break;
	case 0:
		return;
	default:
		printf(">>> 无效选择，请重试。\n");
		return;
	}
	if (!found) {
		printf(">>> 没有找到匹配的药品信息！\n");
	}
}

//修改药品信息
void modifyDrug(HIS_System* sys) {
	if(sys->drugHead == NULL) {
		printf(">>> 系统内没有药品数据！\n");
		return;
	}
	//TODO: 这里的修改功能实现较为复杂，涉及到数据验证和防止重复等逻辑，暂时以提示代替具体实现
}

//显示所有药品信息
void displayAllDrugs(HIS_System* sys) {
	if (sys->drugHead == NULL) {
		printf(">>> 系统内没有药品数据！\n");
		return;
	}

	char buffer[256];
	printf("\n=== 所有药品信息列表 ===\n");
	printf("----------------------------------------------------------------------------------------------------\n");
	// 表头
	printFormattedStr("药品编号", 12);
	printFormattedStr("国标码", 18);
	printFormattedStr("通用名", 20);
	printFormattedStr("商品名", 20);
	printFormattedStr("别名", 12);
	printFormattedStr("库存", 8);
	printFormattedStr("单价(元)", 10);
	printf("\n");
	printf("----------------------------------------------------------------------------------------------------\n");

	Drug* curr = sys->drugHead;
	while (curr != NULL) {
		// 内容列打印
		printFormattedStr(curr->drugId, 12);
		printFormattedStr(curr->drugGbCode, 18);
		printFormattedStr(curr->genericName, 20);
		printFormattedStr(curr->tradeName, 20);
		printFormattedStr(curr->alias, 12);

		sprintf(buffer, "%d", curr->stock);
		printFormattedStr(buffer, 8);

		sprintf(buffer, "%.2f", curr->price);
		printFormattedStr(buffer, 10);
		
		printf("\n");
		curr = curr->next;
	}
	printf("----------------------------------------------------------------------------------------------------\n");
	printf(">>> 药品列表打印完毕！\n");
}

//删除药品记录
void deleteDrug(HIS_System** sys) {
	while(1){
		if((*sys)->drugHead == NULL) {
			printf(">>> 系统内没有药品数据！\n");
			return;
		}
		printf("请选择删除方式:\n");
		printf("1. 按药品编号删除\n");
		printf("2. 按国标码删除\n");
		printf("0. 返回上一级菜单\n");
		int choice = safeGetInt("请输入选择: ");
		if (choice == 0) return; //返回上一级菜单
		else if(choice != 1 && choice != 2) {
			printf(">>> 无效选择，请重试！\n");
			continue;
		}
		char queryStr[STR_LEN];
		Drug* curr = (*sys)->drugHead;
		switch (choice)
		{
			case 1:
				safeGetString("请输入要删除的药品编号: ", queryStr, ID_LEN);
				deleteDrugFunc(&curr, queryStr, 1);
				break;
			case 2:
				safeGetString("请输入要删除的国家药品本位码(16位国标码): ", queryStr, 16);
				deleteDrugFunc(&curr, queryStr, 2);
				break;
			default:
				break;
		}

	}
}

//根据查询字符串和模式删除药品记录
void deleteDrugFunc(Drug** head, const char* queryStr, int mode) {
	Drug* curr = *head;
	Drug* prev = NULL;
	while (curr != NULL) {
		bool match = false;
		if (mode == 1 && strcmp(curr->drugId, queryStr) == 0) {				// 按药品编号删除
			match = true;
		}
		else if (mode == 2 && strcmp(curr->drugGbCode, queryStr) == 0) {	// 按国标码删除
			match = true;
		}
		if (match) {
			if (prev == NULL) { // 删除头节点
				*head = curr->next;
			}
			else {
				prev->next = curr->next;
			}
			printf(">>> 药品 <%s>(%s) 已删除！\n", curr->genericName, curr->tradeName);
			free(curr);
			return;
		}
		prev = curr;
		curr = curr->next;
	}
	printf(">>> 没有找到匹配的药品信息，删除失败！\n");
}