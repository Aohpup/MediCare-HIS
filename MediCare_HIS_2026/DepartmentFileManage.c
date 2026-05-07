#define _CRT_SECURE_NO_WARNINGS
#include"HIS_System.h"
#include"DepartmentFileManage.h"
#include"string.h"
bool is_Department_File_Loaded = false;	//标记是否加载过科室数据

void loadDepartmentSystemData(HIS_System* sys) {
	// 懒加载保护：已加载过则跳过，避免重复解析导致数据膨胀
	if (is_Department_File_Loaded) {
		if (TEST_SYSTEM_DEBUG)
			printf(">>> 科室数据已加载过，跳过重复加载。\n");
		return;
	}
	if (TEST_SYSTEM_DEBUG)
		printf(">>> 正在从科室文件中加载数据...\n");
	FILE* fp = fopen(DEPARTMENT_FILE, "r");
	if (!fp) {
		if (!TEST_SYSTEM_DEBUG) {
			printf("严重错误: 科室数据不存在！请确保文件存在或联系管理员。\n");
			exit(EXIT_FAILURE);
		}
		printf(">>> 警告: 找不到 %s，将作为新系统启动。\n", DEPARTMENT_FILE);
		return;
	}

	char dummyLine[512];
	fgets(dummyLine, sizeof(dummyLine), fp); // 跳过注释行

	char buffer[1024];
	char tempCategory[STR_LEN];
	char tempCategoryId[ID_LEN];
	char tempSubName[STR_LEN];
	char tempSubId[ID_LEN];
	int loadCount = 0;	// 已加载的有效行数
#define MAX_DEPT_LOAD 2000  // 科室数据行上限，超量则告警并停止加载
	while (fgets(buffer, sizeof(buffer), fp) != NULL) {
		int parsed = sscanf(buffer, "%s %s %s %s", tempCategory, tempCategoryId, tempSubName, tempSubId);
		if (parsed == 3) {
			strcpy(tempSubId, tempSubName);
			strcpy(tempSubName, tempCategoryId);
			tempCategoryId[0] = '\0';
		} else if (parsed != 4) {
			if (TEST_SYSTEM_DEBUG)
				printf(">>> 警告: 文件格式错误，已跳过无效行: %s\n", buffer);
			continue;
		}

		// 查找匹配的一级科室
		Department* targetDept = NULL;
		Department* curr = sys->deptHead;
		while (curr != NULL) {
			if (strcmp(curr->categoryName, tempCategory) == 0 &&
				strcmp(curr->categoryId, tempCategoryId) == 0) {
				targetDept = curr;
				break;
			}
			curr = curr->next;
		}

		// 去重：若目标科室下已存在相同 subDeptId，跳过
		if (targetDept != NULL) {
			SubDepartment* subCurr = targetDept->subDeptHead;
			bool dup = false;
			while (subCurr != NULL) {
				if (strcmp(subCurr->subDeptId, tempSubId) == 0 &&
					strcmp(subCurr->subDeptName, tempSubName) == 0) {
					dup = true;
					break;
				}
				subCurr = subCurr->next;
			}
			if (dup) {
				if (TEST_SYSTEM_DEBUG)
					printf(">>> 跳过重复科室: %s/%s -> %s/%s\n",
						tempCategory, tempCategoryId, tempSubName, tempSubId);
				continue;
			}
		}

		loadCount++;
		if (loadCount > MAX_DEPT_LOAD) {
			printf(">>> 警告: 科室数据行数超过上限(%d)，已停止解析剩余行。\n"
				"    请检查 %s 文件是否被异常写入大量数据。\n", MAX_DEPT_LOAD, DEPARTMENT_FILE);
			break;
		}

		SubDepartment* newSubDept = (SubDepartment*)malloc(sizeof(SubDepartment));
		if (newSubDept == NULL) {
			printf(">>> 错误: 内存分配失败，无法加载科室数据！\n");
			fclose(fp);
			return;
		}
		strcpy(newSubDept->subDeptName, tempSubName);
		strcpy(newSubDept->subDeptId, tempSubId);

		if (targetDept != NULL) {
			newSubDept->next = targetDept->subDeptHead;
			targetDept->subDeptHead = newSubDept;
		} else {
			Department* newDepartment = (Department*)malloc(sizeof(Department));
			if (!newDepartment) {
				printf(">>> 错误: 内存分配失败，无法加载科室数据！\n");
				free(newSubDept);
				fclose(fp);
				return;
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
	if (TEST_SYSTEM_DEBUG)
		printf(">>> 科室数据加载完成（有效条目: %d）！\n", loadCount);
	is_Department_File_Loaded = true;
#undef MAX_DEPT_LOAD
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
	if(TEST_SYSTEM_DEBUG)
	printf(">>> 科室数据保存成功！\n");

}