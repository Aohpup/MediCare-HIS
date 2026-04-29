#define _CRT_SECURE_NO_WARNINGS
#include"doctorFileManage.h"
#include"doctorManage.h"
#include"QueueManage.h"
#include<string.h>
//TODO:保存时按序号正序保存，加载时按文件顺序加载（即医生信息行后紧跟该医生的排班信息行），以保证数据一致性和正确性
bool is_Doctor_File_Loaded = false;		//标记是否加载过医生数据

// 追加医生节点到系统中（用于从文件加载数据时创建医生节点）
static doctor* appendDoctorNode(HIS_System* sys, const char* id, const char* name, const char* deptName, const char* subDeptName, const char* roomId) {
	doctor* newDoctor = (doctor*)malloc(sizeof(doctor));
	if (newDoctor == NULL) {
		return NULL;
	}
	memset(newDoctor, 0, sizeof(doctor));
	strcpy(newDoctor->doctorId, id);
	strcpy(newDoctor->doctorName, name);
	strcpy(newDoctor->department, deptName);

	if (subDeptName == NULL || strcmp(subDeptName, "NULL") == 0 || strcmp(subDeptName, "-") == 0) {
		newDoctor->subDepartment[0] = '\0';
	}
	else {
		strcpy(newDoctor->subDepartment, subDeptName);
	}

	if (roomId == NULL || strcmp(roomId, "NULL") == 0 || strcmp(roomId, "-") == 0) {
		newDoctor->subDeptId[0] = '\0';
	}
	else {
		strcpy(newDoctor->subDeptId, roomId);
	}

	newDoctor->scheduleHead = NULL;
	newDoctor->next = sys->docHead;
	sys->docHead = newDoctor;
	return newDoctor;
}

// 从txt文件加载系统数据
void loadDoctorSystemData(HIS_System* sys) {
	if (is_Doctor_File_Loaded) {
		if (TEST_SYSTEM_DEBUG)
			printf(">>> 医生数据已加载过，跳过重复加载。\n");
		return;
	}
	if (TEST_SYSTEM_DEBUG)
		printf(">>> 正在从医生文件中加载数据...\n");

	FILE* fp = fopen(DOCTOR_FILE, "r");
	if (!fp) {
		if (!TEST_SYSTEM_DEBUG) {
			printf("严重错误: 医生数据不存在！请确保文件存在或联系管理员。\n");
			exit(EXIT_FAILURE);
		}
		printf(">>> 警告: 找不到 %s，将作为新系统启动。\n", DOCTOR_FILE);
		return;
	}

	char line[512];
	doctor* currentDoctor = NULL;
	while (fgets(line, sizeof(line), fp) != NULL) {
		line[strcspn(line, "\r\n")] = '\0';
		if (line[0] == '\0' || line[0] == '#') {
			continue;
		}

		// 新格式：D id name dept subDeptName roomId
		if (strncmp(line, "D ", 2) == 0) {
			char id[ID_LEN] = { 0 };
			char name[STR_LEN] = { 0 };
			char deptName[STR_LEN] = { 0 };
			char subDeptName[STR_LEN] = { 0 };
			char roomId[ID_LEN] = { 0 };
			int parsed = sscanf(line + 2, "%24s %49s %49s %49s %24s", id, name, deptName, subDeptName, roomId);
			if (parsed == 5) {
				currentDoctor = appendDoctorNode(sys, id, name, deptName, subDeptName, roomId);
			}
			else if (parsed == 4) {
				// 兼容无 roomId 数据
				currentDoctor = appendDoctorNode(sys, id, name, deptName, subDeptName, "NULL");
			}
			else {
				currentDoctor = NULL;
			}
			continue;
		}

		// 密码行：P password
		if (strncmp(line, "P ", 2) == 0) {
			if (currentDoctor != NULL) {
				strcpy(currentDoctor->password, line + 2);
			}
			continue;
		}

		// 排班信息行：S YYYY-MM-DD slotNo bookingCount
		if (strncmp(line, "S ", 2) == 0) {
			if (currentDoctor == NULL) {
				continue;
			}
			char date[DATE_STR_LEN] = { 0 };
			int slotNo = 0;
			int bookingCount = 0;
			if (sscanf(line + 2, "%19s %d %d", date, &slotNo, &bookingCount) == 3) {
				if (slotNo >= 1 && slotNo <= SLOT_COUNT) {
					importDoctorSchedule(currentDoctor->doctorId, date, (TimeSlot)slotNo, bookingCount);
				}
			}
			continue;
		}

		if (strcmp(line, "END") == 0) {
			currentDoctor = NULL;
			continue;
		}
	}

	fclose(fp);
	printf(">>> 医生数据加载完成！\n");
	is_Doctor_File_Loaded = true;
}

// 将系统数据保存到txt文件
void saveDoctorSystemData(HIS_System* sys) {
	FILE* fp = fopen(DOCTOR_FILE, "w");
	if (!fp) {
		if (!TEST_SYSTEM_DEBUG) {
			printf("严重错误: 无法保存文件！请确保文件权限或联系管理员。\n");
			exit(EXIT_FAILURE);
		}
		printf(">>> 错误: 无法创建或打开保存文件！\n");
		return;
	}

	fprintf(fp, "# HIS doctor DATA FILE\n");

	doctor* curr = sys->docHead;
	while (curr != NULL) {
		const char* persistedSubDept = (curr->subDepartment[0] != '\0') ? curr->subDepartment : "NULL";
		const char* persistedRoom = (curr->subDeptId[0] != '\0') ? curr->subDeptId : "NULL";
		// 新格式：D id name dept subDeptName roomId
		fprintf(fp, "D %s %s %s %s %s\n", curr->doctorId, curr->doctorName, curr->department, persistedSubDept, persistedRoom);
		fprintf(fp, "P %s\n", (curr->password[0] != '\0') ? curr->password : "NULL");
		exportDoctorSchedules(fp, curr->doctorId);
		fprintf(fp, "END\n");
		curr = curr->next;
	}
	fclose(fp);
	printf(">>> 医生数据保存成功！\n");
}

