#define _CRT_SECURE_NO_WARNINGS

#include "HIS_System.h"
#include "DepartmentManage.h"
#include "DepartmentFileManage.h"
#include "InputUtils.h"
#include "ConfirmFunc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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

// 打印单条二级科室详细信息(包含其所属的一级科室)
void printDepartmentInfo(Department* dept, SubDepartment* subDept) {
    printf("一级科室: [%s] | 二级科室: [%s] | 编号: %s\n",
        dept->categoryName,
        subDept ? subDept->subDeptName : "无下属科室",
        subDept ? subDept->subDeptId : "无下属诊室");
}

void addDepartment(HIS_System* sys) {
    while (1) {
        printf("\n--- 录入新科室 (输入 '-1' 取消本次添加) ---\n");
        char tempCategoryName[STR_LEN];
        safeGetString("请输入一级科室名称: ", tempCategoryName, STR_LEN);

        if (strcmp(tempCategoryName, "-1") == 0) {
            printf(">>> 已退出科室添加。\n");
            return;
        }

        Department* targetDept = NULL;
        // 查找是否已经存在该一级科室
        Department* curr = sys->deptHead;
        while (curr != NULL) {
            if (strcmp(curr->categoryName, tempCategoryName) == 0) {
                targetDept = curr;
                break;
            }
            curr = curr->next;
        }

        if (targetDept != NULL) {
            printf(">>> 提示：一级科室 <%s> 已存在！\n", targetDept->categoryName);
            char addSubChoice[STR_LEN];
            safeGetString(">>> 是否要在该科室下直接添加二级科室？(输入 'Y' 添加，其他键重新输入): ", addSubChoice, STR_LEN);

            if (addSubChoice[0] == 'Y' || addSubChoice[0] == 'y') {
                // 在已有的一级科室下添加二级科室
                SubDepartment* newSubDept = (SubDepartment*)malloc(sizeof(SubDepartment));
                if (!newSubDept) { printf(">>> 内存分配失败！\n"); return; }

                safeGetString("请输入新的二级科室名称: ", newSubDept->subDeptName, STR_LEN);
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
            strcpy(newDepartment->categoryName, tempCategoryName);
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
            printf(">>> 全新一级科室 <%s> 及其二级科室 <%s> (编号: %s) 录入成功！\n",
                newDepartment->categoryName, newSubDept->subDeptName, newSubDept->subDeptId);
        }
    }
}

void queryDepartment(HIS_System* sys) {
    if (sys->deptHead == NULL) {
        printf("\n>>> 系统内没有科室数据！\n");
        return;
    }

    int choice;
    printf("\n--- 科室信息查询 ---\n");
    printf("1. 按一级科室名称查询\n");
    printf("2. 按二级科室名称查询\n");
    printf("3. 按科室编号查询\n");
    printf("0. 返回上一级菜单\n");
    choice = safeGetInt("请选择查询方式: ");

    char queryStr[STR_LEN];
    Department* curr = sys->deptHead;
    bool found = false;
    int matchCount = 0;

    switch (choice) {
    case 1:
        safeGetString("请输入一级科室名称: ", queryStr, STR_LEN);
        while (curr != NULL) {
            if (strcmp(curr->categoryName, queryStr) == 0) {
                SubDepartment* subCurr = curr->subDeptHead;
                if (subCurr == NULL) {
                    printf("#%d: ", ++matchCount);
                    printDepartmentInfo(curr, NULL);
                    found = true;
                }
                while (subCurr != NULL) {
                    printf("#%d: ", ++matchCount);
                    printDepartmentInfo(curr, subCurr);
                    found = true;
                    subCurr = subCurr->next;
                }
            }
            curr = curr->next;
        }
        break;
    case 2:
        safeGetString("请输入二级科室名称: ", queryStr, STR_LEN);
        while (curr != NULL) {
            SubDepartment* subCurr = curr->subDeptHead;
            while (subCurr != NULL) {
                if (strcmp(subCurr->subDeptName, queryStr) == 0) {
                    printf("#%d: ", ++matchCount);
                    printDepartmentInfo(curr, subCurr);
                    found = true;
                }
                subCurr = subCurr->next;
            }
            curr = curr->next;
        }
        break;
    case 3:
        safeGetString("请输入科室编号: ", queryStr, ID_LEN);
        while (curr != NULL) {
            SubDepartment* subCurr = curr->subDeptHead;
            while (subCurr != NULL) {
                if (strcmp(subCurr->subDeptId, queryStr) == 0) {
                    printf(">>> 查找到唯一结果:\n");
                    printDepartmentInfo(curr, subCurr);
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
    printf("2. 按二级科室名称修改\n");
    printf("3. 按科室编号修改\n");
    printf("0. 返回上一级菜单\n");
    choice = safeGetInt("请选择修改目标: ");

    char queryStr[STR_LEN];
    Department* curr = sys->deptHead;

    // 用于记录匹配项的指针数组，方便用户选择删除对应目标
    Department* deptMatches[100];
    SubDepartment* subDeptMatches[100];
    int matchCount = 0;

    if (choice == 1) {
        safeGetString("请输入要修改的一级科室名称: ", queryStr, STR_LEN);
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

                if (modifyMode == 1) {
                    // 修改所有匹配项
                    for (int i = 0; i < matchCount; i++) {
                        strcpy(deptMatches[i]->categoryName, newName);
                    }
                    printf(">>> 成功将 %d 个一级科室重命名为 <%s>！\n", matchCount, newName);
                    break;
                }
                else if (modifyMode == 2) {
                    // 展示列表供选择
                    printf("\n>>> 请选择要修改的具体节点：\n");
                    for (int i = 0; i < matchCount; i++) {
                        printf("#[%d]:", i + 1);
                        printDepartmentInfo(deptMatches[i], deptMatches[i]->subDeptHead);
                    }

                    while (1) { // 单独处理下用户输入的序号有效性 
                        int sel = safeGetInt("请输入对应的序号: ");  
                        if (sel >= 1 && sel <= matchCount) {
                            strcpy(deptMatches[sel - 1]->categoryName, newName);
                            printf(">>> 成功修改该节点的名称为 <%s>！\n", newName);
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

    else if (choice == 2) {
        // ========== 修改二级科室名称 ==========
        safeGetString("请输入要修改的二级科室名称: ", queryStr, STR_LEN);
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

        if (modifyMode == 1) {
            for (int i = 0; i < matchCount; i++) {
                strcpy(subDeptMatches[i]->subDeptName, newName);
            }
            printf(">>> 成功将 %d 个二级科室重命名为 <%s>！\n", matchCount, newName);
        }
        else if (modifyMode == 2) {
            printf("\n>>> 请选择要修改的具体节点：\n");
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
    else if (choice == 3) {
		//按科室编号修改,由于编号全局唯一，无需区分批量/精确修改
        safeGetString("请输入要修改的科室编号: ", queryStr, ID_LEN);
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
        printDepartmentInfo(targetParentDept, targetSubDept);

        printf("\n1. 修改该科室名称\n");
		printf("2. 修改该科室编号\n");
        int subChoice = safeGetInt("请选择要修改的内容: ");

        if (subChoice == 1) {
            char newName[STR_LEN];
            safeGetString("请输入新的二级科室名称: ", newName, STR_LEN);
            strcpy(targetSubDept->subDeptName, newName);
            printf(">>> 修改成功！\n");
        }
        else if (subChoice == 2) {
            char newId[ID_LEN];
            safeGetString("请输入新的科室编号: ", newId, ID_LEN);
            if (isDepartmentIdExist(sys->deptHead, newId)) {
                printf(">>> 错误：新编号已存在，修改失败！\n");
            }
            else {
                strcpy(targetSubDept->subDeptId, newId);
                printf(">>> 编号修改成功！\n");
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
	printf("严重错误：未正确处理修改流程，已退出修改模块！\n");
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
        free(tempSub);
    }
	// 最后释放一级科室节点内存
    free(targetDept);
}

// 辅助函数：安全删除一个二级科室节点 (仅删除该节点，不影响父节点和兄弟节点)
static void executeDeleteSubDepartment(Department* parentDept, SubDepartment* targetSub) {
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

    //释放二级科室节点内存
    free(targetSub);
}

//删除科室模块(处理重名逻辑)
void deleteDepartment(HIS_System** sys) {
    if ((*sys)->deptHead == NULL) {
        printf("\n>>> 系统内没有科室数据！\n");
        return;
    }
    int choice;
    printf("\n--- 删除科室信息 ---\n");
    printf("1. 按一级科室名称删除\n");
    printf("2. 按二级科室名称删除\n");
    printf("3. 按科室编号删除\n");
    printf("0. 返回上一级菜单\n");
    choice = safeGetInt("请选择删除方式: ");

    char queryStr[STR_LEN];
    Department* curr = (*sys)->deptHead;

    // 用于记录匹配项的指针数组，方便用户选择删除对应目标
    Department* deptMatches[100];
    SubDepartment* subDeptMatches[100];
    int matchCount = 0;

    if (choice == 1) {
        // ========== 按一级科室名称删除 ==========
        safeGetString("请输入要删除的一级科室名称: ", queryStr, STR_LEN);
        while (curr != NULL) {
            if (strcmp(curr->categoryName, queryStr) == 0 && matchCount < 100) {
                deptMatches[matchCount++] = curr;
            }
            curr = curr->next;
        }

        if (matchCount <= 0) {
            printf(">>> 未找到名为 <%s> 的一级科室！\n", queryStr);
            return;
        }

        int deleteMode = 1;
        while (1) {
            if (matchCount > 1) {
                printf("\n>>> 发现 %d 个同名的一级科室记录！\n", matchCount);
                printf("1. 批量删除：删除所有名为 <%s> 的一级科室\n", queryStr);
                printf("2. 精确删除：选择特定的一个科室进行删除\n");
                deleteMode = safeGetInt("请选择操作方式: ");
            }

            if (confirmFunc("删除", "一级科室及其下属二级科室")) {
                if (deleteMode == 1) {
                    // 批量删除：调用辅助函数依次释放
                    for (int i = 0; i < matchCount; i++) {
                        executeDeleteDepartment(sys, deptMatches[i]);
                    }
                    printf(">>> 成功删除 %d 个一级科室及其下属二级科室！\n", matchCount);
                    break;
                }
                else if (deleteMode == 2) {
                    // 精确删除
                    printf("\n>>> 请选择要删除的具体节点：\n");
                    for (int i = 0; i < matchCount; i++) {
                        printf("#[%d]: ", i + 1);
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
                    break; // 完成精确删除后跳出最外层while
                }
                else {
                    printf(">>> 操作方式选择错误，请重试。\n");
                }
            }
            else {
                printf(">>> 已取消删除。\n\n");
                break;
            }
        }
    }
    else if (choice == 2) {
        // ========== 按二级科室名称删除 ==========
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
                    executeDeleteSubDepartment(deptMatches[i], subDeptMatches[i]);
                }
                printf(">>> 成功删除了 %d 个二级科室！\n", matchCount);
            }
            else if (deleteMode == 2) {
                // 精确删除
                printf("\n>>> 请选择要删除的具体节点：\n");
                for (int i = 0; i < matchCount; i++) {
                    printf("#[%d]: ", i + 1);
                    printDepartmentInfo(deptMatches[i], subDeptMatches[i]);
                }

                while (1) {
                    int sel = safeGetInt("请输入对应的序号: ");
                    if (sel >= 1 && sel <= matchCount) {
                        executeDeleteSubDepartment(deptMatches[sel - 1], subDeptMatches[sel - 1]);
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
    else if (choice == 3) {
        // 由于编号全局唯一，无需区分批量/精确删除
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
        printDepartmentInfo(targetParentDept, targetSubDept);

        if (confirmFunc("删除", "该科室")) {
            // 直接调用辅助函数断开并释放内存
            executeDeleteSubDepartment(targetParentDept, targetSubDept);
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
        printf("0. 返回主菜单\n");
        printf("==================================\n");
        choice = safeGetInt("请输入您的选择: ");

        switch (choice) {
        case 1: addDepartment(sys); break;
        case 2: queryDepartment(sys); break;
        case 3: modifyDepartment(sys); break;
        case 4: 
			printf(">>> 排序功能正在开发中，敬请期待！\n");
            //departmentSortMenu(sys);// 排序科室
            break;
        case 5: deleteDepartment(&sys); break;  // 二维链表删除
        case 6:
            printf(">>> 显示所有科室功能正在开发中，敬请期待！\n");
            // displayAllDepartments(sys);// 显示所有科室
            break;
        case 7:
            saveDepartmentSystemData(sys);
            printf(">>> 科室数据保存请求已发送！\n");
            break;
        case 0: return;
        default: printf(">>> 无效选择，请重新输入！\n");
        }
    }
}