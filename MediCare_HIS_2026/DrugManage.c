#define _CRT_SECURE_NO_WARNINGS

#include"HIS_System.h"
#include"DrugManage.h"
#include"DrugSort.h"
#include"InputUtils.h"
#include"DrugFileManage.h"
#include"PrintFormattedStr.h"
#include"ConfirmFunc.h"

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
	// 默认按照药品编号升序显示
	sortDrugList(sys->drugDisplayHead, NULL, SORT_BY_ID, ORDER_ASC);
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
			queryDrug(sys, "admin");
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
			if (confirmFunc("保存", "药品系统数据")) {
				saveDrugSystemData(sys);
			}
			break;
		case 0:
			return;
		default:
			printf(">>> 无效选择，请重试。\n");
			break;
		}
	}
}

void drugManageMenuDoc(HIS_System* sys, const char* doctorId) {
	if (sys == NULL) {
		if (TEST_SYSTEM_DEBUG) {
			printf(">>> 系统底座未初始化！！！\n");
			return;
		}
		else {
			printf(">>> 严重错误: 系统底座未初始化！！！\n");
			exit(EXIT_FAILURE);
		}
	}
	loadDrugSystemData(sys);   // 从文件加载数据
	int choice = -1;
	while (1) {
		printf("\n========== 药品信息查询 ==========\n");
		printf("1. 查询药品信息\n");
		printf("2. 显示所有药品信息\n");
		printf("0. 返回上一级菜单\n");
		printf("==================================\n");
		choice = safeGetInt("请选择操作: ");
		switch (choice) {
		case 1:
			queryDrug(sys, "doctor");
			break;
		case 2:
			drugSortMenuDoc(sys);
			break;
		case 0:
			return;
		default:
			printf(">>> 无效选择，请重试。\n");
			break;
		}
	}
}

void drugManageMenuPat(HIS_System* sys, const char* patientId) {
	if(sys == NULL) {
		if (TEST_SYSTEM_DEBUG) {
			printf(">>> 系统底座未初始化！！！\n");
			return;
		}
		else {
			printf(">>> 严重错误: 系统底座未初始化！！！\n");
			exit(EXIT_FAILURE);
		}
	}
	loadDrugSystemData(sys);   // 从文件加载数据
	int choice = -1;
	while (1) {
		printf("\n========== 药品信息查询 ==========\n");
		printf("1. 查询药品信息\n");
		printf("2. 显示所有药品信息\n");
		printf("0. 返回上一级菜单\n");
		printf("==================================\n");
		choice = safeGetInt("请选择操作: ");
		switch (choice) {
		case 1:
			queryDrug(sys, "patient");
			break;
		case 2:
			displayAllDrugsPat(sys);
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
		refreshDrugDisplayList(sys);

		printf(">>> 药品 <%s>(%s) 录入成功！\n", newDrug->genericName, newDrug->tradeName);
		
		// 提示是否继续添加
		printf(">>> 提示：已自动继续添加药品；可直接输入 '-1' 可以在下一个编号输入时退出。\n");
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

void printDrugInfoDoc(Drug* drug) {
	if (drug == NULL) return;
	
	char buffer[256];
	
	printf("\n----------------------------------------------------------------------------------------------------\n");
	// 表头
	printFormattedStr("药品编号", 12);
	printFormattedStr("国标码", 18);
	printFormattedStr("通用名", 18);
	printFormattedStr("商品名", 18);
	printFormattedStr("别名", 18);
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
	sprintf(buffer, "%.2f", drug->price);
	printFormattedStr(buffer, 10);
	
	printf("\n----------------------------------------------------------------------------------------------------\n");
}

void printDrugInfoPat(Drug* drug) {
	if (drug == NULL) return;
	
	char buffer[256];
	
	printf("\n--------------------------------------------------------\n");
	// 表头
	printFormattedStr("药品编号", 12);
	printFormattedStr("通用名", 18);
	printFormattedStr("商品名", 18);
	printFormattedStr("单价(元)", 10);
	printf("\n");
	
	// 分隔线
	printf("--------------------------------------------------------\n");
	
	// 内容
	printFormattedStr(drug->drugId, 12);
	printFormattedStr(drug->genericName, 18);
	printFormattedStr(drug->tradeName, 18);
	
	// 数字转字符串后打印
	sprintf(buffer, "%.2f", drug->price);
	printFormattedStr(buffer, 10);
	
	printf("\n--------------------------------------------------------\n");
}

//查询药品信息
void queryDrug(HIS_System* sys, const char* userType) {
	if(sys->drugDisplayHead == NULL) {
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
	Drug* curr = sys->drugDisplayHead;
	bool found = false;
	switch (choice) {
	case 1:
		safeGetString("请输入药品编号: ", queryStr, ID_LEN);
		while (curr != NULL) {
			if (strcmp(curr->drugId, queryStr) == 0) {
				if(userType != NULL && strcmp(userType, "doctor") == 0) {
					printDrugInfoDoc(curr);
				}
				else if(userType != NULL && strcmp(userType, "patient") == 0) {
					printDrugInfoPat(curr);
				}
				else {
					printDrugInfo(curr);
				}
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
				if(userType != NULL && strcmp(userType, "doctor") == 0) {
					printDrugInfoDoc(curr);
				}
				else if(userType != NULL && strcmp(userType, "patient") == 0) {
					printDrugInfoPat(curr);
				}
				else {
					printDrugInfo(curr);
				}
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
				if(userType != NULL && strcmp(userType, "doctor") == 0) {
					printDrugInfoDoc(curr);
				}
				else if(userType != NULL && strcmp(userType, "patient") == 0) {
					printDrugInfoPat(curr);
				}
				else {
					printDrugInfo(curr);
				}
				found = true;
				break;
			}
			curr = curr->next;
		}
		break;
	case 0:
		return; // 返回上一级菜单
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

	int choice;
	printf("\n--- 修改药品信息 ---\n");
	printf("1. 按药品编号修改\n");
	printf("2. 按国标码修改\n");
	printf("3. 按名称(通用名/商品名/别名)修改\n");
	printf("0. 返回上一级菜单\n");
	choice = safeGetInt("请选择修改目标: ");

	if (choice == 0) return;
	if (choice != 1 && choice != 2 && choice != 3) {
		printf(">>> 无效选择，请重试。\n");
		return;
	}

	char queryStr[STR_LEN];
	Drug* matches[100];
	int matchCount = 0;
	Drug* curr = sys->drugHead;

	switch (choice) {
	case 1:
		safeGetString("请输入要修改的药品编号(输入 -1 取消): ", queryStr, ID_LEN);
		if (strcmp(queryStr, "-1") == 0) return;
		while (curr != NULL) {
			if (strcmp(curr->drugId, queryStr) == 0 && matchCount < 100) {
				matches[matchCount++] = curr;
			}
			curr = curr->next;
		}
		break;
	case 2:
		safeGetString("请输入要修改的国家药品本位码(16位国标码，输入 -1 取消): ", queryStr, 16);
		if (strcmp(queryStr, "-1") == 0) return;
		while (curr != NULL) {
			if (strcmp(curr->drugGbCode, queryStr) == 0 && matchCount < 100) {
				matches[matchCount++] = curr;
			}
			curr = curr->next;
		}
		break;
	case 3:
		safeGetString("请输入要修改的名称(通用名/商品名/别名，输入 -1 取消): ", queryStr, STR_LEN);
		if (strcmp(queryStr, "-1") == 0) return;
		while (curr != NULL) {
			if ((strcmp(curr->genericName, queryStr) == 0 || strcmp(curr->tradeName, queryStr) == 0 || strcmp(curr->alias, queryStr) == 0) && matchCount < 100) {
				matches[matchCount++] = curr;
			}
			curr = curr->next;
		}
		break;
	}

	if (matchCount == 0) {
		printf(">>> 没有找到匹配的药品信息！\n");
		return;
	}

	Drug* target = NULL;
	if (matchCount == 1) {
		target = matches[0];
		printf(">>> 已定位到目标药品：\n");
		printDrugInfo(target);
	}
	else {
		printf("\n>>> 发现 %d 条匹配记录，请选择要修改的目标：\n", matchCount);
		for (int i = 0; i < matchCount; i++) {
			printf("\n#[%d]\n", i + 1);
			printDrugInfo(matches[i]);
		}
		while (1) {
			int sel = safeGetInt("请输入对应序号(输入 -1 取消): ");
			if (sel == -1) return;
			if (sel >= 1 && sel <= matchCount) {
				target = matches[sel - 1];
				break;
			}
			printf(">>> 序号无效，请重试。\n");
		}
	}

	printf("\n1. 修改药品编号\n");
	printf("2. 修改国标码\n");
	printf("3. 修改通用名\n");
	printf("4. 修改商品名\n");
	printf("5. 修改别名\n");
	printf("6. 修改库存\n");
	printf("7. 修改价格\n");
	int modChoice = safeGetInt("请选择要修改的字段(输入 -1 取消): ");
	if (modChoice == -1) { printf(">>> 已取消修改。\n"); return; }

	if (!confirmFunc("修改", "药品信息")) {
		printf(">>> 已取消修改。\n");
		return;
	}

	switch (modChoice) {
	case 1: {
		char newId[ID_LEN];
		while (1) {
			safeGetString("请输入新的药品编号(输入 -1 取消): ", newId, ID_LEN);
			if (strcmp(newId, "-1") == 0) break;
			if (strcmp(newId, target->drugId) == 0) {
				printf(">>> 新编号与原编号一致，无需修改。\n");
				break;
			}
			if (isDrugIdExist(sys->drugHead, newId)) {
				printf(">>> 错误：该药品编号已存在，请重新输入！\n");
				continue;
			}
			strcpy(target->drugId, newId);
			printf(">>> 药品编号修改成功！\n");
			break;
		}
		break;
	}
	case 2: {
		char newGb[16];
		while (1) {
			safeGetString("请输入新的国家药品本位码(16位国标码，输入 -1 取消): ", newGb, 16);
			if (strcmp(newGb, "-1") == 0) break;
			if (strcmp(newGb, target->drugGbCode) == 0) {
				printf(">>> 新国标码与原国标码一致，无需修改。\n");
				break;
			}
			if (isDrugGbCodeExist(sys->drugHead, newGb)) {
				printf(">>> 错误：该国标码已存在，请重新输入！\n");
				continue;
			}
			strcpy(target->drugGbCode, newGb);
			printf(">>> 国标码修改成功！\n");
			break;
		}
		break;
	}
	case 3: {
		char newName[STR_LEN];
		while (1) {
			safeGetString("请输入新的通用名(输入 -1 取消): ", newName, STR_LEN);
			if (strcmp(newName, "-1") == 0) break;
			if (strcmp(newName, target->genericName) == 0) {
				printf(">>> 新通用名与原通用名一致，无需修改。\n");
				break;
			}
			if (isDrugGenNameExist(sys->drugHead, newName) || isDrugTraNameExist(sys->drugHead, newName) || isDrugAliNameExist(sys->drugHead, newName)) {
				printf(">>> 错误：该名称与系统内已有名称冲突，请重新输入！\n");
				continue;
			}
			strcpy(target->genericName, newName);
			printf(">>> 通用名修改成功！\n");
			break;
		}
		break;
	}
	case 4: {
		char newName[STR_LEN];
		while (1) {
			safeGetString("请输入新的商品名(输入 -1 取消): ", newName, STR_LEN);
			if (strcmp(newName, "-1") == 0) break;
			if (strcmp(newName, target->tradeName) == 0) {
				printf(">>> 新商品名与原商品名一致，无需修改。\n");
				break;
			}
			if (isDrugGenNameExist(sys->drugHead, newName) || isDrugTraNameExist(sys->drugHead, newName) || isDrugAliNameExist(sys->drugHead, newName)) {
				printf(">>> 错误：该名称与系统内已有名称冲突，请重新输入！\n");
				continue;
			}
			strcpy(target->tradeName, newName);
			printf(">>> 商品名修改成功！\n");
			break;
		}
		break;
	}
	case 5: {
		char newAlias[STR_LEN];
		while (1) {
			safeGetString("请输入新的别名(输入 -1 取消): ", newAlias, STR_LEN);
			if (strcmp(newAlias, "-1") == 0) break;
			if (strcmp(newAlias, target->alias) == 0) {
				printf(">>> 新别名与原别名一致，无需修改。\n");
				break;
			}
			if (isDrugGenNameExist(sys->drugHead, newAlias) || isDrugTraNameExist(sys->drugHead, newAlias) || isDrugAliNameExist(sys->drugHead, newAlias)) {
				printf(">>> 错误：该名称与系统内已有名称冲突，请重新输入！\n");
				continue;
			}
			strcpy(target->alias, newAlias);
			printf(">>> 别名修改成功！\n");
			break;
		}
		break;
	}
	case 6: {
		while (1) {
			int newStock = safeGetInt("请输入新的库存量(输入 -1 取消): ");
			if (newStock == -1) break;
			if (newStock < 0) {
				printf(">>> 错误：库存量不能为负数！\n");
				continue;
			}
			target->stock = newStock;
			printf(">>> 库存修改成功！\n");
			break;
		}
		break;
	}
	case 7: {
		while (1) {
			double newPrice = safeGetDouble("请输入新的单价(输入 -1 取消): ");
			if (newPrice <= -1.00 + 1e-6 && newPrice >= -1.00 - 1e-6) break;
			if (newPrice < 0) {
				printf(">>> 错误：价格不能为负数！\n");
				continue;
			}
			target->price = newPrice;
			printf(">>> 单价修改成功！\n");
			break;
		}
		break;
	}
	default:
		printf(">>> 无效选择，已取消修改。\n");
		break;
	}

	refreshDrugDisplayList(sys);
}

//显示所有药品信息
void displayAllDrugs(HIS_System* sys) {
	if (sys->drugDisplayHead == NULL) {
		printf(">>> 系统内没有药品数据！\n");
		return;
	}

	char buffer[256];
	int number = 1;

	printf("\n=== 所有药品信息列表 ===\n");
	printf("----------------------------------------------------------------------------------------------------\n");
	// 表头
	printFormattedStr("序号", 6);
	printFormattedStr("药品编号", 12);
	printFormattedStr("国标码", 18);
	printFormattedStr("通用名", 20);
	printFormattedStr("商品名", 15);
	printFormattedStr("别名", 12);
	printFormattedStr("库存", 8);
	printFormattedStr("单价(元)", 10);
	printf("\n");
	printf("----------------------------------------------------------------------------------------------------\n");

	Drug* curr = sys->drugDisplayHead;
	while (curr != NULL) {
		// 内容列打印
		sprintf(buffer, "#%d", number++);
		printFormattedStr(buffer, 6);

		printFormattedStr(curr->drugId, 12);
		printFormattedStr(curr->drugGbCode, 18);
		printFormattedStr(curr->genericName, 20);
		printFormattedStr(curr->tradeName, 15);
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

void displayAllDrugsDoc(HIS_System* sys) {
	if (sys->drugDisplayHead == NULL) {
		printf(">>> 系统内没有药品数据！\n");
		return;
	}

	char buffer[256];
	int number = 1;

	printf("\n=== 所有药品信息列表 ===\n");
	printf("----------------------------------------------------------------------------------------------------\n");
	// 表头
	printFormattedStr("序号", 6);
	printFormattedStr("药品编号", 12);
	printFormattedStr("国标码", 18);
	printFormattedStr("通用名", 20);
	printFormattedStr("商品名", 15);
	printFormattedStr("别名", 12);
	printFormattedStr("单价(元)", 10);
	printFormattedStr("状态", 10);
	printf("\n");
	printf("----------------------------------------------------------------------------------------------------\n");
	Drug* curr = sys->drugDisplayHead;
	while (curr != NULL) {
		// 内容列打印
		sprintf(buffer, "#%d", number++);
		printFormattedStr(buffer, 6);
		printFormattedStr(curr->drugId, 12);
		printFormattedStr(curr->drugGbCode, 18);
		printFormattedStr(curr->genericName, 20);
		printFormattedStr(curr->tradeName, 15);
		printFormattedStr(curr->alias, 12);
		sprintf(buffer, "%.2f", curr->price);
		printFormattedStr(buffer, 10);
		
		if(curr->stock > 0) {
			printFormattedStr("正常", 10);
		}
		else {
			printFormattedStr("缺货", 10);
		}

		printf("\n");
		curr = curr->next;
	}
	printf("----------------------------------------------------------------------------------------------------\n");
	printf(">>> 药品列表打印完毕！\n");
}

void displayAllDrugsPat(HIS_System* sys) {
	if (sys->drugDisplayHead == NULL) {
		printf(">>> 系统内没有药品数据！\n");
		return;
	}

	char buffer[256];
	int number = 1;

	printf("\n=== 所有药品信息列表 ===\n");
	printf("--------------------------------------------------------\n");
	// 表头
	printFormattedStr("序号", 6);
	printFormattedStr("通用名", 20);
	printFormattedStr("商品名", 15);
	printFormattedStr("单价(元)", 10);
	printf("\n");
	printf("--------------------------------------------------------\n");

	Drug* curr = sys->drugDisplayHead;
	while (curr != NULL) {
		// 内容列打印
		sprintf(buffer, "#%d", number++);
		printFormattedStr(buffer, 6);

		printFormattedStr(curr->genericName, 20);
		printFormattedStr(curr->tradeName, 15);

		sprintf(buffer, "%.2f", curr->price);
		printFormattedStr(buffer, 10);

		printf("\n");
		curr = curr->next;
	}
	printf("--------------------------------------------------------\n");
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
				(*sys)->drugHead = curr;
				refreshDrugDisplayList(*sys);
				break;
			case 2:
				safeGetString("请输入要删除的国家药品本位码(16位国标码): ", queryStr, 16);
				deleteDrugFunc(&curr, queryStr, 2);
				(*sys)->drugHead = curr;
				refreshDrugDisplayList(*sys);
				break;
			default:
				printf(">>> 无效选择，请重试！\n");
				break;
		}

	}
}

//根据查询字符串和模式删除药品信息
//TODO:不正确的confirmFunc调用位置，后续需要调整
void deleteDrugFunc(Drug** head, const char* queryStr, int mode) {
	confirmFunc("删除", "药品信息");
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