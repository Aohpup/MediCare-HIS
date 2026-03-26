#define _CRT_SECURE_NO_WARNINGS
#include"HIS_System.h"
#include"DepartmentFileManage.h"

void loadDepartmentSystemData(HIS_System* sys) {
	printf(">>> 正在从科室文件中加载数据...\n");
	FILE* fp = fopen(DEPARTMENT_FILE, "r");
	if (!fp) {
		printf(">>> 警告: 找不到 %s，将作为新系统启动。\n", DEPARTMENT_FILE);
		return;
	}
	char buffer[256];
	while (fgets(buffer, sizeof(buffer), fp) != NULL){
		Department* newDepartment = (Department*)malloc(sizeof(Department));
		if(newDepartment == NULL) {
			printf(">>> 错误: 内存分配失败，无法加载科室数据！\n");
			fclose(fp);
			return;
		}
		// txt文件中格式为: 一级大科 二级科室 诊室编号 
		if (sscanf(buffer, "%s %s %s", newDepartment->categoryName, newDepartment->subDeptHead->subDeptName, newDepartment->subDeptHead->subDeptId) == 3) {
			newDepartment->next = sys->deptHead;
			sys->deptHead = newDepartment;
		}
		else {
			free(newDepartment);
		}
	}

	fclose(fp);
	printf(">>> 数据加载完成！\n");
}	

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