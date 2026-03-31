#define _CRT_SECURE_NO_WARNINGS
#include"DocterFileManage.h"
#include"DocterManage.h"
#include<string.h>

// 从txt文件加载系统数据
void loadDoctorSystemData(HIS_System* sys) {
	printf(">>> 正在从医生文件中加载数据...\n");
	FILE* fp = fopen(DOCTOR_FILE, "r");
	if (!fp) {
		printf(">>> 警告: 找不到 %s，将作为新系统启动。\n", DOCTOR_FILE);
		return;
	}
	char buffer[512];
	while (fgets(buffer, sizeof(buffer), fp) != NULL) {
		Docter* newDoctor = (Docter*)malloc(sizeof(Docter));
		if (newDoctor == NULL) {
			printf(">>> 错误: 内存分配失败，无法加载医生数据！\n");
			fclose(fp);
			return;
		}

		char id[ID_LEN];
		char name[STR_LEN];
		char deptName[STR_LEN];
		char roomId[ID_LEN];
		int count = 0;

		int parsed = sscanf(buffer, "%s %s %s %s %d", id, name, deptName, roomId, &count);
		if (parsed == 5) {
			strcpy(newDoctor->docterId, id);
			strcpy(newDoctor->docterName, name);
			strcpy(newDoctor->department, deptName);
			if (strcmp(roomId, "NULL") == 0 || strcmp(roomId, "-") == 0) roomId[0] = '\0';
			strcpy(newDoctor->subDeptId, roomId);
			newDoctor->consultationCount = count;
		}
		else {
			parsed = sscanf(buffer, "%s %s %s %d", id, name, deptName, &count);
			if (parsed != 4) {
				free(newDoctor);
				continue;
			}
			strcpy(newDoctor->docterId, id);
			strcpy(newDoctor->docterName, name);
			strcpy(newDoctor->department, deptName);
			newDoctor->subDeptId[0] = '\0';
			newDoctor->consultationCount = count;
		}

		newDoctor->next = sys->docHead;
		sys->docHead = newDoctor;
	}
	fclose(fp);
	printf(">>> 数据加载完成！\n");
}

// 将系统数据保存到txt文件
void saveDoctorSystemData(HIS_System* sys) {
	FILE* fp = fopen(DOCTOR_FILE, "w");
	if (!fp) {
		printf(">>> 错误: 无法创建或打开保存文件！\n");
		return;
	}
	Docter* curr = sys->docHead;
	while (curr != NULL) {
		const char* persistedRoom = (curr->subDeptId[0] != '\0') ? curr->subDeptId : "NULL";
		fprintf(fp, "%s %s %s %s %d\n", curr->docterId, curr->docterName, curr->department, persistedRoom, curr->consultationCount);
		curr = curr->next;
	}
	fclose(fp);
	printf(">>> 系统数据保存成功！\n");
}

