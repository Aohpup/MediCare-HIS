#define _CRT_SECURE_NO_WARNINGS
#include"HIS_System.h"
#include"DepartmentFileManage.h"
#include"string.h"

//TODO:修改下面的错误认定，应改为：一个一级科室对应下方多个二级科室的情况，txt文件中每行记录一个二级科室信息，包含所属一级科室名称、二级科室名称和诊室编号；加载时如果遇到新的一级科室则创建新的一级科室节点，如果遇到已存在的一级科室则将二级科室添加到该一级科室的二级链表中。
void loadDepartmentSystemData(HIS_System* sys) {
	printf(">>> 正在从科室文件中加载数据...\n");
	FILE* fp = fopen(DEPARTMENT_FILE, "r");
	if (!fp) {
		printf(">>> 警告: 找不到 %s，将作为新系统启动。\n", DEPARTMENT_FILE);
		return;
	}
	char buffer[1024];
	char tempCategory[STR_LEN];// 用于暂存当前行的一级科室名称，判断是否需要创建新节点
	char tempSubName[STR_LEN]; // 用于暂存当前行的二级科室名称
	char tempSubId[ID_LEN];	   // 用于暂存当前行的科室编号
	while (fgets(buffer, sizeof(buffer), fp) != NULL) {
		Department* targetDept = NULL;
		Department* curr = sys->deptHead;
		if (sscanf(buffer, "%s %s %s", tempCategory, tempSubName, tempSubId) == 3) {
			while (curr != NULL) {
				if (strcmp(curr->categoryName, tempCategory) == 0) {
					targetDept = curr;
					break;
				}
				curr = curr->next;
			}
		}
		else {
			printf(">>> 警告: 文件格式错误，请联系管理员。\n已跳过无效行: %s\n", buffer);
			continue;
		}

		SubDepartment* newSubDept = (SubDepartment*)malloc(sizeof(SubDepartment));
		if(newSubDept == NULL) {
			printf(">>> 错误: 内存分配失败，无法加载科室数据！\n");
			fclose(fp);
			return;
		}
		strcpy(newSubDept->subDeptName, tempSubName);
		strcpy(newSubDept->subDeptId, tempSubId);

		if(targetDept != NULL) {
			// 一级科室存在：头插法接入二级链表
			newSubDept->next = targetDept->subDeptHead;
			targetDept->subDeptHead = newSubDept;
		}
		else {
			// 一级科室不存在：创建全新的一级科室及其第一个二级科室
			Department* newDepartment = (Department*)malloc(sizeof(Department));
			if (!newDepartment) { 
				printf(">>> 错误: 内存分配失败，无法加载科室数据！\n"); 
				free(newSubDept); 
				continue;
			}
			strcpy(newDepartment->categoryName, tempCategory);
			
			newSubDept->next = NULL;
			newDepartment->subDeptHead = newSubDept;

			// 头插法接入一级链表
			newDepartment->next = sys->deptHead;
			sys->deptHead = newDepartment;

		}
	}
	fclose(fp);
	printf(">>> 数据加载完成！\n");
}	

//没有实现一级科室对应多个二级科室的情况，txt文件中每行记录一个一级科室信息和一个二级科室信息，加载时如果遇到新的一级科室则创建新的一级科室节点，如果遇到已存在的一级科室则将二级科室添加到该一级科室的二级链表中。
/*
		// 为二级科室分配内存
		newDepartment->subDeptHead = (SubDepartment*)malloc(sizeof(SubDepartment));
		if (newDepartment->subDeptHead == NULL) {
			printf(">>> 错误: 内存分配失败，无法加载科室数据！\n");
			free(newDepartment);
			fclose(fp);
			return;
		}

		// txt文件中格式为: 一级大科 二级科室 诊室编号 
		if (sscanf(buffer, "%s %s %s", newDepartment->categoryName, newDepartment->subDeptHead->subDeptName, newDepartment->subDeptHead->subDeptId) == 3) {
			//头插法接入一级链表
			newDepartment->next = sys->deptHead;
			sys->deptHead = newDepartment;
		}
		else {
			free(newDepartment);
		}
	}

	fclose(fp);
	printf(">>> 数据加载完成！\n");
}	*/


void saveDepartmentSystemData(HIS_System* sys) {
	FILE* fp = fopen(DEPARTMENT_FILE, "w");
	if (!fp) {
		printf(">>> 错误: 无法创建或打开保存文件！\n");
		return;
	}

	Department* curr = sys->deptHead;
	while (curr != NULL) {	// 遍历每个大类科室
		SubDepartment* subCurr = curr->subDeptHead;	
		while (subCurr != NULL) {	// 遍历每个大类科室下的小类科室
			fprintf(fp, "%s %s %s\n", curr->categoryName, subCurr->subDeptName, subCurr->subDeptId);
			subCurr = subCurr->next;
		}
		curr = curr->next;
	}

	fclose(fp);
	printf(">>> 系统数据保存成功！\n");

}