#define _CRT_SECURE_NO_WARNINGS

#include"HIS_System.h"
#include"DepartmentManage.h"
#include"DepartmentFileManage.h"
#include"DepartmentSort.h"
#include"InputUtils.h"
#include"PrintFormattedStr.h"
#include"PauseUtil.h"
#include"ConfirmFunc.h"
#include<string.h>

// 检查一级科室名称是否存在
bool isDepartmentNameExist(Department* head, const char* name) {
	Department* curr = head;
	while (curr != NULL) {
		if (strcmp(curr->categoryName, name) == 0) return true;
		curr = curr->next;
	}
	return false;
}

// 检查二级科室名称是否存在
bool isSubDepartmentNameExist(Department* head, const char* name) {
	Department* curr = head;
	while (curr != NULL) {
		SubDepartment* subCurr = curr->subDeptHead;
		while (subCurr != NULL) {
			if (strcmp(subCurr->subDeptName, name) == 0) return true;
			subCurr = subCurr->next;
		}
		curr = curr->next;
	}
	return false;
}

// 检查科室编号是否存在 (全局唯一)
bool isDepartmentIdExist(Department* head, const char* id) {
	Department* curr = head;
	while (curr != NULL) {
		SubDepartment* subCurr = curr->subDeptHead;
		while (subCurr != NULL) {
			if (strcmp(subCurr->subDeptId, id) == 0) return true;
			subCurr = subCurr->next;
		}
		curr = curr->next;
	}
	return false;
}

// 检查一级科室代码是否存在
bool isCategoryIdExist(Department* head, const char* id) {
	Department* curr = head;
	while (curr != NULL) {
		if (strcmp(curr->categoryId, id) == 0) return true;
		curr = curr->next;
	}
	return false;
}

// 统计某二级科室编号下的医生数量（为可能的“多医生-单科室”扩展预留）
int countDoctorsBySubDeptId(HIS_System* sys, const char* subDeptId) {
	if (sys == NULL || subDeptId == NULL) return 0;
	int count = 0;
	doctor* docCurr = sys->docHead;
	while (docCurr != NULL) {
		if (strcmp(docCurr->subDeptId, subDeptId) == 0) {
			count++;
		}
		docCurr = docCurr->next;
	}
	return count;
}

//打印诊室下医生列表
static void printDoctorsBySubDeptId(HIS_System* sys, const char* subDeptId, int count) {
	if (sys == NULL || subDeptId == NULL) return;
	if(count == 0) {
		printf("    - 该诊室下没有医生 -\n");
		return;
	}
	int newCount = 1;
	doctor* docCurr = sys->docHead;
	while (docCurr != NULL) {
		if (strcmp(docCurr->subDeptId, subDeptId) == 0) {
			if(newCount > count) {
				printf(">>> 警告: 统计的医生数量与实际不符，可能存在数据异常，请联系管理员！\n");
				return;
			}
			printf("    - #%d: 医生编号: %s, 医生姓名: %s -\n", newCount++, docCurr->doctorId, docCurr->doctorName);
		}
		docCurr = docCurr->next;
	}
}

// 当二级科室编号修改时，联动更新医生端绑定字段
static void rebindDoctorsSubDeptId(HIS_System* sys, const char* oldSubDeptId, const char* newSubDeptId) {
	if (sys == NULL || oldSubDeptId == NULL || newSubDeptId == NULL) return;
	doctor* docCurr = sys->docHead;
	while (docCurr != NULL) {
		if (strcmp(docCurr->subDeptId, oldSubDeptId) == 0) {
			strcpy(docCurr->subDeptId, newSubDeptId);
		}
		docCurr = docCurr->next;
	}
}

// 当二级科室被删除时，清空医生端绑定（避免悬挂引用）
static void clearDoctorsSubDeptBinding(HIS_System* sys, const char* subDeptId) {
	if (sys == NULL || subDeptId == NULL) return;
	doctor* docCurr = sys->docHead;
	while (docCurr != NULL) {
		if (strcmp(docCurr->subDeptId, subDeptId) == 0) {
			docCurr->subDeptId[0] = '\0';
			docCurr->department[0] = '\0';
		}
		docCurr = docCurr->next;
	}
}

//打印科室信息列表的表头
static void printDepartmentHeader() {
	printf("\n--- 科室信息列表 ---\n");
	printf("----------------------------------------------------------------\n");
	printFormattedStr("一级科室", 16);
	printFormattedStr("一级编号", 16);
	printFormattedStr("二级科室", 16);
	printFormattedStr("诊室编号", 16);
	printf("\n----------------------------------------------------------------\n");
}

// 打印单条二级科室内容
void printDepartmentInfo(Department* dept, SubDepartment* subDept) {
	printFormattedStr(dept->categoryName, 16);
	printFormattedStr(dept->categoryId, 16);
	printFormattedStr(subDept ? subDept->subDeptName : "无下属科室", 16);
	printFormattedStr(subDept ? subDept->subDeptId : "无下属诊室", 16);
	printf("\n");
}

void addDepartment(HIS_System* sys) {
	if(sys == NULL) {
		printf(">>> 错误：系统指针无效，无法添加科室！\n");
		return;
	}
	while (1) {
		printf("\n--- 录入新科室 (输入 '-1' 取消本次添加) ---\n");
		char tempCategoryName[STR_LEN];
		char tempCategoryId[ID_LEN];
		safeGetString("请输入一级科室名称: ", tempCategoryName, STR_LEN);

		if (strcmp(tempCategoryName, "-1") == 0) {
			printf(">>> 已退出科室添加。\n");
			return;
		}
		safeGetString("请输入一级科室代码: ", tempCategoryId, ID_LEN);
		if (strcmp(tempCategoryId, "-1") == 0) {
			printf(">>> 已退出科室添加。\n");
			return;
		}

		Department* targetDept = NULL;
		Department* curr = sys->deptHead;
		while (curr != NULL) {
			if (strcmp(curr->categoryName, tempCategoryName) == 0 && strcmp(curr->categoryId, tempCategoryId) == 0) {
				targetDept = curr;
				break;
			}
			curr = curr->next;
		}

		if (targetDept != NULL) {
			printf(">>> 提示：一级科室 <%s> 是已存在的科室！\n", targetDept->categoryName);
			char addSubChoice[STR_LEN];
			safeGetString(">>> 是否要在该科室下直接添加/匹配二级科室？(输入 'Y' 添加，其他键重新输入): ", addSubChoice, STR_LEN);

			if (addSubChoice[0] == 'Y' || addSubChoice[0] == 'y') {
				// 在已有的一级科室下添加二级科室
				SubDepartment* newSubDept = (SubDepartment*)malloc(sizeof(SubDepartment));
				if (!newSubDept) { printf(">>> 内存分配失败！\n"); return; }

				safeGetString("请输入新诊室所属二级科室名称: ", newSubDept->subDeptName, STR_LEN);
				if (strcmp(newSubDept->subDeptName, "-1") == 0) { free(newSubDept); continue; }

				safeGetString("请输入二级科室编号: ", newSubDept->subDeptId, ID_LEN);
				if (strcmp(newSubDept->subDeptId, "-1") == 0) { free(newSubDept); continue; }

				if (isDepartmentIdExist(sys->deptHead, newSubDept->subDeptId)) {
					printf(">>> 错误：科室编号已存在全局系统中，添加失败！\n");
					free(newSubDept);
					continue;
				}

				// 头插法接入二级链表
				newSubDept->next = targetDept->subDeptHead;
				targetDept->subDeptHead = newSubDept;
				printf(">>> <%s> - <%s> (编号: %s) 录入成功！\n", targetDept->categoryName, newSubDept->subDeptName, newSubDept->subDeptId);
			}
			else {
				continue; // 重新输入一级科室名称
			}
		}
		else {
			// 创建全新的一级科室及其第一个二级科室
			Department* newDepartment = (Department*)malloc(sizeof(Department));
			if (!newDepartment) { printf(">>> 内存分配失败！\n"); return; }
			if (isCategoryIdExist(sys->deptHead, tempCategoryId)) {
				printf(">>> 错误：一级科室代码已存在，添加失败！\n");
				free(newDepartment);
				continue;
			}
			strcpy(newDepartment->categoryName, tempCategoryName);
			strcpy(newDepartment->categoryId, tempCategoryId);
			newDepartment->subDeptHead = NULL;

			SubDepartment* newSubDept = (SubDepartment*)malloc(sizeof(SubDepartment));
			if (!newSubDept) { printf(">>> 内存分配失败！\n"); free(newDepartment); return; }

			safeGetString("请输入二级科室名称: ", newSubDept->subDeptName, STR_LEN);
			if (strcmp(newSubDept->subDeptName, "-1") == 0) { free(newSubDept); free(newDepartment); continue; }

			safeGetString("请输入二级科室编号: ", newSubDept->subDeptId, ID_LEN);
			if (strcmp(newSubDept->subDeptId, "-1") == 0) { free(newSubDept); free(newDepartment); continue; }

			if (isDepartmentIdExist(sys->deptHead, newSubDept->subDeptId)) {
				printf(">>> 错误：科室编号已存在全局系统中，添加失败！\n");
				free(newSubDept); free(newDepartment);
				continue;
			}

			newSubDept->next = NULL;
			newDepartment->subDeptHead = newSubDept;

			// 头插法接入一级链表
			newDepartment->next = sys->deptHead;
			sys->deptHead = newDepartment;
			printf(">>> 全新一级科室 <%s> (一级编号: %s) 及其二级科室 <%s> (编号: %s) 录入成功！\n",
				newDepartment->categoryName, newDepartment->categoryId, newSubDept->subDeptName, newSubDept->subDeptId);
		}
	}
}

void queryDepartment(HIS_System* sys) {
	if (sys->deptHead == NULL) {
		printf("\n>>> 系统内没有科室数据！\n");
		return;
	}

	//更改顺序
	int choice;
	printf("\n--- 科室信息查询 ---\n");
	printf("1. 按一级科室名称查询\n");
	printf("2. 按一级科室代码查询\n");
	printf("3. 按二级科室名称查询\n");
	printf("4. 按科室编号查询\n");
	printf("0. 返回上一级菜单\n");
	choice = safeGetInt("请选择查询方式: ");

	char queryStr[STR_LEN];
	Department* curr = sys->deptHead;
	bool found = false;
	int matchCount = 0;

	switch (choice) {
	case 1:
		safeGetString("请输入一级科室名称: ", queryStr, STR_LEN);
		bool headerPrinted = false; // 用于控制表头只打印一次
		while (curr != NULL) {
			if (strcmp(curr->categoryName, queryStr) == 0) {
				SubDepartment* subCurr = curr->subDeptHead;
				if (subCurr == NULL) {
					if (!headerPrinted) {
						printDepartmentHeader();
						headerPrinted = true;
					}
					printf("#%d:-------------------------------------------------------------\n", ++matchCount);
					printDepartmentInfo(curr, NULL);
					found = true;
				}
				while (subCurr != NULL) {
					if (!headerPrinted) {
						printDepartmentHeader();
						headerPrinted = true;
					}
					printf("#%d:-------------------------------------------------------------\n", ++matchCount);
					printDepartmentInfo(curr, subCurr);
					found = true;
					subCurr = subCurr->next;
				}
			}
			curr = curr->next;
		}
		break;
	case 2:
		safeGetString("请输入一级科室代码: ", queryStr, ID_LEN);
		while (curr != NULL) {
			if (strcmp(curr->categoryId, queryStr) == 0) {
				SubDepartment* subCurr = curr->subDeptHead;
				if (subCurr == NULL) {
					printf("#%d:-------------------------------------------------\n", ++matchCount);
					printDepartmentHeader();
					printDepartmentInfo(curr, NULL);
					found = true;
				}
				while (subCurr != NULL) {
					if (!found) {
						printDepartmentHeader();
					}
					printf("#%d:-------------------------------------------------\n", ++matchCount);
					printDepartmentInfo(curr, subCurr);
					found = true;
					subCurr = subCurr->next;
				}
			}
			curr = curr->next;
		}
		break;
	case 3:
		safeGetString("请输入二级科室名称: ", queryStr, STR_LEN);
		bool subHeaderPrinted = false; // 二级科室查询的表头控制
		while (curr != NULL) {
			SubDepartment* subCurr = curr->subDeptHead;
			while (subCurr != NULL) {
				if (strcmp(subCurr->subDeptName, queryStr) == 0) {
					if (!subHeaderPrinted) {
						printDepartmentHeader();
						subHeaderPrinted = true;
					}
					printf("#%d:-------------------------------------------------------------\n", ++matchCount);
					printDepartmentInfo(curr, subCurr);
					found = true;
				}
				subCurr = subCurr->next;
			}
			curr = curr->next;
		}
		break;
	case 4:
		safeGetString("请输入科室编号: ", queryStr, ID_LEN);
		while (curr != NULL) {
			SubDepartment* subCurr = curr->subDeptHead;
			while (subCurr != NULL) {
				if (strcmp(subCurr->subDeptId, queryStr) == 0) {
					printf(">>> 查找到唯一结果:\n");
					printDepartmentHeader();
					printDepartmentInfo(curr, subCurr);
					printf(">>> 当前关联医生数量: %d\n", countDoctorsBySubDeptId(sys, subCurr->subDeptId));
					found = true;
					break;
				}
				subCurr = subCurr->next;
			}
			if (found) break; // 编号唯一，找到即可退出
			curr = curr->next;
		}
		break;
	case 0: return;
	default: printf(">>> 无效选择，请重试！\n"); return;
	}

	if (!found) {
		printf(">>> 没有找到匹配的科室信息！\n");
	}
	else {
		printf("----------------------------------------------------------------\n");
		printf(">>> 共找到 %d 条相关记录。\n", matchCount);
	}
}

// 修改科室模块 (处理重名逻辑)
void modifyDepartment(HIS_System* sys) {
	if (sys->deptHead == NULL) {
		printf("\n>>> 系统内没有科室数据！\n");
		return;
	}

	int choice;
	printf("\n--- 修改科室信息 ---\n");
	printf("1. 按一级科室名称修改\n");
	printf("2. 按一级科室代码修改\n");
	printf("3. 按二级科室名称修改\n");
	printf("4. 按诊室编号修改\n");
	printf("0. 返回上一级菜单\n");
	printf("警告：修改科室信息可能会影响相关医生的绑定关系，请确保医生与诊室的绑定关系正确！\n");
	choice = safeGetInt("请选择修改目标: ");
	if (choice == -1 || choice == 0) return;

	char queryStr[STR_LEN];
	Department* curr = sys->deptHead;

	// 用于记录匹配项的指针数组，方便用户选择删除对应目标
	Department* deptMatches[100];
	SubDepartment* subDeptMatches[100];
	int matchCount = 0;

	if (choice == 1) {
		safeGetString("请输入要修改的一级科室名称:(在任意输入环节输入 '-1' 可取消本次修改) ", queryStr, STR_LEN);
		if (strcmp(queryStr, "-1") == 0) return;
		while (curr != NULL) {
			if (strcmp(curr->categoryName, queryStr) == 0) {
				deptMatches[matchCount++] = curr;
			}
			curr = curr->next;
		}

		if (matchCount == 0) {
			printf(">>> 未找到名为 <%s> 的一级科室！\n", queryStr);
			return;
		}

		int modifyMode = 1;
		while (1) { // 处理重名情况
			if (matchCount > 1) {
				printf("\n>>> 发现 %d 个同名的一级科室记录！\n", matchCount);
				printf("1. 批量修改：修改所有<%s>的一级科室\n", queryStr);
				printf("2. 精确修改：选择<%s>下特定的一个科室进行修改\n", queryStr);
				modifyMode = safeGetInt("请选择操作方式: ");
			}

			if (confirmFunc("修改", "一级科室名称")) {
				char newName[STR_LEN];
				safeGetString("请输入新的一级科室名称: ", newName, STR_LEN);
				if (strcmp(newName, "-1") == 0) { printf(">>> 已取消修改。\n"); break; }

				if (modifyMode == 1) {
					// 修改所有匹配项
					for (int i = 0; i < matchCount; i++) {
						strcpy(deptMatches[i]->categoryName, newName);
					}
					printf(">>> 成功将 %d 个一级科室重命名为 <%s>！\n", matchCount, newName);
					printf("注意：如果需要修改一级科室代码，请及时选择对应的修改选项！\n");
					break;
				}
				else if (modifyMode == 2) {
					// 展示列表供选择
					printf("\n>>> 请选择要修改的具体节点：\n");
					printDepartmentHeader();
					for (int i = 0; i < matchCount; i++) {
						printf("#[%d]:\n", i + 1);
						printDepartmentInfo(deptMatches[i], deptMatches[i]->subDeptHead);
					}

					while (1) { // 单独处理下用户输入的序号有效性 
						int sel = safeGetInt("请输入对应的序号: ");  
						if (sel >= 1 && sel <= matchCount) {
							strcpy(deptMatches[sel - 1]->categoryName, newName);
							printf(">>> 成功修改该节点的名称为 <%s>！\n", newName);
							printf("注意：如果需要修改一级科室代码，请及时选择对应的修改选项！\n");
							break;
						}
						else {
							printf(">>> 序号无效，请重新输入。\n\n");
							continue;
						}
					}
				}
				else {
					printf(">>> 无效的修改方式选择，请重新选择。\n\n");
					continue;
				}
			}
			else {
				printf(">>> 已取消修改。\n\n");
				break;
			}
			printf("严重错误：未正确处理修改流程，已退出修改模块！\n\n");
			break;
		}
	}
	else if (choice == 2) { //一级科室代码，代码唯一，直接修改所有匹配项，无需区分批量/精确修改
		safeGetString("请输入要修改的一级科室代码: (在任意输入环节输入 '-1' 可取消本次修改)", queryStr, ID_LEN);
		if (strcmp(queryStr, "-1") == 0) return;
		while (curr != NULL) {
			if (strcmp(curr->categoryId, queryStr) == 0) {
				deptMatches[matchCount++] = curr;
			}
			curr = curr->next;
		}
		if (matchCount == 0) {
			printf(">>> 未找到代码为 <%s> 的一级科室！\n", queryStr);
			return;
		}
		while (1) {
			char newId[ID_LEN];
			safeGetString("请输入新的一级科室代码: ", newId, ID_LEN);
			if (strcmp(newId, "-1") == 0) { printf(">>> 已取消修改。\n"); break; }
			if (isCategoryIdExist(sys->deptHead, newId)) {
				printf(">>> 错误：新一级科室代码已存在，修改失败！\n");
				continue;
			}
			for (int i = 0; i < matchCount; i++) {
				strcpy(deptMatches[i]->categoryId, newId);
			}
			printf(">>> 成功将 %d 个一级科室的代码修改为 <%s>！\n", matchCount, newId);
			break;
		}
	}
	else if (choice == 3) {
		// 修改二级科室名称 
		safeGetString("请输入要修改的二级科室名称(在任意输入环节输入 '-1' 可取消本次修改): ", queryStr, STR_LEN);
		if (strcmp(queryStr, "-1") == 0) return;
		while (curr != NULL) {
			SubDepartment* subCurr = curr->subDeptHead;
			while (subCurr != NULL) {
				if (strcmp(subCurr->subDeptName, queryStr) == 0) {
					deptMatches[matchCount] = curr;
					subDeptMatches[matchCount] = subCurr;
					matchCount++;
				}
				subCurr = subCurr->next;
			}
			curr = curr->next;
		}

		if (matchCount == 0) {
			printf(">>> 未找到名为 <%s> 的二级科室！\n", queryStr);
			return;
		}

		int modifyMode = 1;
		if (matchCount > 1) {
			printf("\n>>> 发现 %d 个同名的二级科室记录！\n", matchCount);
			printf("1. 批量修改：修改所有名为 <%s> 的二级科室\n", queryStr);
			printf("2. 精确修改：选择特定的一个进行修改\n");
			modifyMode = safeGetInt("请选择操作方式(1或2): ");
		}

		char newName[STR_LEN];
		safeGetString("请输入新的二级科室名称: ", newName, STR_LEN);
		if (strcmp(newName, "-1") == 0) { printf(">>> 已取消修改。\n"); return; }

		if (modifyMode == 1) {
			for (int i = 0; i < matchCount; i++) {
				strcpy(subDeptMatches[i]->subDeptName, newName);
			}
			printf(">>> 成功将 %d 个二级科室重命名为 <%s>！\n", matchCount, newName);
		}
		else if (modifyMode == 2) {
			printf("\n>>> 请选择要修改的具体节点：\n");
			printDepartmentHeader();
			for (int i = 0; i < matchCount; i++) {
				printf("[%d] ", i + 1);
				printDepartmentInfo(deptMatches[i], subDeptMatches[i]);
			}
			int sel = safeGetInt("请输入对应的序号: ");
			if (sel >= 1 && sel <= matchCount) {
				strcpy(subDeptMatches[sel - 1]->subDeptName, newName);
				printf(">>> 成功修改该二级科室名称为 <%s>！\n", newName);
			}
			else {
				printf(">>> 序号无效，修改取消。\n");
			}
		}
	}
	else if (choice == 4) {
		//按科室编号修改,由于编号全局唯一，无需区分批量/精确修改
		safeGetString("请输入要修改的科室编号(在任意输入环节输入 '-1' 可取消本次修改): ", queryStr, ID_LEN);
		if (strcmp(queryStr, "-1") == 0) return;
		SubDepartment* targetSubDept = NULL;
		Department* targetParentDept = NULL;

		while (curr != NULL) {
			SubDepartment* subCurr = curr->subDeptHead;
			while (subCurr != NULL) {
				if (strcmp(subCurr->subDeptId, queryStr) == 0) {
					targetSubDept = subCurr;
					targetParentDept = curr;
					break;
				}
				subCurr = subCurr->next;
			}
			if (targetSubDept != NULL) break;
			curr = curr->next;
		}

		if (!targetSubDept) {
			printf(">>> 未找到编号为 %s 的科室！\n", queryStr);
			return;
		}

		printf("\n>>> 查找到目标科室：\n");
		printDepartmentHeader();
		printDepartmentInfo(targetParentDept, targetSubDept);

		printf("\n1. 修改该科室名称\n");
		printf("2. 修改该科室编号\n");
		int subChoice = safeGetInt("请选择要修改的内容: ");
		if (subChoice == -1) return;

		if (subChoice == 1) {
			char newName[STR_LEN];
			safeGetString("请输入新的二级科室名称: ", newName, STR_LEN);
			if (strcmp(newName, "-1") != 0) {
				strcpy(targetSubDept->subDeptName, newName);
				printf(">>> 修改成功！\n");
			}
		}
		else if (subChoice == 2) {
			char newId[ID_LEN];
			safeGetString("请输入新的科室编号: ", newId, ID_LEN);
			if (strcmp(newId, "-1") == 0) return;
			if (isDepartmentIdExist(sys->deptHead, newId)) {
				printf(">>> 错误：新编号已存在，修改失败！\n");
			}
			else {
				char oldId[ID_LEN];
				strcpy(oldId, targetSubDept->subDeptId);
				strcpy(targetSubDept->subDeptId, newId);
				rebindDoctorsSubDeptId(sys, oldId, newId);
				printf(">>> 编号修改成功！并已自动同步医生端绑定关系。\n");
			}
		}
		else {
			printf(">>> 取消修改。\n");
		}
	}
	else if (choice == 0) {
		return;
	}
	else {
		printf(">>> 无效的选择！\n");
	}
}

// 辅助函数：安全删除一个一级科室节点 (会级联删除其下属的所有二级科室)
static void executeDeleteDepartment(HIS_System** sys, Department* targetDept) {
	if (targetDept == NULL) return;

	//从一级链表中断开该节点
	if ((*sys)->deptHead == targetDept) {
		(*sys)->deptHead = targetDept->next;
	}
	else {
		Department* prev = (*sys)->deptHead;
		while (prev != NULL && prev->next != targetDept) {
			prev = prev->next;
		}
		if (prev != NULL) {
			prev->next = targetDept->next;
		}
	}
	// 释放该一级科室节点内存前，先释放其下属的二级科室链表
	SubDepartment* currSub = targetDept->subDeptHead;
	while (currSub != NULL) {
		SubDepartment* tempSub = currSub;
		currSub = currSub->next;
		// 释放前清空医生端绑定
		clearDoctorsSubDeptBinding(*sys, tempSub->subDeptId);
		free(tempSub);
	}
	// 最后释放一级科室节点内存
	free(targetDept);
}

// 辅助函数：安全删除一个二级科室节点 (仅删除该节点，不影响父节点和兄弟节点)
static void executeDeleteSubDepartment(HIS_System* sys, Department* parentDept, SubDepartment* targetSub) {
	if (parentDept == NULL || targetSub == NULL) return;

	//断开一、二级科室
	if (parentDept->subDeptHead == targetSub) {
		parentDept->subDeptHead = targetSub->next;
	}
	else {
		SubDepartment* prevSub = parentDept->subDeptHead;
		while (prevSub != NULL && prevSub->next != targetSub) {
			prevSub = prevSub->next;
		}
		if (prevSub != NULL) {
			prevSub->next = targetSub->next;
		}
	}
	// 释放前清空医生端绑定
	clearDoctorsSubDeptBinding(sys, targetSub->subDeptId);
	//释放二级科室节点内存
	free(targetSub);
}

//删除科室模块
void deleteDepartment(HIS_System** sys) {
	if ((*sys)->deptHead == NULL) {
		printf("\n>>> 系统内没有科室数据！\n");
		return;
	}
	int choice;
	printf("\n--- 删除科室信息 ---\n");
	printf("1. 按一级科室名称删除\n");
	printf("2. 按一级科室代码删除\n");
	printf("3. 按二级科室名称删除\n");
	printf("4. 按科室编号删除\n");
	printf("0. 返回上一级菜单\n");
	choice = safeGetInt("请选择删除方式: ");

	char queryStr[STR_LEN];
	Department* curr = (*sys)->deptHead;

	// 用于记录匹配项的指针数组，方便用户选择删除对应目标
	Department* deptMatches[100];
	SubDepartment* subDeptMatches[100];
	int matchCount = 0;

	if (choice == 1 || choice == 2) {
		bool byName = (choice == 1) ? true : false; // 通过这个布尔变量来区分是按名称删除还是按代码删除，减少重复代码
		if (byName) {
			safeGetString("请输入要删除的一级科室名称: ", queryStr, STR_LEN);
		}
		else {
			safeGetString("请输入要删除的一级科室代码: ", queryStr, ID_LEN);
		}
		const char* searchField[] = { "名称", "代码" };
		int fieldIndex = byName ? 0 : 1;

		while (curr != NULL) {
			bool matched = byName ? (strcmp(curr->categoryName, queryStr) == 0) : (strcmp(curr->categoryId, queryStr) == 0);
			if (matched && matchCount < 100) {
				deptMatches[matchCount++] = curr;
			}
			curr = curr->next;
		}

		if (matchCount <= 0) {
				printf(">>> 未找到%s为 <%s> 的一级科室！\n", searchField[fieldIndex], queryStr);
			return;
		}

		int deleteMode = -1;
		while (1) {
			if (matchCount > 1) {
					printf("\n>>> 发现 %d 个%s相同的一级科室记录！\n", matchCount, searchField[fieldIndex]);
					printf("1. 批量删除：删除所有%s为 <%s> 的一级科室\n", searchField[fieldIndex], queryStr);
				printf("2. 精确删除：选择特定的一个科室进行删除\n");
				deleteMode = safeGetInt("请选择操作方式: ");
			}

			if (confirmFunc("删除", "一级科室及其下属二级科室")) {
				if (deleteMode == 1) {
					for (int i = 0; i < matchCount; i++) {
						executeDeleteDepartment(sys, deptMatches[i]);
					}
					printf(">>> 成功删除 %d 个一级科室及其下属二级科室！\n", matchCount);
					break;
				}
				else if (deleteMode == 2) {
					printf("\n>>> 请选择要删除的具体节点：\n");
					printDepartmentHeader();
					for (int i = 0; i < matchCount; i++) {
						printf("#[%d]:-------------------------------------------------------------\n", i + 1);
						printDepartmentInfo(deptMatches[i], deptMatches[i]->subDeptHead);
					}

					while (1) {
						int sel = safeGetInt("请输入对应的序号: ");
						if (sel >= 1 && sel <= matchCount) {
							executeDeleteDepartment(sys, deptMatches[sel - 1]);
							printf(">>> 成功删除该节点的一级科室及其下属二级科室！\n");
							break;
						}
						else {
							printf(">>> 序号无效，请重新输入。\n");
						}
					}
					break;
				}
				else {
					printf(">>> 操作方式选择错误，请重试。\n");
					continue;
				}
			}
			else {
				printf(">>> 已取消删除。\n\n");
				break;
			}
		}
	}
	else if (choice == 3) {
		// 按二级科室名称删除 
		safeGetString("请输入要删除的二级科室名称: ", queryStr, STR_LEN);
		while (curr != NULL) {
			SubDepartment* subCurr = curr->subDeptHead;
			while (subCurr != NULL) {
				if (strcmp(subCurr->subDeptName, queryStr) == 0 && matchCount < 100) {
					deptMatches[matchCount] = curr;
					subDeptMatches[matchCount] = subCurr;
					matchCount++;
				}
				subCurr = subCurr->next;
			}
			curr = curr->next;
		}

		if (matchCount == 0) {
			printf(">>> 未找到名为 <%s> 的二级科室！\n", queryStr);
			return;
		}

		int deleteMode = 1;
		if (matchCount > 1) {
			printf("\n>>> 发现 %d 个 <%s> 的二级科室记录！\n", matchCount, queryStr);
			printf("1. 批量删除：删除所有名为 <%s> 的二级科室\n", queryStr);
			printf("2. 精确删除：选择特定的一个进行删除\n");
			deleteMode = safeGetInt("请选择操作方式: ");
		}

		if (confirmFunc("删除", "二级科室")) {
			if (deleteMode == 1) {
				// 批量删除
				for (int i = 0; i < matchCount; i++) {
					executeDeleteSubDepartment(*sys, deptMatches[i], subDeptMatches[i]);
				}
				printf(">>> 成功删除了 %d 个二级科室！\n", matchCount);
			}
			else if (deleteMode == 2) {
				// 精确删除
				printf("\n>>> 请选择要删除的具体节点：\n");
				printDepartmentHeader();
				for (int i = 0; i < matchCount; i++) {
					printf("#[%d]:\n", i + 1);
					printDepartmentInfo(deptMatches[i], subDeptMatches[i]);
				}

				while (1) {
					int sel = safeGetInt("请输入对应的序号: ");
					if (sel >= 1 && sel <= matchCount) {
						executeDeleteSubDepartment(*sys, deptMatches[sel - 1], subDeptMatches[sel - 1]);
						printf(">>> 成功删除指定的二级科室！\n");
						break;
					}
					else {
						printf(">>> 序号无效，请重新输入。\n");
					}
				}
			}
		}
		else {
			printf(">>> 已取消删除。\n");
		}
	}
	else if (choice == 4) {
		// 由于编号全局唯一，无需区分批量或者精确删除，直接找到对应编号的科室进行删除即可
		safeGetString("请输入要删除的科室编号: ", queryStr, ID_LEN);
		SubDepartment* targetSubDept = NULL;
		Department* targetParentDept = NULL;

		while (curr != NULL) {
			SubDepartment* subCurr = curr->subDeptHead;
			while (subCurr != NULL) {
				if (strcmp(subCurr->subDeptId, queryStr) == 0) {
					targetSubDept = subCurr;
					targetParentDept = curr;
					break;
				}
				subCurr = subCurr->next;
			}
			if (targetSubDept != NULL) break;
			curr = curr->next;
		}

		if (!targetSubDept) {
			printf(">>> 未找到编号为 %s 的科室！\n", queryStr);
			return;
		}

		printf("\n>>> 查找到目标科室：\n");
		printDepartmentHeader();
		printDepartmentInfo(targetParentDept, targetSubDept);

		if (confirmFunc("删除", "该科室")) {
			// 直接调用辅助函数断开并释放内存
			executeDeleteSubDepartment(*sys, targetParentDept, targetSubDept);
			printf(">>> 科室编号 %s 已成功删除！\n", queryStr);
		}
		else {
			printf(">>> 已取消删除。\n");
		}
	}
	else if (choice == 0) {
		return;
	}
	else {
		printf(">>> 无效的选择！\n");
		return;
	}
}
// 显示系统内所有科室信息
void displayAllDepartments(HIS_System* sys) {
	if (sys->deptHead == NULL) {
		printf("\n>>> 系统内没有科室数据！\n");
		return;
	}
	printDepartmentHeader();
	Department* curr = sys->deptHead;
	int totalcount = 0;
	while (curr != NULL) {
		SubDepartment* subCurr = curr->subDeptHead;
		if (subCurr == NULL) {
			printf("#%d:-------------------------------------------------------------\n", ++totalcount);
			printDepartmentInfo(curr, NULL);
		}
		while (subCurr != NULL) {
			printf("#%d:-------------------------------------------------------------\n", ++totalcount);
			printDepartmentInfo(curr, subCurr);
			int doctorCount = countDoctorsBySubDeptId(sys, subCurr->subDeptId);
			printf("   关联医生数量: %d\n", doctorCount);
			printDoctorsBySubDeptId(sys, subCurr->subDeptId, doctorCount);
			subCurr = subCurr->next;
		}
		curr = curr->next;
	}
	printf("----------------------------------------------------------------\n");
	printf(">>> 共显示 %d 条科室记录。\n", totalcount);
	pressEnterToContinue();
}


void departmentManageMenu(HIS_System* sys) {
	if(sys == NULL) {
		printf(">>> 严重错误: 系统底座未初始化！！！\n");
		return;
	}

	loadDepartmentSystemData(sys);

	int choice;
	while (1) {
		printf("\n========== 科室管理系统 ==========\n");
		printf("1. 添加科室 (支持一级下挂载多个二级)\n");
		printf("2. 查询科室\n");
		printf("3. 修改科室 (支持同名批量/精确修改)\n");
		printf("4. 排序科室\n");
		printf("5. 删除科室\n");
		printf("6. 显示所有科室\n");
		printf("7. 保存科室数据\n");
		printf("0. 返回上一级菜单\n");
		printf("==================================\n");
		choice = safeGetInt("请输入您的选择: ");

		switch (choice) {
		case 1: addDepartment(sys); break;
		case 2: queryDepartment(sys); break;
		case 3: modifyDepartment(sys); break;	
		case 4: departmentSortMenu(sys); break;// 排序科室
		case 5: deleteDepartment(&sys); break;  // 二维链表删除
		case 6:	displayAllDepartments(sys); break; // 显示所有科室信息
		case 7:
			saveDepartmentSystemData(sys);
			printf(">>> 科室数据保存请求已发送！\n");
			break;
		case 0: return;
		default: printf(">>> 无效选择，请重新输入！\n");
		}
	}
}