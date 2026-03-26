#define _CRT_SECURE_NO_WARNINGS
#include"HIS_System.h"
#include"DocterFileManage.h"
#include"DocterManage.h"
#include"ConfirmFunc.h"
#include"DocterSort.h"
#include"InputUtils.h"

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

void addDoctor(HIS_System* sys) {
	while (1) {
		printf("\n--- 录入新医生 (在任意输入环节输入 '-1' 可取消本次添加) ---\n");
		Docter* newDoctor = (Docter*)malloc(sizeof(Docter));
		if (newDoctor == NULL) {
			printf(">>> 内存分配失败！\n");
			return;
		}
		bool hasCancelFlag = false;

		// 输入并验证医生编号
			safeGetString("请输入医生编号: ", newDoctor->docterId, ID_LEN);
			if (strcmp(newDoctor->docterId, "-1") == 0) {
				hasCancelFlag = true;
				break;
			}
			if (isDoctorIdExist(sys->docHead, newDoctor->docterId)) {
				printf(">>> 医生编号已存在，请重新输入！\n");
				continue;
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
				//医生姓名不需要强制唯一，允许同名医生存在
				/*if (isDoctorNameExist(sys->docHead, newDoctor->docterName)) {
					printf(">>> 医生姓名已存在，请重新输入！\n");
					continue;
				}*/
				break;
			}
			if(hasCancelFlag) {
				free(newDoctor);
				printf(">>> 已取消添加医生。\n");
				break;
			}

			// 输入医生所在科室
			while (1) {
				safeGetString("请输入医生所在科室: ", newDoctor->department, STR_LEN);
				//TODO: 增加科室合法性验证，例如预设一个科室列表，要求输入必须在列表中
				if (strcmp(newDoctor->department, "-1") == 0) {
					hasCancelFlag = true;
					break;
				}
				break;
			}
			if (hasCancelFlag) {
				free(newDoctor);
				printf(">>> 已取消添加医生。\n");
				break;
			}
			// 新医生默认诊号数量为0
			newDoctor->consultationCount = 0;

		// 头插法
		newDoctor->next = sys->docHead;
		sys->docHead = newDoctor;

		printf(">>> 医生 <%s> (Id: %s) 录入成功！\n", newDoctor->docterName, newDoctor->docterId);
		
		// 询问是否继续添加
		printf(">>> 提示：已自动继续添加医生；可直接输入 '-1' 可以在下一个编号输入时退出。\n");
	}
}

void printDoctorInfo(Docter* doctor) {
	printf("\n========== 医生信息 ==========\n");
	printf("医生编号: %s\n", doctor->docterId);
	printf("医生姓名: %s\n", doctor->docterName);
	printf("所在科室: %s\n", doctor->department);
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
	printf("3. 按所在科室查询\n");
	printf("0. 返回上一级菜单\n");
	choice = safeGetInt("请选择查询方式: ");
	char queryStr[STR_LEN];
	Docter* curr = sys->docHead;
	bool found = false;
	switch (choice) {
	case 1:
		safeGetString("请输入医生编号: ", queryStr, ID_LEN);
		while (curr != NULL) {
			if (strcmp(curr->docterId, queryStr) == 0) {
				printDoctorInfo(curr);
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
				found = true;
				break;
			}
			curr = curr->next;
		}
		break;
	case 3:
		safeGetString("请输入医生所在科室: ", queryStr, STR_LEN);
		int personCount = 0;
		while (curr != NULL) {
			if (strcmp(curr->department, queryStr) == 0) {
				printf("\n%s号科室第%d位医生:\n", queryStr, ++personCount);
				printDoctorInfo(curr);
				found = true;
			}
			curr = curr->next;
		}
		break;
	case 0:
		return; // 返回上一级菜单
	default:
		printf(">>> 无效选择，请重试！\n");
		break;

		if(!found) {
			printf(">>> 没有找到匹配的医生信息！\n");
		}
	}
}

void modifyDoctor(HIS_System* sys) {
	if(sys->docHead == NULL) {
		printf("\n>>> 系统内没有医生数据！\n");
		return;
	}
	//TODO: 修改医生数据的实现需要先查询到目标医生节点，然后根据用户选择修改对应字段，涉及较多交互细节，暂时以提示代替具体实现
	printf(">>> 模块待开发: 修改医生数据功能...\n");
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
		printf("3. 按所在科室删除\n");
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
			safeGetString("请输入要删除的医生所在科室: ", queryStr, STR_LEN);
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
	int personCount = 0; // 用于按科室删除时记录匹配的医生数量
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
				// 按所在科室删除时，先列出所有匹配的医生节点，最后询问用户删除目标，避免误删
				printf("医生界面只支持单次删除一位医生，若要一键删除整个科室的医生，请前往科室管理模块\n");
				printf("请根据以下列表确认要删除的医生信息:\n");
				if (strcmp(curr->department, queryStr) == 0) {
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
							printf(">>> 已跳过该医生，继续查找%s科室下一个匹配的医生...\n", queryStr);
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

void doctorManageMenu(HIS_System* sys) {
	if (sys == NULL) {
		printf(">>> 严重错误: 系统底座未初始化！！！\n");
		return;
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
		printf("7. 保存系统数据\n");
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


