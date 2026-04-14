#define _CRT_SECURE_NO_WARNINGS
#include"DocterFileManage.h"
#include"DocterManage.h"
#include"QueueManage.h"
#include<string.h>
bool is_Doctor_File_Loaded = false;		//标记是否加载过医生数据

static Docter* appendDoctorNode(HIS_System* sys, const char* id, const char* name, const char* deptName, const char* roomId) {
	Docter* newDoctor = (Docter*)malloc(sizeof(Docter));
	if (newDoctor == NULL) {
		return NULL;
	}
	strcpy(newDoctor->docterId, id);
	strcpy(newDoctor->docterName, name);
	strcpy(newDoctor->department, deptName);
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
		return;
	}
	if(TEST_SYSTEM_DEBUG)
	printf(">>> 正在从医生文件中加载数据...\n");
	FILE* fp = fopen(DOCTOR_FILE, "r");
	if (!fp) {
		if(!TEST_SYSTEM_DEBUG) {
			printf("严重错误: 医生数据不存在！请确保文件存在或联系管理员。\n");
			exit(EXIT_FAILURE); // 直接退出程序，避免后续操作导致更严重的错误
		}
		printf(">>> 警告: 找不到 %s，将作为新系统启动。\n", DOCTOR_FILE);
		return;
	}

	char line[512];	// 用于读取文件行的缓冲区
	Docter* currentDoctor = NULL;
	while (fgets(line, sizeof(line), fp) != NULL) {
		line[strcspn(line, "\r\n")] = '\0';
		if (line[0] == '\0') {
			continue;
		}

		// S.6医生信息行，格式：D DOC001 张三 内科 NULL
		if (strncmp(line, "D ", 2) == 0) {
			char id[ID_LEN] = { 0 };
			char name[STR_LEN] = { 0 };
			char deptName[STR_LEN] = { 0 };
			char roomId[ID_LEN] = { 0 };
			int parsed = sscanf(line + 2, "%24s %49s %49s %24s", id, name, deptName, roomId);	// 解析医生信息行
			if (parsed < 4) { // 解析失败，格式不正确，跳过该行并继续读取下一行
				currentDoctor = NULL;
				continue;
			}
			// 创建医生节点并添加到系统中
			currentDoctor = appendDoctorNode(sys, id, name, deptName, roomId);
			continue;
		}

		// 兼容旧格式：S.5医生排班信息行，格式：S 2026-01-01 1 3
		if (strncmp(line, "S ", 2) == 0) {
			if (currentDoctor == NULL) {
				continue;
			}

			// 解析排班信息行
			char date[DATE_STR_LEN] = { 0 };
			int slotNo = 0;
			int bookingCount = 0;
			if (sscanf(line + 2, "%19s %d %d", date, &slotNo, &bookingCount) == 3) {	// 解析成功，继续处理排班信息
				if (slotNo >= 1 && slotNo <= SLOT_COUNT) {
					importDoctorSchedule(currentDoctor->docterId, date, (TimeSlot)slotNo, bookingCount);
				}
			}
			continue;
		}

		if (strcmp(line, "END") == 0) {
			currentDoctor = NULL;
			continue;
		}

		//不出诊医生的兼容处理：如果当前行不以 "D " 或 "S " 开头，也不是 "END"，但当前医生节点不为NULL，则尝试解析为医生信息行（兼容旧格式）
		{
			char id[ID_LEN] = { 0 };
			char name[STR_LEN] = { 0 };
			char deptName[STR_LEN] = { 0 };
			char roomId[ID_LEN] = { 0 };
			int parsed = sscanf(line, "%24s %49s %49s %24s", id, name, deptName, roomId);
			if (parsed >= 4) {
				currentDoctor = appendDoctorNode(sys, id, name, deptName, roomId);
			}
		}
	}
	fclose(fp);
	printf(">>> 数据加载完成！\n");
	is_Doctor_File_Loaded = true;	//标记已加载过医生数据

}

// 将系统数据保存到txt文件
void saveDoctorSystemData(HIS_System* sys) {
	FILE* fp = fopen(DOCTOR_FILE, "w");
	if (!fp) {
		if (!TEST_SYSTEM_DEBUG) {
			printf("严重错误: 无法保存文件！请确保文件权限或联系管理员。\n");
			exit(EXIT_FAILURE); // 直接退出程序，避免数据丢失或后续操作导致更严重的错误
		}
		printf(">>> 错误: 无法创建或打开保存文件！\n");
		return;
	}
	Docter* curr = sys->docHead;
	while (curr != NULL) {
		const char* persistedRoom = (curr->subDeptId[0] != '\0') ? curr->subDeptId : "NULL";
		fprintf(fp, "D %s %s %s %s\n", curr->docterId, curr->docterName, curr->department, persistedRoom);
		exportDoctorSchedules(fp, curr->docterId);
		fprintf(fp, "END\n");
		curr = curr->next;
	}
	fclose(fp);
	printf(">>> 医生数据保存成功！\n");
}

