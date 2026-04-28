#define _CRT_SECURE_NO_WARNINGS
#include"HIS_System.h"
#include"WardManage.h"
#include"WardFileManage.h"
#include"WardSort.h"
#include"ConfirmFunc.h"
#include"InputUtils.h"
#include"PauseUtil.h"
#include"DepartmentManage.h"
#include<string.h>

//检查病房编号是否存在
bool isWardIdExist(Ward* head, const char* id) {
	Ward* curr = head;
	while (curr != NULL) {
		if (strcmp(curr->wardId, id) == 0) return true;
		curr = curr->next;
	}
	return false;
}

//检查病床编号是否存在
bool isBedIdExist(Bed* head, const char* id) {
	Bed* curr = head;
	while (curr != NULL) {
		if (strcmp(curr->bedId, id) == 0) return true;
		curr = curr->next;
	}
	return false;
}

static const char* wardTypeToStr(WardType type) {
	switch (type) {
	case WARD_NORMAL: return "普通病房";
	case WARD_VIP: return "VIP病房";
	case WARD_ICU: return "ICU病房";
	default: return "未知";
	}
}

//统计床位数
static int countBeds(Ward* ward) {
	int c = 0; Bed* b = ward->bedListHead; while (b) { c++; b = b->next; } return c;
}
//统计已占用床位数
static int countOccupiedBeds(Ward* ward) {
	int c = 0; Bed* b = ward->bedListHead; while (b) { if (b->isOccupied) c++; b = b->next; } return c;
}

//按床位编号查找
static Bed* findBed(Ward* ward, const char* bedId) {
	Bed* b = ward->bedListHead; while (b) { if (strcmp(b->bedId, bedId) == 0) return b; b = b->next; } return NULL;
}

//录入新病房
void addWard(HIS_System* sys) {
	while (1) {
		printf("\n--- 录入新病房 (在任意输入环节输入 '-1' 可取消本次修改) ---\n");
		Ward* newWard = (Ward*)malloc(sizeof(Ward));
		if (!newWard) { 
			printf(">>> 内存分配失败！\n");
			return; 
		}
		bool cancel = false;

		while (1) {
			safeGetString("请输入病房编号: ", newWard->wardId, ID_LEN);
			if (strcmp(newWard->wardId, "-1") == 0) {
				cancel = true;
				break; 
			}
			if (isWardIdExist(sys->wardHead, newWard->wardId)) {
				printf(">>> 病房编号已存在！\n");
				continue; 
			}
			break;
		}
		if (cancel) {
			free(newWard);
			printf(">>> 已取消录入。\n");
			return; 
		}

		while (1) {
			int t = safeGetInt("请选择病房种类 (1普通 2VIP 3ICU): ");
			if (t == -1) { cancel = true; break; }
			if (t < 1 || t > 3) { printf(">>> 无效种类！\n"); continue; }
			newWard->type = (WardType)t;
			break;
		}
		if (cancel) {
			free(newWard);
			printf(">>> 已取消录入。\n");
			return; 
		}

		while (1) {
			safeGetString("请输入所属科室: ", newWard->department, STR_LEN);
			if (strcmp(newWard->department, "-1") == 0) {
				cancel = true;
				break; 
			}
			if (!isDepartmentNameExist(sys->deptHead, newWard->department)) {
				printf(">>> 提示：系统内未找到该科室，请确认是否已在科室模块创建。\n");
			}
			break;
		}
		if (cancel) {
			free(newWard);
			printf(">>> 已取消录入。\n");
			return; 
		}

		newWard->bedListHead = NULL;
		int bedCount = 0;
		while (1) {
			bedCount = safeGetInt("请输入要录入的床位数量: ");
			if (bedCount == -1) {
				cancel = true;
				break; 
			}
			if (bedCount <= 0) {
				printf(">>> 床位数量需大于0！\n");
				continue; 
			}
			break;
		}
		if (cancel) { 
			free(newWard);
			printf(">>> 已取消录入。\n");
			return;
		}

		for (int i = 0; i < bedCount; ++i) {
			Bed* b = (Bed*)malloc(sizeof(Bed));
			if (!b) {
				printf(">>> 床位内存分配失败，已中断。\n");
				cancel = true;
				break; 
			}
			while (1) {
				char prompt[64]; sprintf(prompt, "请输入第%d个床位编号: ", i + 1);
				safeGetString(prompt, b->bedId, ID_LEN);
				if (strcmp(b->bedId, "-1") == 0) {
					cancel = true;
					break; 
				}
				if (isBedIdExist(newWard->bedListHead, b->bedId)) { printf(">>> 床位编号重复！\n"); continue; }
				break;
			}
			if (cancel) {
				free(b);
				break; 
			}
			b->isOccupied = false;
			b->patient[0] = '\0';
			b->next = newWard->bedListHead;
			newWard->bedListHead = b;
		}
		if (cancel) {
			// 清理已创建的床位
			Bed* b = newWard->bedListHead; while (b) { Bed* tmp = b; b = b->next; free(tmp); }
			free(newWard); printf(">>> 已取消录入。\n"); return;
		}

		sortBedList(&newWard->bedListHead);	// 对床位列表排序(取地址)

		newWard->next = sys->wardHead;
		sys->wardHead = newWard;
		printf(">>> 病房 <%s> 录入成功，种类:%s 科室:%s 床位:%d。\n",
			newWard->wardId, wardTypeToStr(newWard->type), newWard->department, bedCount);
		printf(">>> 继续添加请直接输入，下一个编号输入 -1 可退出。\n");
	}
}

//打印单条病房详细信息
void printWardInfo(Ward* ward) {
	if (!ward) return;
	int total = countBeds(ward);
	int used = countOccupiedBeds(ward);
	int freeCnt = total - used;
	printf("\n==================== 病房信息 ====================\n");
	printf("病房编号: %s\n", ward->wardId);
	printf("病房种类: %s\n", wardTypeToStr(ward->type));
	printf("所属科室: %s\n", ward->department);
	printf("床位总数: %d\n", total);
	printf("已占用数: %d\n", used);
	printf("剩余床位: %d\n", freeCnt);
	printf("--- 床位列表 ---\n");
	Bed* b = ward->bedListHead;
	int idx = 1;
	while (b) {
		printf(" #%d 床位编号:%s 状态:%s 患者编号:%s\n", idx++, b->bedId, b->isOccupied ? "已占用" : "空闲", b->isOccupied ? b->patient : "-");
		b = b->next;
	}
	printf("==================================================\n");
}

//查询病房信息
void queryWard(HIS_System* sys) {
	if (sys->wardHead == NULL) {
		printf("\n>>> 系统内没有病房数据！\n");
		return;
	}
	int choice;
	printf("\n--- 病房信息查询 ---\n");
	printf("1. 按病房编号\n");
	printf("2. 按病房种类\n");
	printf("3. 按所属科室\n");
	printf("4. 按床位总数\n");
	printf("5. 按剩余床位数\n");
	printf("6. 按已使用床位数\n");
	printf("7. 按床位编号\n");
	printf("8. 按患者编号\n");
	printf("0. 返回上一级\n");
	choice = safeGetInt("请选择查询方式: ");

	char strBuf[STR_LEN];
	int num;
	Ward* curr = sys->wardHead;
	bool found = false;
	switch (choice) {
	case 1:
		safeGetString("请输入病房编号: ", strBuf, ID_LEN);
		while (curr) {
			if (strcmp(curr->wardId, strBuf) == 0){ 
				printWardInfo(curr);
				found = true; 
				break; 
			} 
			curr = curr->next; 
		}
		break;
	case 2:{
	 int t = safeGetInt("请输入病房种类(1普通 2VIP 3ICU): "); 
		while (curr) {
			if ((int)curr->type == t) {
				printWardInfo(curr);
				found = true;
			} 
			curr = curr->next; 
		}
		break; 
	}
	case 3:
		safeGetString("请输入所属科室: ", strBuf, STR_LEN);
		while (curr) {
			if (strcmp(curr->department, strBuf) == 0) 
				printWardInfo(curr);
			found = true; 
		} 
		curr = curr->next; 
		break;
	case 4:
		num = safeGetInt("请输入床位总数: ");
		while (curr) { 
			if (countBeds(curr) == num) {
				printWardInfo(curr);
				found = true;
			} 
			curr = curr->next; }
		break;
	case 5:
		num = safeGetInt("请输入剩余床位数: ");
		while (curr) {
			if (countBeds(curr) - countOccupiedBeds(curr) == num)
			{ 
				printWardInfo(curr);
				found = true; 
			} 
			curr = curr->next; 
		}
		break;
	case 6:
		num = safeGetInt("请输入已使用床位数: ");
		while (curr) {
			if (countOccupiedBeds(curr) == num) {
				printWardInfo(curr);
				found = true;
			}
			curr = curr->next;
		}
		break;
	case 7:
		safeGetString("请输入床位编号: ", strBuf, ID_LEN);
		while (curr) {
			Bed* b = findBed(curr, strBuf);
			if (b) { 
				printWardInfo(curr);
				found = true; 
			}
			curr = curr->next;
		}
		break;
	case 8:
		safeGetString("请输入患者编号: ", strBuf, ID_LEN);
		while (curr) {
			Bed* b = curr->bedListHead;
			while (b) {
				if (b->isOccupied && strcmp(b->patient, strBuf) == 0) {
					printWardInfo(curr);
					found = true;
					break; 
				}
				b = b->next;
			}
			curr = curr->next;
		}
		break;
	case 0: return;
	default: printf(">>> 无效选择！\n"); 
		return;
	}
	if (!found) printf(">>> 没有找到匹配的病房信息！\n");
}

//修改病房信息
void modifyWard(HIS_System* sys) {
	if (sys->wardHead == NULL) { 
		printf("\n>>> 系统内没有病房数据！\n");
		return; 
	}
	char wardId[ID_LEN];
	while (1) {
		safeGetString("请输入要修改的病房编号 (输入 -1 取消): ", wardId, ID_LEN);
		if (strcmp(wardId, "-1") == 0)  return; 
		Ward* target = sys->wardHead;	
		while (target && strcmp(target->wardId, wardId) != 0) 
			target = target->next;

		if (!target) {
			printf(">>> 未找到该病房！\n");
			continue; 
		}

		printWardInfo(target);
		printf("\n1. 修改病房编号\n");
		printf("2. 修改病房种类\n");
		printf("3. 修改所属科室\n");
		printf("4. 新增床位\n");
		printf("5. 删除床位\n");
		printf("6. 床位占用/释放\n");
		int ch = safeGetInt("请选择要修改的内容 (输入 -1 取消): ");
		if (ch == -1) return;
		if (!confirmFunc("修改", "病房信息")) {
			printf(">>> 已取消修改。\n");
			return; 
		}

		switch (ch) {
		case 1: {
			char newId[ID_LEN];
			while (1) {
				safeGetString("请输入新的病房编号 (输入 -1 取消): ", newId, ID_LEN);
				if (strcmp(newId, "-1") == 0) break;
				if (isWardIdExist(sys->wardHead, newId)) {
					printf(">>> 编号已存在！\n");
					continue; 
				}
				strcpy(target->wardId, newId);
				printf(">>> 病房编号修改成功！\n");
				break;
			}
			break; }
		case 2: {
			while (1) {
				int t = safeGetInt("请选择病房种类 (1普通 2VIP 3ICU，-1取消): ");
				if (t == -1) break;
				if (t < 1 || t > 3) {
					printf(">>> 无效种类！\n");
					continue; 
				}
				target->type = (WardType)t; 
				printf(">>> 病房种类已修改！\n");
				break;
			}
			break; }
		case 3: {
			char newDept[STR_LEN];
			while (1) {
				safeGetString("请输入新的所属科室 (输入 -1 取消): ", newDept, STR_LEN);
				if (strcmp(newDept, "-1") == 0) break;
				strcpy(target->department, newDept);
				printf(">>> 所属科室已修改！\n");
				break;
			}
			break; }
		case 4: {
			Bed* b = (Bed*)malloc(sizeof(Bed));
			if (!b) {
				printf(">>> 内存分配失败！\n");
				break; 
			}
			while (1) {
				safeGetString("请输入新床位编号 (输入 -1 取消): ", b->bedId, ID_LEN);
				if (strcmp(b->bedId, "-1") == 0) {
					free(b);
					b = NULL;
					break; 
				}
				if (isBedIdExist(target->bedListHead, b->bedId)) {
					printf(">>> 编号重复！\n");
					continue; 
				}
				break;
			}
			if (b) {
				b->isOccupied = false; b->patient[0] = '\0';
				b->next = target->bedListHead; target->bedListHead = b;
				printf(">>> 床位添加成功！\n");
			}

			sortBedList(&target->bedListHead);	// 添加后排序

			break; }
		case 5: {
			char delId[ID_LEN];
			safeGetString("请输入要删除的床位编号 (输入 -1 取消): ", delId, ID_LEN);
			if (strcmp(delId, "-1") == 0) break;
			Bed* b = target->bedListHead; Bed* prev = NULL;
			while (b && strcmp(b->bedId, delId) != 0) {
				prev = b; 
				b = b->next; 
			}
			if (!b) {
				printf(">>> 未找到该床位！\n");
				break; 
			}
			if (prev)
				prev->next = b->next;
			else 
				target->bedListHead = b->next;

			free(b); 
			printf(">>> 床位删除成功！\n");

			break; }
		case 6: {
			char bedId[ID_LEN];
			safeGetString("请输入床位编号 (输入 -1 取消): ", bedId, ID_LEN);
			if (strcmp(bedId, "-1") == 0) break;

			Bed* b = findBed(target, bedId);

			if (!b) {
				printf(">>> 未找到该床位！\n"); 
				break; 
			}

			printf("当前状态:%s\n", b->isOccupied ? "已占用" : "空闲");

			int op = safeGetInt("1-设为空闲 2-设为占用 (-1取消): ");
			if (op == -1) break;

			if (op == 1) {
				b->isOccupied = false;
				b->patient[0] = '\0';
				printf(">>> 已释放！\n"); 
			}
			else if (op == 2) {
				safeGetString("请输入患者编号 (输入 -1 取消): ", b->patient, ID_LEN);
				if (strcmp(b->patient, "-1") == 0) break;
				b->isOccupied = true;
				printf(">>> 已设置占用！\n"); 
			}
			else 
				printf(">>> 无效操作！\n");
			break; }
		default:
			printf(">>> 无效选择。\n");
		}
		break; // 成功定位病房后退出外层 while(1)
	}
}

//删除病房记录
void deleteWard(HIS_System** sys) {
	while (1) {
		if ((*sys)->wardHead == NULL) {
			printf("\n>>> 系统内没有病房数据！\n");
			return;
		}
		printf("\n--- 删除病房 ---\n");
		printf("1. 按病房编号删除\n");
		printf("2. 按所属科室删除(将删除该科室下所有病房)\n");
		printf("0. 返回\n");
		int ch = safeGetInt("请选择: ");
		if (ch == 0) return;
		if (ch != 1 && ch != 2) {
			printf(">>> 无效选择！\n");
			continue; 
		}
		char q[STR_LEN];
		switch (ch) {
		case 1:
			safeGetString("请输入病房编号: ", q, ID_LEN);
			deleteWardFunc(&((*sys)->wardHead), q, 1);
			return;
		case 2:
			safeGetString("请输入所属科室: ", q, STR_LEN);
			deleteWardFunc(&((*sys)->wardHead), q, 2);
			return;
		}
	}
}

//根据删除方式和查询字符串的删除工具
void deleteWardFunc(Ward** head, const char* queryStr, int mode) {
	Ward* curr = *head; Ward* prev = NULL; bool deleted = false;
	if (!confirmFunc("删除", "病房")) {
		printf(">>> 已取消删除。\n");
		return; 
	}
	while (curr) {
		bool match = false;
		switch (mode) {
		case 1: match = (strcmp(curr->wardId, queryStr) == 0); break;
		case 2: match = (strcmp(curr->department, queryStr) == 0); break;
		default: break;
		}
		if (match) {
			// delete
			Bed* b = curr->bedListHead; while (b) {
				Bed* tmp = b;
				b = b->next;
				free(tmp); 
			}
			if (prev) prev->next = curr->next; else *head = curr->next;
			printf(">>> 病房 <%s> 已删除。\n", curr->wardId);

			Ward* tmpW = curr;
			curr = curr->next;
			free(tmpW);
			deleted = true;

			if (mode == 1) 
				break; // 单条
			else 
				continue;
		}
		prev = curr;
		curr = curr->next;
	}
	if (!deleted) 
		printf(">>> 未找到匹配的病房。\n");
}

//显示所有病房信息
void displayAllWards(HIS_System* sys) {
	if (sys->wardHead == NULL) { printf("\n>>> 系统内没有病房数据！\n"); return; }
	Ward* curr = sys->wardHead; int count = 0;
	while (curr) { printf("\n--- 病房 #%d ---\n", ++count); printWardInfo(curr); curr = curr->next; }
	printf(">>> 共计 %d 个病房。\n", count);
	pressEnterToContinue();
}

//病房管理菜单界面
void wardManageMenu(HIS_System* sys) {
	if (sys == NULL) {
		printf(">>> 严重错误: 系统底座未初始化！！！\n");
		return;
	}

	if (sys->deptHead == NULL) {
		if (TEST_SYSTEM_DEBUG) {
			printf(">>> 提示：当前内存中尚无科室数据，病房模块需要绑定科室信息。\n");
			printf("没有同步科室数据，病房模块将无法正常使用，建议载入科室数据！\n");
		}
		// 同步科室数据到病房模块
		// 在测试模式下直接提示用户是否同步，在正式模式下强制同步（因为没有科室数据病房模块无法使用）
		if (!TEST_SYSTEM_DEBUG || adminConfirmFunc("同步", "科室数据到病房模块")) {
			loadDepartmentSystemData(sys);
		}
		else {
			return;
		}
	}
	loadWardSystemData(sys);

	int choice;
	while (1) {
		printf("\n========== 病房管理系统 ==========\n");
		printf("1. 录入新病房\n");
		printf("2. 查询病房\n");
		printf("3. 修改病房\n");
		printf("4. 排序病房\n");
		printf("5. 删除病房\n");
		printf("6. 显示所有病房\n");
		printf("7. 保存病房数据\n");
		printf("0. 返回上一级菜单\n");
		printf("=================================\n");
		choice = safeGetInt("请选择操作: ");
		switch (choice) {
		case 1: addWard(sys); break;
		case 2: queryWard(sys); break;
		case 3: modifyWard(sys); break;
		case 4: wardSortMenu(sys); break;
		case 5: deleteWard(&sys); break;
		case 6: displayAllWards(sys); break;
		case 7: saveWardSystemData(sys); break;
		case 0: return;
		default: printf(">>> 无效选择，请重试。\n"); break;
		}
	}
}
