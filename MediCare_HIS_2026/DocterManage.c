#define _CRT_SECURE_NO_WARNINGS
#include"HIS_System.h"
#include"DocterFileManage.h"
#include"DocterManage.h"
#include"DepartmentManage.h"
#include"DepartmentFileManage.h"
#include"ConfirmFunc.h"
#include"DocterSort.h"
#include"QueueManage.h"
#include"InputUtils.h"
#include<string.h>

bool isDoctorIdExist(Docter* head, const char* id) {
	Docter* curr = head;
	while (curr != NULL) {
		if (strcmp(curr->docterId, id) == 0) return true;
		curr = curr->next;
	}
	return false;
}

// 医生姓名防重复（可选功能，根据实际需求决定是否启用）
/*
bool isDoctorNameExist(Docter* head, const char* name) {
	Docter* curr = head;
	while (curr != NULL) {
		if (strcmp(curr->docterName, name) == 0) return true;
		curr = curr->next;
	}
	return false;
}
*/

static bool getSubDepartmentNameUnderCategory(HIS_System* sys, const char* categoryName, const char* subDeptId, char* outSubDeptName) {
	if (sys == NULL || categoryName == NULL || subDeptId == NULL) return false;
	Department* deptCurr = sys->deptHead;
	while (deptCurr != NULL) {
		if (strcmp(deptCurr->categoryName, categoryName) == 0) {
			SubDepartment* subCurr = deptCurr->subDeptHead;
			while (subCurr != NULL) {
				if (strcmp(subCurr->subDeptId, subDeptId) == 0) {
					if (outSubDeptName != NULL) {
						strcpy(outSubDeptName, subCurr->subDeptName);
					}
					return true;
				}
				subCurr = subCurr->next;
			}
			break;
		}
		deptCurr = deptCurr->next;
	}
	return false;
}

void addDoctor(HIS_System* sys) {
	if (sys == NULL) {
		printf(">>> 错误：系统未初始化！\n");
		return;
	}
	if (sys->deptHead == NULL) {
		printf(">>> 提示：当前尚未录入诊室，请先到科室管理模块录入科室后再添加医生。\n");
		return;
	}

	while (1) {
		printf("\n--- 录入新医生 (在任意输入环节输入 '-1' 可取消本次添加) ---\n");
		Docter* newDoctor = (Docter*)malloc(sizeof(Docter));
		if (newDoctor == NULL) {
			printf(">>> 内存分配失败！\n");
			return;
		}
		bool hasCancelFlag = false;

		// 输入并验证医生编号
		while (1) {
			safeGetString("请输入医生编号: ", newDoctor->docterId, ID_LEN);
			if (strcmp(newDoctor->docterId, "-1") == 0) {
				hasCancelFlag = true;
				break;
			}
			if (isDoctorIdExist(sys->docHead, newDoctor->docterId)) {
				printf(">>> 医生编号已存在，请重新输入！\n");
				continue;
			}
			break;
		}
		if (hasCancelFlag) {
			free(newDoctor);
			printf(">>> 已取消添加医生。\n");
			return;
		}

		// 输入并验证医生姓名
		while (1) {
			safeGetString("请输入医生姓名: ", newDoctor->docterName, STR_LEN);
			if (strcmp(newDoctor->docterName, "-1") == 0) {
				hasCancelFlag = true;
				break;
			}
			break;
		}
		if (hasCancelFlag) {
			free(newDoctor);
			printf(">>> 已取消添加医生。\n");
			return;
		}

		// 输入并验证医生所属一级科室
		while (1) {
			safeGetString("请输入医生所属一级科室名称: ", newDoctor->department, STR_LEN);
			if (strcmp(newDoctor->department, "-1") == 0) {
				hasCancelFlag = true;
				break;
			}
			if (!isDepartmentNameExist(sys->deptHead, newDoctor->department)) {
				printf(">>> 错误：系统中尚无该一级科室，请先创建或重新输入！\n");
				continue;
			}
			break;
		}
		if (hasCancelFlag) {
			free(newDoctor);
			printf(">>> 已取消添加医生。\n");
			return;
		}

		// 输入并验证医生所在诊室编号（一级科室 + 诊室编号唯一映射）
		while (1) {
			safeGetString("请输入医生所属诊室编号: ", newDoctor->subDeptId, ID_LEN);
			if (strcmp(newDoctor->subDeptId, "-1") == 0) {
				hasCancelFlag = true;
				break;
			}
			if (!getSubDepartmentNameUnderCategory(sys, newDoctor->department, newDoctor->subDeptId, NULL)) {
				printf(">>> 错误：诊室编号与所选一级科室不匹配，请重新输入！\n");
				continue;
			}
			break;
		}
		if (hasCancelFlag) {
			free(newDoctor);
			printf(">>> 已取消添加医生。\n");
			return;
		}

		newDoctor->consultationCount = 0;
		newDoctor->next = sys->docHead;
		sys->docHead = newDoctor;

		printf(">>> 医生 <%s> (Id: %s) 录入成功！\n", newDoctor->docterName, newDoctor->docterId);
		printf(">>> 提示：医生已与 <%s诊室: %s> 建立关联。\n", newDoctor->department, newDoctor->subDeptId);
		printf(">>> 提示：已自动继续添加医生；可在下一个输入环节输入 '-1' 退出。\n");
	}
}

void printDoctorInfo(Docter* doctor) {
	if (doctor == NULL) return;
	const char* deptName = (doctor->department[0] != '\0') ? doctor->department : "未绑定";
	const char* roomId = (doctor->subDeptId[0] != '\0') ? doctor->subDeptId : "未绑定";
	printf("\n========== 医生信息 ==========\n");
	printf("医生编号: %s\n", doctor->docterId);
	printf("医生姓名: %s\n", doctor->docterName);
	printf("所属一级科室: %s\n", deptName);
	printf("所属诊室编号: %s\n", roomId);
	printf("诊号数量: %d\n", doctor->consultationCount);
	printf("=============================\n");
}

void queryDoctor(HIS_System* sys) {
	if (sys->docHead == NULL) {
		printf("\n>>> 系统内没有医生数据！\n");
		return;
	}
	int choice;
	printf("\n--- 医生信息查询 ---\n");
	printf("1. 按医生编号查询\n");
	printf("2. 按医生姓名查询\n");
	printf("3. 按所属一级科室+诊室编号查询\n");
	printf("0. 返回上一级菜单\n");
	choice = safeGetInt("请选择查询方式: ");
	char queryStr[STR_LEN];
	char queryDept[STR_LEN];
	char queryRoom[ID_LEN];
	char subDeptName[STR_LEN];
	Docter* curr = sys->docHead;
	bool found = false;
	switch (choice) {
	case 1:
		safeGetString("请输入医生编号: ", queryStr, ID_LEN);
		while (curr != NULL) {
			if (strcmp(curr->docterId, queryStr) == 0) {
				printDoctorInfo(curr);
				if (curr->subDeptId[0] != '\0' && getSubDepartmentNameUnderCategory(sys, curr->department, curr->subDeptId, subDeptName)) {
					printf("关联诊室名称: %s\n", subDeptName);
				}
				found = true;
				break;
			}
			curr = curr->next;
		}
		break;
	case 2:
		safeGetString("请输入医生姓名: ", queryStr, STR_LEN);
		while (curr != NULL) {
			if (strcmp(curr->docterName, queryStr) == 0) {
				printDoctorInfo(curr);
				if (curr->subDeptId[0] != '\0' && getSubDepartmentNameUnderCategory(sys, curr->department, curr->subDeptId, subDeptName)) {
					printf("关联诊室名称: %s\n", subDeptName);
				}
				found = true;
				break;
			}
			curr = curr->next;
		}
		break;
	case 3:
		safeGetString("请输入所属一级科室名称: ", queryDept, STR_LEN);
		safeGetString("请输入诊室编号: ", queryRoom, ID_LEN);
		int personCount = 0;
		while (curr != NULL) {
			if (strcmp(curr->department, queryDept) == 0 && strcmp(curr->subDeptId, queryRoom) == 0) {
				printf("\n%s + %s 下第%d位医生:\n", queryDept, queryRoom, ++personCount);
				printDoctorInfo(curr);
				if (curr->subDeptId[0] != '\0' && getSubDepartmentNameUnderCategory(sys, curr->department, curr->subDeptId, subDeptName)) {
					printf("关联诊室名称: %s\n", subDeptName);
				}
				found = true;
			}
			curr = curr->next;
		}
		break;
	case 0:
		return;
	default:
		printf(">>> 无效选择，请重试！\n");
		break;
	}

	if (!found && choice != 0) {
		printf(">>> 没有找到匹配的医生信息！\n");
	}
}

void modifyDoctor(HIS_System* sys) {
	if(sys->docHead == NULL) {
		printf("\n>>> 系统内没有医生数据！\n");
		return;
	}

	int choice;
	printf("\n--- 修改医生信息 ---\n");
	printf("1. 按医生编号修改\n");
	printf("2. 按医生姓名修改\n");
	printf("3. 按所属诊室编号修改\n");
	printf("0. 返回上一级菜单\n");
	choice = safeGetInt("请选择修改目标(输入 -1 取消): ");

	if (choice == 0 || choice == -1) return;
	if (choice != 1 && choice != 2 && choice != 3) {
		printf(">>> 无效选择，请重试！\n");
		return;
	}

	char queryStr[STR_LEN];
	Docter* matches[100];
	int matchCount = 0;
	Docter* curr = sys->docHead;

	switch (choice) {
	case 1:
		safeGetString("请输入要修改的医生编号(输入 -1 取消): ", queryStr, ID_LEN);
		if (strcmp(queryStr, "-1") == 0) return;
		while (curr != NULL) {
			if (strcmp(curr->docterId, queryStr) == 0 && matchCount < 100) {
				matches[matchCount++] = curr;
			}
			curr = curr->next;
		}
		break;
	case 2:
		safeGetString("请输入要修改的医生姓名(输入 -1 取消): ", queryStr, STR_LEN);
		if (strcmp(queryStr, "-1") == 0) return;
		while (curr != NULL) {
			if (strcmp(curr->docterName, queryStr) == 0 && matchCount < 100) {
				matches[matchCount++] = curr;
			}
			curr = curr->next;
		}
		break;
	case 3:
		safeGetString("请输入要修改的所属诊室编号(输入 -1 取消): ", queryStr, ID_LEN);
		if (strcmp(queryStr, "-1") == 0) return;
		while (curr != NULL) {
			if (strcmp(curr->subDeptId, queryStr) == 0 && matchCount < 100) {
				matches[matchCount++] = curr;
			}
			curr = curr->next;
		}
		break;
	}

	if (matchCount == 0) {
		printf(">>> 没有找到匹配的医生信息！\n");
		return;
	}

	Docter* target = NULL;
	if (matchCount == 1) {
		target = matches[0];
		printf(">>> 已定位到目标医生：\n");
		printDoctorInfo(target);
	}
	else {
		printf("\n>>> 发现 %d 条匹配记录，请选择要修改的目标：\n", matchCount);
		for (int i = 0; i < matchCount; i++) {
			printf("\n#[%d]\n", i + 1);
			printDoctorInfo(matches[i]);
		}
		while (1) {
			int sel = safeGetInt("请输入对应序号: ");
			if (sel >= 1 && sel <= matchCount) {
				target = matches[sel - 1];
				break;
			}
			printf(">>> 序号无效，请重试。\n");
		}
	}

	printf("\n1. 修改医生编号\n");
	printf("2. 修改医生姓名\n");
	printf("3. 修改所属诊室编号\n");
	printf("4. 修改诊号数量\n");
	int modChoice = safeGetInt("请选择要修改的字段(输入 -1 取消): ");
	if (modChoice == -1) { printf(">>> 已取消修改。\n"); return; }

	printf("警告：请于修改前确认相关信息是否正确，以免误修改！\n");
	if (!confirmFunc("修改", "医生信息")) {
		printf(">>> 已取消修改。\n");
		return;
	}

	switch (modChoice) {
	case 1: {
		char newId[ID_LEN];
		while (1) {
			safeGetString("请输入新的医生编号(输入 -1 取消): ", newId, ID_LEN);
			if (strcmp(newId, "-1") == 0) break;
			if (strcmp(newId, target->docterId) == 0) {
				printf(">>> 新编号与原编号一致，无需修改。\n");
				break;
			}
			if (isDoctorIdExist(sys->docHead, newId)) {
				printf(">>> 医生编号已存在，请重新输入！\n");
				continue;
			}
			strcpy(target->docterId, newId);
			printf(">>> 医生编号修改成功！\n");
			break;
		}
		break;
	}
	case 2: {
		char newName[STR_LEN];
		safeGetString("请输入新的医生姓名(输入 -1 取消): ", newName, STR_LEN);
		if (strcmp(newName, "-1") != 0) {
			strcpy(target->docterName, newName);
			printf(">>> 医生姓名修改成功！\n");
		}
		break;
	}
	case 3: {
		char newDept[STR_LEN];
		char newRoom[ID_LEN];
		while (1) {
			safeGetString("请输入新的所属一级科室名称(输入 -1 取消): ", newDept, STR_LEN);
			if (strcmp(newDept, "-1") == 0) break;
			if (!isDepartmentNameExist(sys->deptHead, newDept)) {
				printf(">>> 错误：系统中尚无该一级科室，请重新输入！\n");
				continue;
			}
			while (1) {
				safeGetString("请输入新的诊室编号(输入 -1 取消): ", newRoom, ID_LEN);
				if (strcmp(newRoom, "-1") == 0) break;
				if (!getSubDepartmentNameUnderCategory(sys, newDept, newRoom, NULL)) {
					printf(">>> 错误：该诊室编号与所选一级科室不匹配，请重新输入！\n");
					continue;
				}
				strcpy(target->department, newDept);
				strcpy(target->subDeptId, newRoom);
				printf(">>> 医生所属科室/诊室修改成功！\n");
				break;
			}
			break;
		}
		break;
	}
	case 4: {
		while (1) {
			int newCount = safeGetInt("请输入新的诊号数量(输入 -1 取消): ");
			if (newCount == -1) break;
			if (newCount < 0) {
				printf(">>> 错误：诊号数量不能为负数！\n");
				continue;
			}
			target->consultationCount = newCount;
			printf(">>> 诊号数量修改成功！\n");
			break;
		}
		break;
	}
	default:
		printf(">>> 无效选择，已取消修改。\n");
		break;
	}
}

void deleteDoctor(HIS_System** sys) {
	while (1) {
		if ((*sys)->docHead == NULL) {
			printf("\n>>> 系统内没有医生数据！\n");
			return;
		}
		printf("请选择删除方式:\n");
		printf("1. 按医生编号删除\n");
		printf("2. 按医生姓名删除\n");
		printf("3. 按所属诊室编号删除\n");
		printf("0. 返回上一级菜单\n");
		int choice = safeGetInt("请输入选择: ");
		if (choice == 0) return; //返回上一级菜单
		else if (choice != 1 && choice != 2 && choice != 3) {
			printf(">>> 无效选择，请重试！\n");
			continue;
		}
		char queryStr[STR_LEN];
		Docter* curr = (*sys)->docHead;
		switch (choice) {
		case 1:
			safeGetString("请输入要删除的医生编号: ", queryStr, ID_LEN);
			deleteDoctorFunc(&curr, queryStr, 1);
			break;
		case 2:
			safeGetString("请输入要删除的医生姓名: ", queryStr, STR_LEN);
			deleteDoctorFunc(&curr, queryStr, 2);
			break;
		case 3:
			safeGetString("请输入要删除的所属诊室编号: ", queryStr, ID_LEN);
			deleteDoctorFunc(&curr, queryStr, 3);
			break;
		default:
			printf(">>> 无效选择，请重试！\n");
			break;
		}
	}

}

// 根据用户选择的删除方式和查询字符串，在医生链表中查找匹配的节点并删除
void deleteDoctorFunc(Docter** head, const char* queryStr, int mode) {
	Docter* curr = *head;
	Docter* prev = NULL;
	int personCount = 0; // 用于按诊室删除时记录匹配的医生数量
	confirmFunc("删除", "医生信息");
	while (curr != NULL) {
		bool match = false;
		switch (mode) {
			case 1: 
				match = (strcmp(curr->docterId, queryStr) == 0); 
				break;
			case 2: 
				match = (strcmp(curr->docterName, queryStr) == 0); 
				break;
			case 3: 
				// 按所在诊室删除时，先列出所有匹配的医生节点，最后询问用户删除目标，避免误删
				printf("医生界面只支持单次删除一位医生，若要一键删除整个诊室下的医生，请前往科室管理模块\n");
				printf("请根据以下列表确认要删除的医生信息:\n");
				if (strcmp(curr->subDeptId, queryStr) == 0) {
					printf("\n找到第%d位匹配的医生:\n", ++personCount);
					printDoctorInfo(curr);
					while(!match){
						char confirm[4];
						safeGetString(">>> 确认删除该医生吗？(yes/no): ", confirm, 4);
						if (strcmp(confirm, "yes") == 0) {
							match = true;
							break;
						}
						else if(strcmp(confirm, "no") == 0){
							printf(">>> 已跳过该医生，继续查找诊室 %s 下一个匹配的医生...\n", queryStr);
							break;
						}
						else {
							printf(">>> 无效输入！！！\n");
							clearerr(stdin);
							continue;
						}
					}
				}
				break;
			default: printf(">>> 无效的删除方式！\n"); return;
		}
		if (match) {
			if (prev == NULL) { // 删除头节点
				*head = curr->next;
			} else {
				prev->next = curr->next;
			}
			printf(">>> 医生 <%s> (Id: %s) 删除成功！\n\n", curr->docterName, curr->docterId);
			free(curr);
			return;
		} 
		else {
			prev = curr;
			curr = curr->next;
		}
	}
		printf(">>> 没有找到匹配的医生信息，删除失败！\n");
}

void displayAllDoctors(HIS_System* sys) {
	if (sys->docHead == NULL) {
		printf("\n>>> 系统内没有医生数据！\n");
		return;
	}
	Docter* curr = sys->docHead;
	int count = 0;
	while (curr != NULL) {
		printf("\n--- 医生 #%d ---\n", ++count);
		printDoctorInfo(curr);
		curr = curr->next;
	}
	printf(">>> 共计 %d 位医生信息显示完毕！\n", count);
}

void doctorScheduleMenu(HIS_System* sys) {
	char doctorId[ID_LEN];
	char date[DATE_STR_LEN];
	safeGetString(">>> 请输入需要排班的医生编号: ", doctorId, ID_LEN);
	Docter* doctor = sys->docHead;
	while (doctor != NULL && strcmp(doctor->docterId, doctorId) != 0) {
		doctor = doctor->next;
	}
	if (doctor == NULL) {
		printf(">>> 未找到该医生，无法排班。\n");
		return;
	}
	safeGetString(">>> 请输入排班日期(YYYY-MM-DD): ", date, DATE_STR_LEN);
	if (!isValidDate(date)) {
		printf(">>> 日期格式无效。\n");
		return;
	}
	printf("请选择要开放的时段(输入0结束)：\n");
	while (1) {
		printAllTimeSlots();
		int slotNo = safeGetInt(">>> 时段编号: ");
		if (slotNo == 0) {
			break;
		}
		if (slotNo < 1 || slotNo > SLOT_COUNT) {
			printf(">>> 时段无效，请重试。\n");
			continue;
		}
		if (openDoctorScheduleSlot(doctorId, date, (TimeSlot)slotNo)) {
			printf(">>> 已开放 [%s] 的号源。\n", slot_names[slotNo - 1]);
		}
	}
}

void doctorCallQueueMenu(HIS_System* sys) {
	(void)sys;
	char doctorId[ID_LEN];
	char date[DATE_STR_LEN];
	safeGetString(">>> 请输入叫号医生编号: ", doctorId, ID_LEN);
	safeGetString(">>> 请输入叫号日期(YYYY-MM-DD): ", date, DATE_STR_LEN);
	if (!isValidDate(date)) {
		printf(">>> 日期格式无效。\n");
		return;
	}
	printAllTimeSlots();
	int slotNo = safeGetInt(">>> 请选择叫号时段: ");
	if (slotNo < 1 || slotNo > SLOT_COUNT) {
		printf(">>> 时段编号无效。\n");
		return;
	}
	TimeSlot slot = (TimeSlot)slotNo;
	printSlotQueue(doctorId, date, slot);
	if (!confirmFunc("执行", "下一位患者叫号")) {
		return;
	}
	Patient* called = callNextPatient(doctorId, date, slot);
	if (called != NULL) {
		printf(">>> 请患者 %s (%s) 到诊室就诊。\n", called->name, called->patientId);
	}
	printSlotQueue(doctorId, date, slot);
}

void doctorManageMenu(HIS_System* sys) {
	if (sys == NULL) {
		printf(">>> 严重错误: 系统底座未初始化！！！\n");
		return;
	}

	if (sys->deptHead == NULL) {
		printf(">>> 提示：当前内存中尚无科室数据，医生需要绑定一级科室+诊室编号。\n");
		printf("没有同步科室数据，医生模块将无法正常使用，建议载入科室数据！\n");
		if (confirmFunc("同步", "科室数据到医生模块")) {
			loadDepartmentSystemData(sys);
		}
		else {
			return;
		}
	}

	loadDoctorSystemData(sys);   // 从文件加载数据

	int choice = -1;
	while (1) {
		printf("\n========== 医生管理中心 ==========\n");
		printf("1. 录入新医生\n");
		printf("2. 查询医生信息\n");
		printf("3. 修改医生数据\n");
		printf("4. 排序医生列表\n");
		printf("5. 删除医生记录\n");
		printf("6. 显示所有医生信息\n");
		printf("7. 医生排班\n");
		printf("8. 医生叫号\n");
		printf("9. 保存系统数据\n");
		printf("0. 返回上一级菜单\n");
		printf("==================================\n");
		choice = safeGetInt("请选择医生管理操作: ");
		switch (choice) {
		case 1:
			addDoctor(sys);
			break;
		case 2:
			queryDoctor(sys);
			break;
		case 3:
			modifyDoctor(sys);
			break;
		case 4:
			doctorSortMenu(sys);
			break;
		case 5:
			deleteDoctor(&sys);
			break;
		case 6:
			displayAllDoctors(sys);
			break;
		case 7:
			doctorScheduleMenu(sys);
			break;
		case 8:
			doctorCallQueueMenu(sys);
			break;
		case 9:
			if (confirmFunc("保存", "医生系统数据")) {
				saveDoctorSystemData(sys);
			}
			break;
		case 0:
			return;
		default:
			printf(">>> 无效选择，请重试。\n");
			break;
		}
	}
}





