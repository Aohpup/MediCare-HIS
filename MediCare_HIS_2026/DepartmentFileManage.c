#define _CRT_SECURE_NO_WARNINGS
#include"HIS_System.h"
#include"DepartmentFileManage.h"
#include"string.h"
bool is_Department_File_Loaded = false;	//标记是否加载过科室数据

void loadDepartmentSystemData(HIS_System* sys) {
	if(TEST_SYSTEM_DEBUG)
	printf(">>> 正在从科室文件中加载数据...\n");
	FILE* fp = fopen(DEPARTMENT_FILE, "r");
	if (!fp) {
		if(!TEST_SYSTEM_DEBUG) {
			printf("严重错误: 科室数据不存在！请确保文件存在或联系管理员。\n");
			exit(EXIT_FAILURE); // 直接退出程序，避免后续操作导致更严重的错误
		}
		printf(">>> 警告: 找不到 %s，将作为新系统启动。\n", DEPARTMENT_FILE);
		return;
	}

	char dummyLine[512]; // 用于读取和丢弃文件开头的注释行
	fgets(dummyLine, sizeof(dummyLine), fp); // 读取并丢弃第一行注释

	char buffer[1024];
	char tempCategory[STR_LEN];		// 一级科室名称
	char tempCategoryId[ID_LEN];	// 一级科室代码
	char tempSubName[STR_LEN]; 		// 二级科室名称
	char tempSubId[ID_LEN];			// 二级科室编号
	while (fgets(buffer, sizeof(buffer), fp) != NULL) {
		Department* targetDept = NULL;
		Department* curr = sys->deptHead;

		int parsed = sscanf(buffer, "%s %s %s %s", tempCategory, tempCategoryId, tempSubName, tempSubId);
		if (parsed == 3) {
			strcpy(tempSubId, tempSubName);
			strcpy(tempSubName, tempCategoryId);
			tempCategoryId[0] = '\0';
		}
		else if (parsed != 4) {
			printf(">>> 警告: 文件格式错误，请联系管理员。\n已跳过无效行: %s\n", buffer);
			continue;
		}

		while (curr != NULL) {
			if (strcmp(curr->categoryName, tempCategory) == 0 && strcmp(curr->categoryId, tempCategoryId) == 0) {
				targetDept = curr;
				break;
			}
			curr = curr->next;
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
			newSubDept->next = targetDept->subDeptHead;
			targetDept->subDeptHead = newSubDept;
		}
		else {
			Department* newDepartment = (Department*)malloc(sizeof(Department));
			if (!newDepartment) {
				printf(">>> 错误: 内存分配失败，无法加载科室数据！\n");
				free(newSubDept);
				continue;
			}
			strcpy(newDepartment->categoryName, tempCategory);
			strcpy(newDepartment->categoryId, tempCategoryId);

			newSubDept->next = NULL;
			newDepartment->subDeptHead = newSubDept;
			newDepartment->next = sys->deptHead;
			sys->deptHead = newDepartment;
		}
	}
	fclose(fp);
	printf(">>> 数据加载完成！\n");
	is_Department_File_Loaded = true;	//标记已加载过科室数据

}

void saveDepartmentSystemData(HIS_System* sys) {
	FILE* fp = fopen(DEPARTMENT_FILE, "w");
	if (!fp) {
		if (!TEST_SYSTEM_DEBUG) {
			printf("严重错误: 无法保存文件！请确保文件权限或联系管理员。\n");
			exit(EXIT_FAILURE); // 直接退出程序，避免数据丢失或后续操作导致更严重的错误
		}
		printf(">>> 错误: 无法创建或打开保存文件！\n");
		return;
	}

	fprintf(fp, "# HIS DEPARTMENT DATA FILE\n");

	Department* curr = sys->deptHead;
	while (curr != NULL) {
		SubDepartment* subCurr = curr->subDeptHead;
		while (subCurr != NULL) {
			fprintf(fp, "%s %s %s %s\n", curr->categoryName, curr->categoryId, subCurr->subDeptName, subCurr->subDeptId);
			subCurr = subCurr->next;
		}
		curr = curr->next;
	}

	fclose(fp);
	printf(">>> 科室数据保存成功！\n");

}