#define _CRT_SECURE_NO_WARNINGS
#include"DocterFileManage.h"
#include"DocterManage.h"

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
		// txt文件中格式为: ID 姓名 所在科室 诊号数量
		if (sscanf(buffer, "%s %s %s %d", newDoctor->docterId, newDoctor->docterName, newDoctor->department, &newDoctor->consultationCount) == 4) {
			newDoctor->next = sys->docHead;
			sys->docHead = newDoctor;
		}
		else {
			free(newDoctor);
		}
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
		fprintf(fp, "%s %s %s %d\n", curr->docterId, curr->docterName, curr->department, curr->consultationCount);
		curr = curr->next;
	}
	fclose(fp);
	printf(">>> 系统数据保存成功！\n");
}

