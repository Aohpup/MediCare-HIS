#define _CRT_SECURE_NO_WARNINGS
#include"HIS_System.h"
#include"doctorFileManage.h"
#include"doctorManage.h"
#include"DepartmentManage.h"
#include"DepartmentFileManage.h"
#include"PatientManage.h"
#include"PatientFileManage.h"
#include"QueueManage.h"
#include"QueueFileManage.h"
#include"ConfirmFunc.h"
#include"PauseUtil.h"
#include"doctorSort.h"
#include"InputUtils.h"
#include"StringCheck.h"
#include"WardManage.h"
#include"WardFileManage.h"
#include"StringCheck.h"
#include"DayTimeUtils.h"
#include<string.h>

bool is_Doctor_Logged_In = false;	//标记医生是否已登录

char currentDoctorId[ID_LEN];		//当前登录医生的编号

bool logInDoctor(HIS_System* sys) {
	if (sys == NULL) {
		printf(">>> 错误：系统未初始化！\n");
		return false;
	}
	if (sys->docHead == NULL) {
		printf(">>> 提示：当前尚未录入医生，请先到医生管理模块录入医生后再登录。\n");
		return false;
	}
	while (1) {
		char doctorId[ID_LEN];
		safeGetString("请输入医生编号进行登录:（输入 -1 取消登录）", doctorId, ID_LEN);
		if (strcmp(doctorId, "-1") == 0) {
			printf(">>> 已取消登录。\n");
			return false;
		}
		doctor* curr = sys->docHead;
		while (curr != NULL) {
			if (strcmp(curr->doctorId, doctorId) == 0) {
				// 密码验证
				char inputPwd[STR_LEN];
				safeGetString("请输入登录密码(输入 -1 取消登录): ", inputPwd, STR_LEN);
				if (strcmp(inputPwd, "-1") == 0) {
					printf(">>> 已取消登录。\n");
					return false;
				}
				// 空密码（兼容旧数据）或密码匹配则允许登录
				if (curr->password[0] == '\0' || strcmp(curr->password, inputPwd) == 0) {
					is_Doctor_Logged_In = true;
					strcpy(currentDoctorId, doctorId);
					printf(">>> 医生 <%s> 登录成功！\n", curr->doctorName);
					return true;
				}
				printf(">>> 密码错误，请重新输入医生编号！\n");
				break;	// 跳出内层 while，回到外层重新输入编号
			}
			curr = curr->next;
		}
		if (curr == NULL) {
			printf(">>> 错误：医生编号不存在，请重新输入！\n");
		}
		continue;
	}
}

char* getCurrentDoctorId(void) {
	if (is_Doctor_Logged_In) {
		return currentDoctorId;
	} else {
		return NULL;
	}
}

bool isDoctorIdExist(doctor* head, const char* id) {
	doctor* curr = head;
	while (curr != NULL) {
		if (strcmp(curr->doctorId, id) == 0) return true;
		curr = curr->next;
	}
	return false;
}

// 该函数目前未被使用，但保留以备后续可能的需求
bool isDoctorNameExist(doctor* head, const char* name) {
	doctor* curr = head;
	while (curr != NULL) {
		if (strcmp(curr->doctorName, name) == 0) return true;
		curr = curr->next;
	}
	return false;
}

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
		doctor* newDoctor = (doctor*)malloc(sizeof(doctor));
		if (newDoctor == NULL) {
			printf(">>> 内存分配失败！\n");
			return;
		}
		memset(newDoctor, 0, sizeof(doctor));
		bool hasCancelFlag = false;

		// 输入并验证医生编号
		while (1) {
			safeGetString("请输入医生编号: ", newDoctor->doctorId, ID_LEN);
			if (strcmp(newDoctor->doctorId, "-1") == 0) {
				hasCancelFlag = true;
				break;
			}
			if (isDoctorIdExist(sys->docHead, newDoctor->doctorId)) {
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
			safeGetString("请输入医生姓名: ", newDoctor->doctorName, STR_LEN);
			if (strcmp(newDoctor->doctorName, "-1") == 0) {
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
			if (!getSubDepartmentNameUnderCategory(sys, newDoctor->department, newDoctor->subDeptId, newDoctor->subDepartment)) {
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

		newDoctor->scheduleHead = NULL;
		sprintf(newDoctor->password, "%s@new", newDoctor->doctorId);
		newDoctor->next = sys->docHead;
		sys->docHead = newDoctor;

		printf(">>> 医生 <%s> (Id: %s) 录入成功！\n", newDoctor->doctorName, newDoctor->doctorId);
		printf(">>> 提示：医生已与 <%s诊室: %s> 建立关联。\n", newDoctor->department, newDoctor->subDeptId);
		printf(">>> 提示：已自动继续添加医生；可在下一个输入环节输入 '-1' 退出。\n");
	}
}

void printDoctorInfo(doctor* doctor) {
	if (doctor == NULL) return;
	const char* deptName = (doctor->department[0] != '\0') ? doctor->department : "未绑定";
	const char* roomId = (doctor->subDeptId[0] != '\0') ? doctor->subDeptId : "未绑定";
	printf("\n========== 医生信息 ==========\n");
	printf("医生编号: %s\n", doctor->doctorId);
	printf("医生姓名: %s\n", doctor->doctorName);
	printf("所属一级科室: %s\n", deptName);
	printf("所属二级科室: %s\n", (doctor->subDepartment[0] != '\0') ? doctor->subDepartment : "未绑定");
	printf("所属诊室编号: %s\n", (roomId[0] != '\0') ? roomId : "未绑定");
	//printf("诊号数量: %d\n", doctor->consultationCount);
	printf("=============================\n");
	pressEnterToContinue();
}

// 按姓名查询医生，统一处理重名：唯一显示，重名则带序号展示并让用户选择
// 返回 true 表示找到并已显示
static bool queryDoctorByNameWithDupHandling(HIS_System* sys, const char* name) {
	doctor* matches[100];
	int matchCount = 0;			// 最多支持100个同名医生的查询结果，超过则只显示前100个，请联系管理员优化数据
	doctor* curr = sys->docHead;
	while (curr != NULL) {
		if (strcmp(curr->doctorName, name) == 0 && matchCount < 100) {
			matches[matchCount++] = curr;
		}
		curr = curr->next;
	}
	if (matchCount == 0) return false;

	if (matchCount == 1) {
		printDoctorInfo(matches[0]);
		return true;
	}

	// 重名：带序号展示
	printf("\n>>> 找到 %d 位同名医生:\n", matchCount);
	for (int i = 0; i < matchCount; i++) {
		printf("#%d:", i + 1);
		printDoctorInfo(matches[i]);
	}
	int sel = safeGetInt("请选择您要进一步查看的医生序号(输入 -1 取消): ");
	if (sel == -1) {
		printf(">>> 已取消选择。\n");
		return true;
	}
	if (sel >= 1 && sel <= matchCount) {
		printf("\n--- 您选择的医生详细信息 ---\n");
		printDoctorInfo(matches[sel - 1]);
	}
	return true;
}

void queryDoctor(HIS_System* sys, const char* doctorId) {
	if (sys->docHead == NULL) {
		printf("\n>>> 系统内没有医生数据！\n");
		return;
	}
	if(doctorId != NULL) {
		doctor* curr = sys->docHead;
		while (curr != NULL) {
			if (strcmp(curr->doctorId, doctorId) == 0) {
				printDoctorInfo(curr);
				return;
			}
			curr = curr->next;
		}
		printf(">>> 没有找到医生编号 %s 的信息！请检查是否注册。\n", doctorId);
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
	doctor* curr = sys->docHead;
	bool found = false;
	switch (choice) {
	case 1:
		safeGetString("请输入医生编号: ", queryStr, ID_LEN);
		while (curr != NULL) {
			if (strcmp(curr->doctorId, queryStr) == 0) {
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
		found = queryDoctorByNameWithDupHandling(sys, queryStr);
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

// 患者端医生查询二级菜单
void queryDoctorPat(HIS_System* sys, const char* patientId) {
	if (sys == NULL) {
		printf(">>> 系统未初始化。\n");
		return;
	}
	if (!isPatientLoggedIn()) {
		printf(">>> 您尚未登录，请先登录后再进行查询！\n");
		return;
	}
	if (sys->docHead == NULL) {
		printf("\n>>> 系统内没有医生数据！\n");
		return;
	}

	int choice;
	char queryStr[STR_LEN];

	while (1) {
		printf("\n--- 医生信息查询 ---\n");
		printf("1. 按一级科室查询\n");
		printf("2. 按序号查询\n");
		printf("3. 按名字查询\n");
		printf("0. 返回上一级菜单\n");
		choice = safeGetInt("请选择查询方式: ");

		switch (choice) {
		case 1: {
			// 列出所有一级科室
			Department* deptArr[100];
			int deptCount = 0;
			Department* d = sys->deptHead;
			if (d == NULL) {
				printf(">>> 系统中暂无科室数据。\n");
				break;
			}
			printf("\n--- 科室列表 ---\n");
			while (d != NULL && deptCount < 100) {
				deptArr[deptCount++] = d;
				printf("%d. %s\n", deptCount, d->categoryName);
				d = d->next;
			}

			char input[STR_LEN];
			safeGetString("请输入科室序号或名称(输入 -1 返回): ", input, STR_LEN);
			if (strcmp(input, "-1") == 0) break;

			Department* targetDept = NULL;
			if (isAllDigits(input)) {
				int idx = atoi(input);
				if (idx >= 1 && idx <= deptCount)
					targetDept = deptArr[idx - 1];
			} else {
				for (int i = 0; i < deptCount; i++) {
					if (strcmp(deptArr[i]->categoryName, input) == 0) {
						targetDept = deptArr[i];
						break;
					}
				}
			}
			if (targetDept == NULL) {
				printf(">>> 未找到该科室。\n");
				break;
			}

			// 筛选该科室医生
			doctor* cur = sys->docHead;
			int foundCount = 0;
			while (cur != NULL) {
				if (strcmp(cur->department, targetDept->categoryName) == 0) {
					foundCount++;
					printDoctorInfo(cur);
				}
				cur = cur->next;
			}
			if (foundCount == 0) {
				printf(">>> %s 科室下暂无医生。\n", targetDept->categoryName);
			} else {
				printf(">>> 共找到 %d 位 %s 科室医生。\n", foundCount, targetDept->categoryName);
			}
			break;
		}
		case 2: {
			// 按医生编号查询
			char doctorId[ID_LEN];
			safeGetString("请输入医生编号(输入 -1 返回): ", doctorId, ID_LEN);
			if (strcmp(doctorId, "-1") == 0) break;
			doctor* cur = sys->docHead;
			bool found = false;
			while (cur != NULL) {
				if (strcmp(cur->doctorId, doctorId) == 0) {
					printDoctorInfo(cur);
					found = true;
					break;
				}
				cur = cur->next;
			}
			if (!found)
				printf(">>> 没有找到医生编号 %s 的信息！\n", doctorId);
			break;
		}
		case 3: {
			// 按医生姓名查询（含重名处理）
			char name[STR_LEN];
			safeGetString("请输入医生姓名(输入 -1 返回): ", name, STR_LEN);
			if (strcmp(name, "-1") == 0) break;
			if (!queryDoctorByNameWithDupHandling(sys, name))
				printf(">>> 没有找到医生姓名 %s 的信息！\n", name);
			break;
		}
		case 0:
			return;
		default:
			printf(">>> 无效选择，请重试！\n");
			break;
		}
	}
}

void modifyDoctor(HIS_System* sys) {
	if(sys->docHead == NULL) {
		printf("\n>>> 系统内没有医生数据！\n");
		return;
	}

	int choice;
	char queryStr[STR_LEN];
	doctor* matches[100];
	int matchCount = 0;
	doctor* curr = sys->docHead;
	doctor* target = NULL;

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

		switch (choice) {
		case 1:
			safeGetString("请输入要修改的医生编号(输入 -1 取消): ", queryStr, ID_LEN);
			if (strcmp(queryStr, "-1") == 0) return;
			while (curr != NULL) {
				if (strcmp(curr->doctorId, queryStr) == 0 && matchCount < 100) {
					matches[matchCount++] = curr;
				}
				curr = curr->next;
			}
			break;
		case 2:
			safeGetString("请输入要修改的医生姓名(输入 -1 取消): ", queryStr, STR_LEN);
			if (strcmp(queryStr, "-1") == 0) return;
			while (curr != NULL) {
				if (strcmp(curr->doctorName, queryStr) == 0 && matchCount < 100) {
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
			if (strcmp(newId, target->doctorId) == 0) {
				printf(">>> 新编号与原编号一致，无需修改。\n");
				break;
			}
			if (isDoctorIdExist(sys->docHead, newId)) {
				printf(">>> 医生编号已存在，请重新输入！\n");
				continue;
			}
			strcpy(target->doctorId, newId);
			printf(">>> 医生编号修改成功！\n");
			break;
		}
		break;
	}
	case 2: {
		char newName[STR_LEN];
		safeGetString("请输入新的医生姓名(输入 -1 取消): ", newName, STR_LEN);
		if (strcmp(newName, "-1") != 0) {
			strcpy(target->doctorName, newName);
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
				if (!getSubDepartmentNameUnderCategory(sys, newDept, newRoom, target->subDepartment)) {
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
		doctor* curr = (*sys)->docHead;
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
void deleteDoctorFunc(doctor** head, const char* queryStr, int mode) {
	doctor* curr = *head;
	doctor* prev = NULL;
	int personCount = 0; // 用于按诊室删除时记录匹配的医生数量
	confirmFunc("删除", "医生信息");
	while (curr != NULL) {
		bool match = false;
		switch (mode) {
			case 1: 
				match = (strcmp(curr->doctorId, queryStr) == 0); 
				break;
			case 2: 
				match = (strcmp(curr->doctorName, queryStr) == 0); 
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
			printf(">>> 医生 <%s> (Id: %s) 删除成功！\n\n", curr->doctorName, curr->doctorId);
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
	doctor* curr = sys->docHead;
	int count = 0;
	while (curr != NULL) {
		printf("\n--- 医生 #%d ---\n", ++count);
		printDoctorInfo(curr);
		curr = curr->next;
	}
	printf(">>> 共计 %d 位医生信息显示完毕！\n", count);
	pressEnterToContinue();
}

// 以下函数实现医生排班设置和取消功能，医生可以选择特定日期的特定时间段进行排班或取消排班
static void setDoctorSchedule(HIS_System* sys, char* doctorId, const char* date) {
	// 若当前已登录医生则直接使用其编号，否则手动输入
	doctor* Doctor = NULL;
	if (doctorId[0] == '\0') {
		safeGetString(">>> 请输入需要排班的医生编号: ", doctorId, ID_LEN);
		Doctor = findDoctorByIdInQueue(sys, doctorId);
		if(Doctor == NULL) {
			printf(">>> 未找到该医生，无法设置排班。\n");
			return;
		}

	}
	else {	// 已登录医生只能设置自己的排班
		Doctor = findDoctorByIdInQueue(sys, doctorId);
		if(Doctor == NULL) {
			printf(">>> 未找到该医生，无法设置排班。\n");
			return;
		}
		printf(">>> 当前登录医生: %s，将为其设置排班。\n", Doctor->doctorName);
	}

	safeGetString(">>> 请输入排班日期(YYYY-MM-DD): ", date, DATE_STR_LEN);
	if (!isValidDate(date)) {
		printf(">>> 日期格式无效。\n");
		return;
	}
	printf("请选择要开放的时段(输入 0 或 -1 结束)：\n");
	while (1) {
		printAllTimeSlots();
		int slotNo = safeGetInt(">>> 时段编号: ");
		if (slotNo == 0 || slotNo == -1) {
			printf(">>> 已结束排班设置。\n");
			break;
		}
		if (slotNo < 1 || slotNo > SLOT_COUNT) {
			printf(">>> 时段无效，请重试。\n");
			continue;
		}
		if (isNoonSlot((TimeSlot)slotNo)) {
			printf(">>> 午休时段（11:30-13:30）暂不开放看诊，请选择其他时段。\n");
			continue;
		}
		if (isDoctorSlotOpen(doctorId, date, (TimeSlot)slotNo)) {
			printf(">>> 时段 [%s] 已经开放过了，无需重复开放。\n", slot_names[slotNo - 1]);
			continue;
		}
		if (openDoctorScheduleSlot(doctorId, date, (TimeSlot)slotNo)) {
			printf(">>> 已开放 [%s] 的号源。\n", slot_names[slotNo - 1]);
		}
	}
}

static void cancelDoctorSchedule(HIS_System* sys, char* doctorId, const char* date) {
	// 若当前已登录医生则直接使用其编号，否则手动输入
	if (doctorId[0] == '\0') {
		safeGetString(">>> 请输入需要取消排班的医生编号: ", doctorId, ID_LEN);
	} else {
		printf(">>> 当前登录医生: %s，将为其取消排班。\n", doctorId);
	}
	doctor* doctor = sys->docHead;
	while (doctor != NULL && strcmp(doctor->doctorId, doctorId) != 0) {
		doctor = doctor->next;
	}
	if (doctor == NULL) {
		printf(">>> 未找到该医生，无法取消排班。\n");
		return;
	}
	safeGetString(">>> 请输入取消排班的日期(YYYY-MM-DD): ", date, DATE_STR_LEN);
	if (!isValidDate(date)) {
		printf(">>> 日期格式无效。\n");
		return;
	}
	printf("请选择要取消的时段(输入 0 或 -1 结束)：\n");
	while (1) {
		printAllTimeSlots();
		int slotNo = safeGetInt(">>> 时段编号: ");
		if (slotNo == 0 || slotNo == -1) {
			printf(">>> 已结束排班取消。\n");
			break;
		}
		if (slotNo < 1 || slotNo > SLOT_COUNT) {
			printf(">>> 时段无效，请重试。\n");
			continue;
		}
		if (!isDoctorSlotOpen(doctorId, date, (TimeSlot)slotNo)) {
			printf(">>> 时段 [%s] 本来就没有开放，无需取消。\n", slot_names[slotNo - 1]);
			continue;
		}
		if (cancelDoctorScheduleSlot(doctorId, date, (TimeSlot)slotNo)) {
			printf(">>> 已取消 [%s] 的号源。\n", slot_names[slotNo - 1]);
		}
	}
}

void doctorScheduleMenu(HIS_System* sys, const char* currentDoctorId) {
	if(!hasScheduleData()) {
		if(TEST_SYSTEM_DEBUG){
			printf(">>> 提示：当前内存中尚无医生排班数据，建议先同步医生数据！\n");
			printf(">>> 请确保已加载包含 S 行排班信息的 HIS_doctors.txt。\n");
			return;
		}
		else {
			printf(">>> 严重错误: 系统内没有医生排班数据，无法进入排班设置界面！！！\n");
			exit(EXIT_FAILURE);
		}
	}
	char date[DATE_STR_LEN];
	int choice = -1;
	while (1) {
		printf("\n--- 医生排班设置 ---\n");
		printf("1. 显示当前排班\n");
		printf("2. 设置排班\n");
		printf("3. 取消排班\n");
		printf("0. 返回上一级菜单\n");
		choice = safeGetInt(">>> 请选择操作: ");
		switch (choice)
		{
		case 1: {
			if (confirmFunc("查看", "今日排班")) {
				strcpy(date, getCurrentDateStr());
			}
			else {
			safeGetString(">>> 请输入查看日期(YYYY-MM-DD): ", date, DATE_STR_LEN);
			}

			if (!isValidDate(date)) {
				printf(">>> 日期格式无效。\n");
				return;
			}
			printDoctorScheduleTable(sys, currentDoctorId, date);
			break;
		}
		case 2:	setDoctorSchedule(sys, currentDoctorId, date); break;
		case 3: cancelDoctorSchedule(sys, currentDoctorId, date); break;
		case 0:	return;
		default: printf(">>> 无效选择，请重试！\n"); return;
		}
	}
}

void doctorCallQueueMenu(HIS_System* sys, const char* currentDoctorId) {
	(void)sys;  // 当前函数不直接操作系统底座数据，但保留 sys 参数以便未来扩展使用
	char doctorId[ID_LEN];
	char date[DATE_STR_LEN];
	if (TEST_SYSTEM_DEBUG)
		if (currentDoctorId == NULL) {
			safeGetString("请输入要叫号的医生", doctorId, ID_LEN);
		}
		else
			strcpy(doctorId, currentDoctorId);	
	else 
		strcpy(doctorId, currentDoctorId);

	if(TEST_SYSTEM_DEBUG) {
		if (confirmFunc("选择", "当前日期"))
			strcpy(date, getCurrentDateStr());
		else
			safeGetString(">>> 请输入叫号日期(YYYY-MM-DD): ", date, DATE_STR_LEN);
	}
	else
		strcpy(date, getCurrentDateStr());

	if (!isValidDate(date)) {
		printf(">>> 日期格式无效。\n");
		return;
	}
	int slotNo;

	if (TEST_SYSTEM_DEBUG) {
		if (confirmFunc("选择", "当前时间段"))
			slotNo = changeTimeToSlot(getCurrentTimeStr());
		else{
			printAllTimeSlots();
			slotNo = safeGetInt(">>> 请选择叫号时段: ");
		}
	}
	else
		slotNo = changeTimeToSlot(getCurrentTimeStr());

	if (slotNo < 1 || slotNo > SLOT_COUNT) {
		printf(">>> 时段编号无效。\n");
		return;
	}
	TimeSlot slot = (TimeSlot)slotNo;
	printf("当前列表：\n");
	printSlotQueue(doctorId, date, slot);
	if (!confirmFunc("执行", "下一位患者叫号")) {
		return;
	}
	Patient* called = callNextPatient(doctorId, date, slot);
	if (called != NULL) {
		printf(">>> 请患者 %s (%s) 到诊室就诊。\n", called->name, called->patientId);
		pressEnterToContinue();
	}
}

void doctorViewScheduleBoardMenu(HIS_System* sys) {
	(void)sys;
	char doctorId[ID_LEN];
	char date[DATE_STR_LEN];
	safeGetString(">>> 请输入医生编号: ", doctorId, ID_LEN);
	safeGetString(">>> 请输入查看日期(YYYY-MM-DD): ", date, DATE_STR_LEN);
	if (!isValidDate(date)) {
		printf(">>> 日期格式无效。\n");
		return;
	}
	printDoctorScheduleTable(sys, doctorId, date);
	if (confirmFunc("查看", "指定时段候诊队列")) {
		printAllTimeSlots();
		int slotNo = safeGetInt(">>> 请选择时段编号: ");
		if (slotNo >= 1 && slotNo <= SLOT_COUNT) {
			printSlotQueue(doctorId, date, (TimeSlot)slotNo);
		}
	}
}

void doctorManageMenu(HIS_System* sys) {
	if (sys == NULL) {
		printf(">>> 严重错误: 系统底座未初始化！！！\n");
		return;
	}

	if (sys->deptHead == NULL) {
		if(TEST_SYSTEM_DEBUG) {
		printf(">>> 提示：当前内存中尚无科室数据，医生需要绑定一级科室+诊室编号。\n");
		printf("没有同步科室数据，医生模块将无法正常使用，建议载入科室数据！\n");
		}
		// 同步科室数据到病房模块
		// 在测试模式下直接提示用户是否同步，在正式模式下强制同步（因为没有科室数据病房模块无法使用）
		if (!TEST_SYSTEM_DEBUG || adminConfirmFunc("同步", "科室数据到医生模块")) {
			loadDepartmentSystemData(sys);
		}
		else {
			return;
		}
	}

	loadDoctorSystemData(sys);   // 从文件加载医生排班数据
	loadQueueTicketData(sys);    // 从文件加载排队挂号数据，确保候诊队列查询可用

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
		printf("8. 查看排班表与候诊队列\n");
		printf("9. 医生叫号\n");
		printf("10. 保存系统数据\n");
		printf("0. 返回上一级菜单\n");
		printf("==================================\n");
		choice = safeGetInt("请选择医生管理操作: ");
		switch (choice) {
		case 1:
			addDoctor(sys);
			break;
		case 2:
			queryDoctor(sys, NULL);
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
			doctorScheduleMenu(sys, "curr_logged_in_doctor_id");		//TODO
			break;
		case 8:
			doctorViewScheduleBoardMenu(sys);
			break;
		case 9:
			doctorCallQueueMenu(sys, "curr_logged_in_doctor_id");		//TODO
			break;
		case 10:
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

void doctorManageMenuDoc(HIS_System* sys, const char* doctorId) {
	if (sys == NULL) {
		printf(">>> 严重错误: 系统底座未初始化！！！\n");
		return;
	}
	if (doctorId == NULL) {
		printf(">>> 错误：尚未登录，无法进入个人中心。\n");
		return;
	}
	loadDoctorSystemData(sys);   // 从文件加载数据

	// 查找当前登录的医生节点

	doctor* currentDoctor = findDoctorByIdInQueue(sys, doctorId);

	if (currentDoctor == NULL) {
		printf(">>> 错误：未找到当前登录医生的信息。\n");
		return;
	}

	int choice = -1;
	while (1) {
		printf("\n========== 医生个人中心 ==========\n");
		printf("1. 查看个人信息\n");
		printf("2. 修改登录密码\n");
		printf("0. 返回上一级菜单\n");
		printf("==================================\n");
		choice = safeGetInt("请选择操作: ");
		switch (choice) {
		case 1:
			printDoctorInfo(currentDoctor);
			break;
		case 2: {
			char oldPwd[STR_LEN];
			char newPwd[STR_LEN];
			char confirmPwd[STR_LEN];

			safeGetString(">>> 请输入当前密码(输入 -1 取消): ", oldPwd, STR_LEN);
			if (strcmp(oldPwd, "-1") == 0) {
				printf(">>> 已取消修改密码。\n");
				break;
			}
			if (strcmp(oldPwd, currentDoctor->password) != 0) {
				printf(">>> 密码错误，修改失败！\n");
				break;
			}

			safeGetString(">>> 请输入新密码(输入 -1 取消): ", newPwd, STR_LEN);
			if (strcmp(newPwd, "-1") == 0) {
				printf(">>> 已取消修改密码。\n");
				break;
			}

			safeGetString(">>> 请再次输入新密码确认(输入 -1 取消): ", confirmPwd, STR_LEN);
			if (strcmp(confirmPwd, "-1") == 0) {
				printf(">>> 已取消修改密码。\n");
				break;
			}

			if (strcmp(newPwd, confirmPwd) != 0) {
				printf(">>> 两次新密码输入不一致，修改失败！\n");
				break;
			}

			strcpy(currentDoctor->password, newPwd);
			saveDoctorSystemData(sys);
			printf(">>> 密码修改成功！\n");
			break;
		}
		case 0:
			return;
		default:
			printf(">>> 无效选择，请重试。\n");
			break;
		}
	}
}

void doctorManageMenuPat(HIS_System* sys, const char* currentPatientId) {
	if(sys == NULL) {
		printf(">>> 严重错误: 系统底座未初始化！！！\n");
		return;
	}
	if(!isPatientLoggedIn()) {
		printf(">>> 您尚未登录，请先登录后再进行相关操作！\n");
		return;
	}

	loadDoctorSystemData(sys);   // 从文件加载数据

	int choice = -1;
	while(1) {
		printf("\n========== 医生信息公示栏 ==========\n");
		printf("1. 查询相关医生信息\n");
		printf("2. 查询全部医生\n");
		printf("3. 展示所有医生信息\n");
		printf("0. 返回上一级菜单\n");
		printf("==================================\n");
		choice = safeGetInt("请选择查询方式: ");
		char queryStr[STR_LEN];
		char queryDept[STR_LEN];
		char queryRoom[ID_LEN];
		switch (choice) {
		case 1:
			{
				Patient* pat = findPatientById(sys, currentPatientId);
				if (pat == NULL || pat->viewHead == NULL) {
					printf(">>> 您当前没有就诊医生记录，请先挂号或就诊。\n");
					break;
				}
				strcpy(queryStr, pat->viewHead->doctorId);
				queryDoctor(sys, queryStr);
			}
			break;
		case 2:
			queryDoctor(sys, NULL);
			break;
		case 3:
			displayAllDoctors(sys);
			break;
		case 0:
			return;
		default:
			printf(">>> 无效选择，请重试！\n");
			break;
		}
	}
}

// 医生端：查看患者住院信息（当前叫号患者 / 手动指定）
void doctorViewStayInfo(HIS_System* sys, const char* doctorId) {
	if (sys == NULL || doctorId == NULL) {
		printf(">>> 医生未登录，无法查看住院信息。\n");
		return;
	}

	loadWardSystemData(sys);
	loadPatientsSystemData(sys);

	if (sys->wardHead == NULL) {
		printf("\n>>> 系统内暂无病房数据。\n");
		pressEnterToContinue();
		return;
	}

	// 优先展示当前叫号患者
	const char* calledId = findCalledPatientIdByDoctor(doctorId);
	if (calledId != NULL) {
		Patient* p = findPatientById(sys, calledId);
		if (p != NULL) {
			printf("\n>>> 当前叫号患者: %s (%s)\n", p->name, calledId);
			if (confirmFunc("查看", "当前叫号患者住院信息")) {
				docterViewPatientStayInfo(sys, calledId);
				return;
			}
		}
	}

	// 手动输入患者编号
	char pid[ID_LEN];
	safeGetString("请输入患者编号 (输入 -1 取消): ", pid, ID_LEN);
	if (strcmp(pid, "-1") == 0) {
		printf(">>> 已取消查看。\n");
		return;
	}
	if(!isPatientCalledByDoctor(pid, doctorId)) {
		printf(">>> 该患者不是您的患者或者未在候诊队列中，您无法查看住院信息。\n");
		return;
	}
	Patient* p = findPatientById(sys, pid);
	if (p == NULL) {
		printf(">>> 未找到患者 %s 的信息。\n", pid);
		return;
	}
	docterViewPatientStayInfo(sys, pid);
}

// 医生端：安排患者住院（分配病房和床位）
void doctorArrangeWard(HIS_System* sys, const char* doctorId) {
	if (sys == NULL || doctorId == NULL) {
		printf(">>> 医生未登录，无法进行病房分配。\n");
		return;
	}

	loadWardSystemData(sys);
	loadPatientsSystemData(sys);

	if (sys->wardHead == NULL) {
		printf("\n>>> 系统内暂无病房数据，无法分配。\n");
		pressEnterToContinue();
		return;
	}

	printf("\n===== 安排患者住院 =====\n");

	// 输入患者编号
	char patientId[ID_LEN];
	const char* calledId = findCalledPatientIdByDoctor(doctorId);
	Patient* patient = NULL;

	bool useCalledPatient = false;

	// 优先展示当前叫号患者，医生可选择直接使用当前叫号患者信息进行分配，避免重复输入和查询
	if (calledId != NULL) {
		Patient* p = findPatientById(sys, calledId);
		if (p != NULL) {
			printf("\n>>> 当前叫号患者: %s (%s)\n", p->name, calledId);
			if (confirmFunc("分配病房", "给当前叫号患者")) {
				strcpy(patientId, calledId);	// 直接使用当前叫号患者编号，无需再输入
				patient = p;					// 直接使用当前叫号患者信息，无需再查询
				markTicketAsInRoom(calledId, doctorId);		// 将当前叫号患者的挂号单状态标记为已进入诊室
				useCalledPatient = true;
			}
		}
	}

	if(!useCalledPatient) {
		safeGetString(">>> 请输入患者编号 (输入 -1 取消): ", patientId, ID_LEN);
		if (strcmp(patientId, "-1") == 0) {
			printf(">>> 已取消病房分配。\n");
			pressEnterToContinue();
			return;
		}

		if(!isPatientCalledByDoctor(patientId, doctorId)) {
			printf(">>> 该患者不是您的患者或者未在候诊队列中，您无法分配病房。\n");
			pressEnterToContinue();
			return;
		}

		patient = findPatientById(sys, patientId);
		if (patient == NULL) {
			printf(">>> 未找到该患者，请检查编号。\n");
			pressEnterToContinue();
			return;
		}
	}

	// 检查是否已住院
	Ward* existingWard = findPatientWard(sys, patientId);
	if (existingWard != NULL) {
		printf(">>> 该患者已在病房 [%s] 住院中，无需重复分配。\n", existingWard->wardId);
		pressEnterToContinue();
		return;
	}

	// 显示患者信息
	printf("\n患者编号: %s\n", patient->patientId);
	printf("患者姓名: %s\n", patient->name);
	printf("患者类型: %s\n",
		patient->type == PATIENT_VIP ? "VIP" :
		patient->type == PATIENT_EMERGENCY ? "急诊" : "普通");

	// 获取医生科室（用于自动推荐）
	doctor* doc = findDoctorByIdInQueue(sys, doctorId);
	const char* docDept = (doc != NULL) ? doc->department : "";

	// TODO: [权限接口] 在此处检查医生职位是否具有分配病房权限
	// if (!hasWardAssignPermission(doc)) { printf(">>> 权限不足。\n"); return; }

	// 自动推荐
	bool useRecommend = false;
	Ward* recommended = NULL;
	if (confirmFunc("自动推荐", "使用系统自动推荐最优病房")) {
		recommended = autoRecommendWard(sys, patient->type, docDept);
		if (recommended != NULL) {
			printf("\n>>> 系统推荐病房:\n");
			printf("    病房编号: %s  科室: %s  种类: %s  价格: %.2f 元/日\n",
				recommended->wardId, recommended->department,
				wardTypeToStr(recommended->type), recommended->price);
			printf("    已入住: %d/%d  剩余床位: %d\n",
				countOccupiedBeds(recommended),
				countBeds(recommended),
				countBeds(recommended) - countOccupiedBeds(recommended));

			if (confirmFunc("采纳推荐", "使用系统推荐的病房")) {
				useRecommend = true;
			}
		}
		else {
			printf(">>> 系统未能推荐出合适的病房，请手动选择。\n");
		}
	}

	// 医生选择病房（强制指定需二次确认）
	char wardId[ID_LEN];
	Ward* targetWard = NULL;

	if (useRecommend) {
		strcpy(wardId, recommended->wardId);
		targetWard = recommended;
	}
	else {
		// 展示所有有空床的病房供医生选择
		printf("\n--- 可选病房（有空床） ---\n");
		Ward* w = sys->wardHead;
		int wCnt = 0;
		while (w != NULL) {
			int freeBeds = countBeds(w) - countOccupiedBeds(w);
			if (freeBeds > 0) {
				printf("  [%d] 病房:%s  科室:%s  种类:%s  价格:%.0f元  剩余:%d床\n",
					++wCnt, w->wardId, w->department,
					wardTypeToStr(w->type), w->price, freeBeds);
			}
			w = w->next;
		}
		if (wCnt == 0) {
			printf(">>> 所有病房已满，无法分配。\n");
			pressEnterToContinue();
			return;
		}

		safeGetString(">>> 请输入目标病房编号 (输入 -1 取消): ", wardId, ID_LEN);
		if (strcmp(wardId, "-1") == 0) {
			printf(">>> 已取消病房分配。\n");
			pressEnterToContinue();
			return;
		}

		// 查找病房
		targetWard = sys->wardHead;
		while (targetWard != NULL && strcmp(targetWard->wardId, wardId) != 0)
			targetWard = targetWard->next;
		if (targetWard == NULL) {
			printf(">>> 未找到该病房编号。\n");
			pressEnterToContinue();
			return;
		}

		// 非推荐病房需二次确认（强制指定）
		if (!confirmFunc("强制指定", "非系统推荐的病房")) {
			printf(">>> 已取消病房分配。\n");
			pressEnterToContinue();
			return;
		}
		char confirmMsg[256];
		sprintf(confirmMsg, "指定病房 [%s] 给患者 [%s]",
			targetWard->wardId, patient->name);
		if (!confirmFunc("再度确认", confirmMsg)) {
			printf(">>> 已取消病房分配。\n");
			pressEnterToContinue();
			return;
		}
	}

	// 选择空闲床位
	printf("\n--- 病房 [%s] 空闲床位 ---\n", targetWard->wardId);
	Bed* bed = targetWard->bedListHead;
	int bCnt = 0;
	while (bed != NULL) {
		if (!bed->isOccupied) {
			printf("  [%d] 床位编号:%s\n", ++bCnt, bed->bedId);
		}
		bed = bed->next;
	}
	if (bCnt == 0) {
		printf(">>> 该病房已无空床位。\n");
		pressEnterToContinue();
		return;
	}

	char bedInput[BED_ID_LEN];
	safeGetString(">>> 请选择床位（输入序号或编号，-1 取消）: ", bedInput, BED_ID_LEN);
	if (strcmp(bedInput, "-1") == 0) {
		printf(">>> 已取消病房分配。\n");
		pressEnterToContinue();
		return;
	}

	Bed* targetBed = NULL;

	// 纯数字 → 按序号查找（第 N 个空闲床位）
	if (isAllDigits(bedInput)) {
		int idx = atoi(bedInput);
		if (idx >= 1 && idx <= bCnt) {
			Bed* b = targetWard->bedListHead;
			int n = 0;
			while (b != NULL) {
				if (!b->isOccupied) {
					n++;
					if (n == idx) { targetBed = b; break; }
				}
				b = b->next;
			}
		} else {
			printf(">>> 床位序号超出范围（1-%d），请重试。\n", bCnt);
			pressEnterToContinue();
			return;
		}
	} else {
		// 非纯数字 → 按床位编号查找
		targetBed = findBed(targetWard, bedInput);
		if (targetBed == NULL) {
			printf(">>> 未找到床位编号 [%s]，请重试。\n", bedInput);
			pressEnterToContinue();
			return;
		}
	}

	if (targetBed == NULL) {
		printf(">>> 未找到匹配的床位。\n");
		pressEnterToContinue();
		return;
	}
	if (targetBed->isOccupied) {
		printf(">>> 该床位已被占用。\n");
		pressEnterToContinue();
		return;
	}

	// 最终确认
	printf("\n>>> 住院分配确认:\n");
	printf("    患者: %s (%s)\n", patient->name, patient->patientId);
	printf("    病房: %s (%s, %s)\n",
		targetWard->wardId, targetWard->department, wardTypeToStr(targetWard->type));
	printf("    床位: %s\n", targetBed->bedId);

	if (!confirmFunc("住院分配", "以上住院分配信息")) {
		printf(">>> 已取消病房分配。\n");
		pressEnterToContinue();
		return;
	}

	// 预扣 7 天押金
	double deposit = 7.0 * targetWard->price;
	printf(">>> 住院押金: 7天 × %.2f元/天 = %.2f 元\n", targetWard->price, deposit);
	printf(">>> 当前余额: %.2f 元，扣费后余额: %.2f 元\n",
		patient->balance, patient->balance - deposit);
	patient->balance -= deposit;
	addHospitalRevenue(sys, deposit);

	// 执行分配
	strcpy(targetBed->patient, patientId);
	targetBed->isOccupied = true;
	saveWardSystemData(sys);
	printf(">>> 住院分配成功！患者 [%s] 已入住病房 [%s] 床位 [%s]。\n",
		patient->name, targetWard->wardId, targetBed->bedId);

	// 同步住院记录到患者数据
	char deptInfo[STR_LEN];
	sprintf(deptInfo, "科室[%s]", targetWard->department);
	appendStayMedicalRecord(sys, patientId, doctorId, deptInfo, targetBed->bedId,
		getCurrentDateStr(), "待定", "未出院", targetWard->wardId);

	savePatientsSystemData(sys);

	// 推进叫号状态为就诊中
	markTicketAsInRoom(patientId, doctorId);
	pressEnterToContinue();
}

// 医生端：批准办理患者出院（合并原"批准出院"与"办理出院"）
void doctorApproveDischarge(HIS_System* sys, const char* doctorId) {
	if (sys == NULL || doctorId == NULL) {
		printf(">>> 医生未登录，无法办理。\n");
		return;
	}

	loadWardSystemData(sys);
	loadPatientsSystemData(sys);

	char patientId[ID_LEN];
	bool useCalledPatient = false;
	const char* calledId = findCalledPatientIdByDoctor(doctorId);
	if (calledId != NULL) {
		Patient* p = findPatientById(sys, calledId);
		if (p != NULL) {
			printf("\n>>> 当前叫号患者: %s (%s)\n", p->name, calledId);
			if (confirmFunc("选择", "为叫号患者办理出院")) {
				strcpy(patientId, calledId);
				useCalledPatient = true;
			}
		}
	}

	if (!useCalledPatient) {
		safeGetString(">>> 请输入要批准出院的患者编号 (输入 -1 取消): ", patientId, ID_LEN);
		if (strcmp(patientId, "-1") == 0) {
			printf(">>> 已取消。\n");
			return;
		}
		if (!isPatientCalledByDoctor(patientId, doctorId)
			&& !hasPatientCalledByDoctor(patientId, doctorId)) {
			printf(">>> 该患者不是您的患者或者未在候诊队列中，您无法批准出院。\n");
			return;
		}
	}

	Patient* patient = findPatientById(sys, patientId);
	if (patient == NULL) {
		printf(">>> 未找到患者信息。\n");
		pressEnterToContinue();
		return;
	}

	Ward* currentWard = findPatientWard(sys, patientId);
	if (currentWard == NULL) {
		printf(">>> 患者 %s (%s) 当前未住院，无法批准出院。\n", patient->name, patientId);
		pressEnterToContinue();
		return;
	}

	StayRecord* activeStay = NULL;
	StayRecord* s = patient->stayHead;
	while (s != NULL) {
		if (strcmp(s->wardId, currentWard->wardId) == 0 && (strcmp(s->endDate, "未出院") == 0 || strcmp(s->endDate, "待出院") == 0)) {
			activeStay = s;
			break;
		}
		s = s->next;
	}

	if (activeStay == NULL) {
		printf(">>> 未找到患者住院记录，无法批准出院。\n");
		pressEnterToContinue();
		return;
	}
	if (activeStay->dischargeApproved == 1) {
		printf(">>> 该患者已获出院许可，无需重复批准。\n");
		pressEnterToContinue();
		return;
	}

	// 设置出院日期
	char endDate[DATE_STR_LEN];
	if (TEST_SYSTEM_DEBUG) {
		safeGetString(">>> 请输入出院日期(YYYY-MM-DD，输入 -1 取消): ", endDate, DATE_STR_LEN);
		if (strcmp(endDate, "-1") == 0) {
			printf(">>> 已取消。\n");
			return;
		}
		if (!isValidDate(endDate)) {
			printf(">>> 日期格式无效，操作取消。\n");
			return;
		}
		if (!confirmFunc("确认", "出院日期")) {
			printf(">>> 已取消。\n");
			return;
		}
	} else {
		strcpy(endDate, getCurrentDateStr());
	}

	// 校验出院日期不得早于入院日期
	if (strcmp(endDate, activeStay->startDate) < 0) {
		printf(">>> 日期无效，不得早于入院日期 (%s)。\n", activeStay->startDate);
		pressEnterToContinue();
		return;
	}

	if (!confirmFunc("批准", "患者出院")) {
		printf(">>> 已取消批准出院。\n");
		pressEnterToContinue();
		return;
	}

	// 设置出院许可并写入出院日期
	activeStay->dischargeApproved = 1;
	strncpy(activeStay->endDate, endDate, ID_LEN - 1);
	activeStay->endDate[ID_LEN - 1] = '\0';
	// 更新住院时长
	int days = daysBetweenDates(activeStay->startDate, endDate);
	char duration[ID_LEN];
	sprintf(duration, "%d天", days);
	strncpy(activeStay->duration, duration, ID_LEN - 1);
	activeStay->duration[ID_LEN - 1] = '\0';

	savePatientsSystemData(sys);
	printf(">>> 患者 %s 出院许可已批准，出院日期: %s。\n", patient->name, endDate);
	pressEnterToContinue();
}

// 医生端：住院管理二级菜单
void doctorWardMenu(HIS_System* sys, const char* doctorId) {
	int choice;
	while (1) {
		printf("\n======== 住院管理 ========\n");
		printf("1. 分配住院\n");
		printf("2. 查看住院信息\n");
		printf("3. 批准办理患者出院\n");
		printf("0. 返回上级菜单\n");
		printf("==========================\n");
		choice = safeGetInt("请选择操作: ");
		switch (choice) {
		case 1: doctorArrangeWard(sys, doctorId); break;
		case 2: doctorViewStayInfo(sys, doctorId); break;
		case 3: doctorApproveDischarge(sys, doctorId); break;
		case 0: return;
		default: printf(">>> 无效选择，请重试。\n");
		}
	}
}
