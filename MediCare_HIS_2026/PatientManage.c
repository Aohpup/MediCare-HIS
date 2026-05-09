#define _CRT_SECURE_NO_WARNINGS
#include"HIS_System.h"
#include"DrugFileManage.h"
#include"DrugSort.h"
#include"DrugManage.h"
#include"doctorManage.h"
#include"doctorFileManage.h"
#include"DepartmentFileManage.h"
#include"DepartmentManage.h"
#include"WardFileManage.h"
#include"WardManage.h"
#include"PatientManage.h"
#include"PatientFileManage.h"
#include"ExamManage.h"
#include"QueueManage.h"
#include"PrintFormattedStr.h"
#include"PauseUtil.h"
#include"InputUtils.h"
#include"ConfirmFunc.h"
#include"DayTimeUtils.h"
#include"string.h"

bool is_Patient_Logged_In = false;	//标记患者是否已登录

static Patient* currentPatient = NULL;

static Patient* getCurrentPatientNode(void) {
	return currentPatient;
}

bool isPatientLoggedIn(void) {
/*	if (!is_Patient_Logged_In) {
		printf(">>> 您尚未登录，请先登录后再进行相关操作！\n");
	}*/
	return is_Patient_Logged_In;
}

const char* getCurrentPatientId(void) {
	if (currentPatient != NULL) {
		return currentPatient->patientId;
	}
	return NULL;
}

// 根据患者编号查找患者节点
Patient* findPatientById(HIS_System* sys, const char* patientId) {
	if (sys == NULL || patientId == NULL) {
		return NULL;
	}
	Patient* curr = sys->patientHead;
	while (curr != NULL) {
		if (strcmp(curr->patientId, patientId) == 0) {
			return curr;
		}
		curr = curr->next;
	}
	return NULL;
}

// 生成病例编号，格式为 [prefix]+患者编号后4位+3位随机数
static void generateRecordId(char* outId, const char* prefix, const char* patientId) {
	if (outId == NULL || prefix == NULL || patientId == NULL) {
		return;
	}
	int len = (int)strlen(patientId);
	const char* suffix = patientId;
	if (len > 4) {
		suffix = patientId + (len - 4);
	}
	sprintf(outId, "%s%s%03d", prefix, suffix, rand() % 1000);
}

// 打印单条看诊/检查病例
static void printConsultationRecord(const ConsultationRecord* rec) {
	if (rec == NULL) {
		return;
	}
	printf("记录编号: %s\n", rec->recordId);
	printf("记录日期: %s\n", rec->date);
	printf("医生编号: %s\n", rec->doctorId);
	printf("记录内容: %s\n", rec->details);
}

// 打印单条住院病例
static void printStayRecord(const StayRecord* rec) {
	if (rec == NULL) {
		return;
	}
	bool discharged = (strcmp(rec->endDate, "未出院") != 0 && strcmp(rec->endDate, "待出院") != 0);
	printf("记录编号: %s\n", rec->recordId);
	printf("开始日期: %s\n", rec->startDate);
	printf("住院时长: %s\n", rec->duration);
	printf("结束日期: %s\n", rec->endDate);
	printf("住院状态: %s\n", discharged ? "已出院" : "住院中");
	printf("科室描述: %s\n", rec->deptInfo);
	printf("医生编号: %s\n", rec->doctorId);
	printf("病房编号: %s\n", rec->wardId);
	printf("床位编号: %s\n", rec->bedId);
	printf("事件描述: %s\n", rec->details);
}

// 设置当前登录的患者信息指针，并根据指针是否为NULL更新登录状态标志
static void setCurrentPatient(Patient* patient) {
	currentPatient = patient;
	is_Patient_Logged_In = (patient != NULL);	//如果传入的患者指针不为NULL，说明患者已登录；如果为NULL，说明患者已退出登录
}

// 根据医生编号在系统中查找对应的医生信息
// 区别于doctor文件内的findDoctorById函数，这里可以返回医生信息指针，供挂号时使用；
// 而doctor文件内的函数主要用于验证医生编号是否存在，无法返回医生信息指针
static doctor* findDoctorById(HIS_System* sys, const char* doctorId) {
	doctor* curr = sys->docHead;
	while (curr != NULL) {
		if (strcmp(curr->doctorId, doctorId) == 0) {
			return curr;
		}
		curr = curr->next;
	}
	return NULL;
}

// 打印所有可选的挂号预约时间段列表，并提示患者输入选择的时段编号，返回对应的TimeSlot枚举值
static TimeSlot inputTimeSlotChoice(void) {
	printAllTimeSlots();
	printf(">>> 请选择时段编号(1-%d, 午休时段11:30-13:30暂不开放): ", SLOT_COUNT);
	int slotNo = safeGetInt("");
	if (slotNo < 1 || slotNo > SLOT_COUNT) {
		return SLOT_INVALID;
	}
	TimeSlot slot = (TimeSlot)slotNo;
	if (isNoonSlot(slot)) {
		printf(">>> 午休时段（11:30-13:30）暂不开放看诊，请选择其他时段。\n");
		return SLOT_INVALID;
	}
	return slot;
}

// 将新的挂号记录追加到患者的挂号记录链表末尾
static void appendRegistrationRecord(Patient* patient, const char* doctorId, const char* department, const char* date, TimeSlot slot) {
	// 检查是否已存在相同的挂号记录（同医生同日期同时段），避免重复追加
	RegistrationRecord* exist = patient->regHead;
	while (exist != NULL) {
		if (strcmp(exist->doctorId, doctorId) == 0 &&
			strcmp(exist->date, date) == 0 &&
			strcmp(exist->time, slot_names[slot - 1]) == 0) {
			printf(">>> 该时段挂号记录已存在，无需重复添加。\n");
			return;
		}
		exist = exist->next;
	}

	RegistrationRecord* reg = (RegistrationRecord*)malloc(sizeof(RegistrationRecord));
	if (reg == NULL) {
		printf(">>> 内存不足，挂号记录写入失败。\n");
		return;
	}
	sprintf(reg->recordId, "R%s%03d", patient->patientId, rand() % 1000);	//生成挂号记录编号，格式为R+患者编号+3位随机数（如R10000001001）TODO: 可以改进为更有规律的编号生成方式，如使用全局挂号记录计数器，确保编号唯一且有序
	strcpy(reg->doctorId, doctorId);
	strcpy(reg->department, department);
	strcpy(reg->date, date);
	strcpy(reg->time, slot_names[slot - 1]);
	reg->next = NULL;
	if (patient->regHead == NULL) {
		patient->regHead = reg;
		patient->currRegTail = reg;
	}
	else {
		patient->currRegTail->next = reg;
		patient->currRegTail = reg;
	}
}
	

// 生成新的患者编号，格式为P+8位数字（如P10000001），并返回编号字符串指针
static char* generatePatientId() {
	static char id[ID_LEN];
	sprintf(id, "P%08d", currentPatientId++);	//生成患者编号，格式为P+8位数字（如P10000001）
	return id;
}

bool isIdCardExist(HIS_System* sys, const char* idCard) {
	Patient* curr = sys->patientHead;
	while (curr) {
		if (strcmp(curr->idCard, idCard) == 0) return true;
		curr = curr->next;
	}
	return false;
}

void registerPatient(HIS_System* sys, const char* remainIdCard) {
	while (1) {
		printf("\n--- 患者注册（在任意输入环节输入 '-1' 可取消本次注册) ---）\n");
		Patient* newPatient = (Patient*)malloc(sizeof(Patient));
		if (newPatient == NULL) {
			printf(">>> 内存分配失败，无法进行患者注册！\n");
			return;
		}
		bool cancelFlag = false;

		strcpy(newPatient->patientId, generatePatientId());	//生成患者编号
		// 确保生成的患者编号不重复
		while (findPatientById(sys, newPatient->patientId) != NULL) {
			printf(">>> 警告: 患者编号 %s 已存在，重新生成。\n", newPatient->patientId);
			strcpy(newPatient->patientId, generatePatientId());
		}

		safeGetString("请输入患者姓名: ", newPatient->name, STR_LEN);
		if (strcmp(newPatient->name, "-1") == 0) {
			cancelFlag = true;
			free(newPatient);
			printf(">>> 已取消患者注册！正在返回操作菜单...\n");
			return; // 修正：free 后立即 return，避免后续访问 newPatient
		}

		safeGetString("请输入患者电话: ", newPatient->phone, ID_LEN);
		if (strcmp(newPatient->phone, "-1") == 0) {
			cancelFlag = true;
			free(newPatient);
			printf(">>> 已取消患者注册！正在返回操作菜单...\n");
			return;
		}

		while (1) {
			if(remainIdCard != NULL) {
				if (confirmFunc("使用", "上次输入的身份证号")) {
					printf(">>> 已使用上次输入的身份证号: %s\n", remainIdCard);
					strcpy(newPatient->idCard, remainIdCard);
				}
				else {
					safeGetString("请输入患者身份证号: ", newPatient->idCard, 18 + 3);
				}
				remainIdCard = NULL; 
			} 
			else {
				safeGetString("请输入患者身份证号: ", newPatient->idCard, 18 + 3);
			}

			if (strcmp(newPatient->idCard, "-1") == 0) {
				cancelFlag = true;
				free(newPatient);
				printf(">>> 已取消患者注册！正在返回操作菜单...\n");
				break;
			}

			if (isIdCardExist(sys, newPatient->idCard)) {
				printf(">>> 患者身份证号已存在，请重新输入！\n");
				continue;
			}

			break;
		}

		safeGetString("请输入患者性别: ", newPatient->gender, STR_LEN);
		if (strcmp(newPatient->gender, "-1") == 0) {
			cancelFlag = true;
			free(newPatient);
			printf(">>> 已取消患者注册！正在返回操作菜单...\n");
			return;
		}

		while (1) {
			newPatient->age = safeGetInt("请输入患者年龄: ");
			if (newPatient->age == -1) {
				cancelFlag = true;
				free(newPatient);
				printf(">>> 已取消患者注册！正在返回操作菜单...\n");
				return;
			}

			if (newPatient->age < 0 || newPatient->age > 150) {
				printf(">>> 年龄不合理(0-150)，请重新输入。\n");
				continue;
			}

			break;
		}

		newPatient->type = safeGetInt("请输入患者类别 (1-普通, 2-VIP, 3-急诊): ");
		if (newPatient->type == -1) {
			cancelFlag = true;
			free(newPatient);
			printf(">>> 已取消患者注册！正在返回操作菜单...\n");
			return;
		}

		if (!cancelFlag) {
			// 初始化患者记录链表头和末尾指针
			newPatient->regHead = NULL;
			newPatient->viewHead = NULL;
			newPatient->stayHead = NULL;
			newPatient->medHead = NULL;
			newPatient->currRegTail = NULL;
			newPatient->currViewTail = NULL;
			newPatient->currStayTail = NULL;
			newPatient->currMedTail = NULL;
			newPatient->realBalance = 0.0;
			newPatient->bonusBalance = 0.0;
			newPatient->loginCount = 0;
			newPatient->next = NULL;

			// 将新患者添加到链表末尾
			if (sys->patientTail == NULL) {	//链表为空，新患者成为头节点
				sys->patientHead = newPatient;
				sys->patientTail = newPatient;	//初始化末尾指针
			}
			else {	//链表不空，添加到末尾
				sys->patientTail->next = newPatient;
				sys->patientTail = newPatient;	//更新末尾指针
			}
			printf(">>> 患者 <%s> 注册成功！\n", newPatient->name);
			savePatientsSystemData(sys);	// 注册成功后立即保存数据到文件，确保数据持久化
			//注册成功后询问是否直接登录
			if(confirmFunc("登录", "患者账户")) {
				setCurrentPatient(newPatient); // 设置当前登录的患者
				printf(">>> 登录成功！欢迎您，%s！正在返回患者服务台...\n", newPatient->name);
			}
			return;
		}
		else {
			printf(">>> 已取消患者注册！正在返回操作菜单...\n");
			return;
		}
	}
}

void logInPatient(HIS_System* sys) {
	while (1) {
		if(is_Patient_Logged_In) {
			Patient* currPatient = getCurrentPatientNode();
			printf("\n>>> 已登录患者账户 %s，", currPatient->name);
			if(confirmFunc("更换", "新的患者账户")) {
				setCurrentPatient(NULL); // 设置当前登录的患者为NULL，表示退出登录
				printf(">>> 已退出患者 %s账户！请重新登录!\n", currPatient->name);
			}
			else {
				printf(">>> 正在返回患者服务台...\n");
				return;
			}
		}
		printf("\n--- 患者登录（输入 '-1' 可取消本次登录) ---\n");
		char idCard[18 + 3];
		safeGetString("请输入患者身份证号: ", idCard, 18 + 3);
		if (strcmp(idCard, "-1") == 0) {
			printf(">>> 已取消患者登录！正在返回操作菜单...\n");
			return;
		}
		Patient* curr = sys->patientHead;
		if(curr == NULL) {
			if (!TEST_SYSTEM_DEBUG) {
				printf("严重错误: 当前没有患者数据，请查询权限或联系管理员。\n");
				exit(EXIT_FAILURE);
			}
			printf(">>> 当前没有患者数据，请先进行患者注册！正在返回操作菜单...\n");
			return;
		}
		while (curr) {
			if (strcmp(curr->idCard, idCard) == 0) {
				curr->loginCount++;
				if (getTotalBalance(curr) <= -2000.0 && curr->loginCount > 1) {
					printf(">>> 警告: 您的账户已欠费超过2000元，账户已锁定！请立即充值。\n");
					patientRechargeMenuForPatient(sys, curr);
					if (getTotalBalance(curr) <= -2000.0) {
						printf(">>> 余额仍不足，账户继续锁定。请充值后再试。\n");
						return;
					}
				}
				printf(">>> 登录成功！欢迎您，%s！正在返回患者服务台...\n", curr->name);
				setCurrentPatient(curr); // 设置当前登录的患者
				return;
			}
			curr = curr->next;
		}
		printf(">>> 患者身份证号%s不存在，请检查输入是否正确！\n", idCard);
		printf(">>> 如果该身份证号还没有被注册，请先进行患者注册。\n");
		if (confirmFunc("注册", "患者")) {	//如果用户选择注册，直接调用注册函数，并直接利用患者输入的身份证
			registerPatient(sys, idCard);
			return;
		}
	}
}

void logOutPatient() {
	if(is_Patient_Logged_In) {
		printf(">>> 已登录患者账户 %s，", getCurrentPatientNode()->name);
		if(confirmFunc("退出", "当前患者账户")) {
			setCurrentPatient(NULL); // 设置当前登录的患者为NULL，表示退出登录
			printf(">>> 已退出患者账户！正在返回操作菜单...\n");
		}
		else printf(">>> 正在返回患者服务台...\n");
	}
	else {
		printf(">>> 您尚未登录患者账户，无需退出！正在返回操作菜单...\n");
	}
}

void registerAppointment(HIS_System* sys) {
	if(!is_Patient_Logged_In) {
		printf(">>> 请先登录患者账户后再进行挂号预约！正在返回操作菜单...\n\n");
		return;
	}
	if (sys->docHead == NULL && !TEST_SYSTEM_DEBUG) {
		printf("严重错误: 当前没有医生数据，请查询权限或联系管理员。\n");
		exit(EXIT_FAILURE);
	}
	while (1) {
		printf("\n--- 挂号与签到 ---\n");
		printf("1. 预约挂号\n");
		printf("2. 当场挂号\n");
		printf("3. 签到排队\n");
		printf("4. 晚间急诊挂号\n");
		printf("0. 返回上一级菜单\n");
		int choice = safeGetInt("请选择操作: ");
		if (choice == 0) {
			return;
		}

		// 晚间急诊挂号：完全独立入口
		if (choice == 4) {
			registerNightEmergency(sys);
			continue;
		}

		if (!TEST_SYSTEM_DEBUG) {
			TimeSlot currSlot = changeTimeToSlot(getCurrentTimeStr());
			// 当场挂号：时段最后3分钟禁止
			if (choice == 2) {
				int h, m, s;
				getCurrentTime(&h, &m, &s);
				int nowMin = h * 60 + m;
				int slotStart = 480 + (currSlot - 1) * 30;
				if (nowMin - slotStart >= 27) {
					printf("\n>>> 为确保看诊秩序，每个时段内最后3分钟不可当场挂号! \n");
					continue;
				}
			}
		}


		if (choice == 3) {
			// 收集当前患者所有待签到挂号记录（未签到且未被取消）
			Patient* curPat = getCurrentPatientNode();
			QueueTicket* tickets[256];
			int tCount = 0;
			QueueTicket* t = queueGetTicketHead();
			while (t != NULL) {
				if (t->patient == curPat && !t->checkedIn && t->status != STATUS_CANCELLED) {
					if (tCount < 256) tickets[tCount++] = t;
				}
				t = t->next;
			}
			if (tCount == 0) {
				printf(">>> 当前没有待签到的挂号记录。\n");
				continue;
			}
			printf("\n--- 待签到挂号记录 ---\n");
			printFormattedStr("序号", 5);
			printFormattedStr("日期", 12);
			printFormattedStr("时段", 14);
			printFormattedStr("医生编号", 12);
			printFormattedStr("类型", 8);
			printf("\n");
			for (int i = 0; i < tCount; i++) {
				char buf[32];
				sprintf(buf, "%d", i + 1);
				printFormattedStr(buf, 5);
				printFormattedStr(tickets[i]->date, 12);
				printFormattedStr(slot_names[tickets[i]->slot - 1], 14);
				printFormattedStr(tickets[i]->doctor->doctorId, 12);
				printFormattedStr(tickets[i]->isOnsite ? "当场" : "预约", 8);
				printf("\n");
			}
			printf("0. 取消签到\n");

			int sel = safeGetInt("请选择要签到的记录序号: ");
			if (sel == 0) {
				printf(">>> 已取消签到。\n");
				continue;
			}
			if (sel < 1 || sel > tCount) {
				printf(">>> 序号无效。\n");
				continue;
			}
			QueueTicket* chosen = tickets[sel - 1];
			char signTime[TIME_STR_LEN];
			if (TEST_SYSTEM_DEBUG) {
				if (confirmFunc("使用", "自定义时间")) {
					safeGetString(">>> 请输入自定义时间(HH:MM): ", signTime, TIME_STR_LEN);
				} else {
					strcpy(signTime, getCurrentTimeStr());
				}
			} else {
				strcpy(signTime, getCurrentTimeStr());
			}
			char usingDate[DATE_STR_LEN];
			if (TEST_SYSTEM_DEBUG) {
				if (confirmFunc("使用", "自定义日期")) {
					safeGetString(">>> 请输入自定义日期(YYYY-MM-DD): ", usingDate, DATE_STR_LEN);
				} else {
					strcpy(usingDate, getCurrentDateStr());
				}
			} else {
				strcpy(usingDate, getCurrentDateStr());
			}
			if (checkInQueueTicket(curPat->patientId, chosen->doctor->doctorId, chosen->date, chosen->slot, signTime)) {
				printf(">>> 签到成功！请查看候诊队列：\n");
				printSlotQueue(chosen->doctor->doctorId, chosen->date, chosen->slot);
			}
			continue;
		}

		if (choice != 1 && choice != 2) {
			printf(">>> 无效的选择，请输入 0/1/2/3/4。\n");
			continue;
		}

		// 挂号前支持查询医生信息
		if (confirmFunc("查询", "医生信息")) {
			queryDoctorPat(sys, getCurrentPatientId());
			if (!confirmFunc("继续", "挂号操作")) {
				printf(">>> 已取消挂号操作，正在返回挂号菜单...\n");
				continue;
			}
		}

		char doctorId[ID_LEN];
		safeGetString(">>> 请输入医生编号(输入 -1 取消): ", doctorId, ID_LEN);
		if (strcmp(doctorId, "-1") == 0) {
			printf(">>> 已取消挂号。\n");
			continue;
		}
		doctor* doctor = findDoctorById(sys, doctorId);
		if (doctor == NULL) {
			printf(">>> 未找到该医生，请检查输入。\n");
			continue;
		}

		TimeSlot slot = SLOT_INVALID;
		char testTime[TIME_STR_LEN];
		bool isUseCustomTime = false;	//标记是否使用了自定义时间，主要用于当场挂号后自动签到时的时间判断
		if (choice == 1){
			slot = inputTimeSlotChoice();
			if (slot <= SLOT_INVALID || slot > SLOT_COUNT) {
				printf(">>> 时段编号无效。\n");
				continue;
			}
		}
		else if(choice == 2){
			if(TEST_SYSTEM_DEBUG) {
				printf(">>> 测试模式下，当场挂号可以选择使用自定义当前时间。\n");
				if (confirmFunc("使用", "自定义当前时间")) {
					safeGetString(">>> 请输入自定义当前时间(HH:MM): ", testTime, TIME_STR_LEN);
					strcat(testTime, ":00");	//将用户输入的时间补全为HH:MM:SS格式，秒部分默认为00
					strcpy(testTime, setTestTime(testTime));
					slot = changeTimeToSlot(testTime);
					isUseCustomTime = true;
				}
				else
					slot = changeTimeToSlot(getCurrentTimeStr());
			}
			else
				slot = changeTimeToSlot(getCurrentTimeStr());

			if (slot == SLOT_INVALID) {
				printf(">>> 当场挂号当前没有可用的时段。\n");
				continue;
			}
		}

		char date[DATE_STR_LEN];
		if (choice == 1) {
			safeGetString(">>> 请输入预约日期(YYYY-MM-DD, 输入 -1 取消): ", date, DATE_STR_LEN);
			if (strcmp(date, "-1") == 0) {
				printf(">>> 已取消挂号。\n");
				continue;
			}
		}
		else {
			strcpy(date, getCurrentDateStr());
			printf(">>> 当场挂号默认日期为 %s\n", date);
		}

		if (!isValidDate(date)) {
			printf(">>> 日期格式无效。\n");
			continue;
		}
		
		if (TEST_SYSTEM_DEBUG) {
			printf(">>> 测试模式下，可以选择使用自定义日期或当前日期。\n");
			if (confirmFunc("使用", "自定义日期")) {
				safeGetString(">>> 请输入自定义日期(YYYY-MM-DD): ", date, DATE_STR_LEN);
				if (!isValidDate(date)) {
					printf(">>> 日期格式无效。\n");
					continue;
				}
			}
			else {
				strcpy(date, getCurrentDateStr());
				printf(">>> 已使用当前日期: %s\n", date);
			}
		}

		// 当场挂号：时段最后3分钟禁止
		if (choice == 2) {
			int h, m, s;
			getCurrentTime(&h, &m, &s);
			int nowMin = h * 60 + m;
			int slotStart = 480 + (slot - 1) * 30;
			if (nowMin - slotStart >= 27) {
				printf(">>> 当前时段最后3分钟不可当场挂号。\n");
				continue;
			}
		}

		//尝试挂号，如果挂号成功则追加挂号记录并打印挂号成功信息和当前时段剩余号源数量；如果是当场挂号且挂号成功，则自动进行签到并打印相关信息
		if (bookQueueTicket(getCurrentPatientNode(), doctor, date, slot, choice == 2)) {
			appendRegistrationRecord(getCurrentPatientNode(), doctor->doctorId, doctor->department, date, slot);
			printf(">>> 挂号成功：医生[%s] 日期[%s] 时段[%s]。\n", doctor->doctorName, date, slot_names[slot - 1]);
			printf(">>> 当前时段剩余号源：%d\n", getDoctorSlotRemain(doctor->doctorId, date, slot));
			if (choice == 2) {
				char signTime[TIME_STR_LEN];
					if(TEST_SYSTEM_DEBUG) {
						if (confirmFunc("使用", "自定义时间")) {
							if(isUseCustomTime)
								strcpy(signTime, testTime);	//如果之前已经使用了自定义时间，直接使用该时间作为签到时间，避免用户再次输入；如果之前没有使用自定义时间，则让用户输入一个新的自定义时间
							else
								safeGetString(">>> 请输入自定义时间(HH:MM): ", signTime, TIME_STR_LEN);
						}
						else
							strcpy(signTime, getCurrentTimeStr());
					}
					else
						strcpy(signTime, getCurrentTimeStr());
				if (checkInQueueTicket(getCurrentPatientNode()->patientId, doctor->doctorId, date, slot, signTime)) {
					printf(">>> 当场挂号已自动签到，请及时就诊。\n");
					printSlotQueue(doctor->doctorId, date, slot);
				}
			}
		}
	}
}

// 晚间急诊挂号（独立入口，与白天挂号完全隔离）
void registerNightEmergency(HIS_System* sys) {
	if (!is_Patient_Logged_In) {
		printf(">>> 请先登录后再进行晚间急诊挂号！\n");
		return;
	}
	if (sys->docHead == NULL) {
		if (!TEST_SYSTEM_DEBUG) {
			printf("严重错误: 医生数据不存在！\n");
			exit(EXIT_FAILURE);
		}
		printf(">>> 警告: 医生数据为空。\n");
		return;
	}

	char date[DATE_STR_LEN];
	if (TEST_SYSTEM_DEBUG) {
		char* testDate = setTestDate("2026-05-09");
		strcpy(date, testDate);
	}
	else {
		strcpy(date, getCurrentDateStr());
	}

	printf("\n========== 晚间急诊挂号 ==========\n");
	printf("日期: %s\n", date);
	printf("----------------------------------\n");

	// 收集当天有晚间急诊排班的医生
	typedef struct {
		char doctorId[ID_LEN];
		char doctorName[STR_LEN];
	} NightDoc;
	NightDoc nightDocs[64];
	int nightDocCount = 0;

	doctor* d = sys->docHead;
	while (d != NULL) {
		if (isDoctorSlotOpen(d->doctorId, date, SLOT_NIGHT)) {
			strcpy(nightDocs[nightDocCount].doctorId, d->doctorId);
			strcpy(nightDocs[nightDocCount].doctorName, d->doctorName);
			nightDocCount++;
		}
		d = d->next;
	}

	if (nightDocCount == 0) {
		printf(">>> 今晚暂无晚间急诊值班医生。\n");
		printf("==================================\n");
		pressEnterToContinue();
		return;
	}

	// 显示可选医生列表
	printf("可选晚间急诊医生:\n");
	for (int i = 0; i < nightDocCount; ++i) {
		printf("%d) %s (%s)\n", i + 1, nightDocs[i].doctorName, nightDocs[i].doctorId);
	}
	printf("0) 返回\n");
	printf("----------------------------------\n");

	int choice = safeGetInt("请选择医生: ");
	if (choice == 0) {
		return;
	}
	if (choice < 1 || choice > nightDocCount) {
		printf(">>> 无效选择。\n");
		return;
	}

	doctor* chosen = findDoctorById(sys, nightDocs[choice - 1].doctorId);
	if (chosen == NULL) {
		printf(">>> 医生数据错误。\n");
		return;
	}

	// 执行晚间急诊挂号
	if (bookNightEmergencyTicket(getCurrentPatientNode(), chosen, date)) {
		printf(">>> 晚间急诊挂号成功！当前为急诊通道，请耐心等候叫号。\n");
		printNightQueue(chosen->doctorId, date);
	}
}

// 写入看诊病例信息（医生开具）
bool appendViewMedicalRecord(HIS_System* sys, const char* patientId, const char* doctorId, const char* details, const char* date) {
	if (sys == NULL || patientId == NULL || doctorId == NULL || details == NULL || date == NULL) {
		return false;
	}
	Patient* target = findPatientById(sys, patientId);
	if (target == NULL) {
		return false;
	}
	ConsultationRecord* rec = (ConsultationRecord*)malloc(sizeof(ConsultationRecord));
	if (rec == NULL) {
		printf(">>> 内存不足，看诊病例写入失败。\n");
		return false;
	}
	memset(rec, 0, sizeof(ConsultationRecord));
	generateRecordId(rec->recordId, "V", patientId);
	rec->record = REC_VIEW;
	strncpy(rec->details, details, sizeof(rec->details) - 1);
	rec->details[sizeof(rec->details) - 1] = '\0';
	strncpy(rec->date, date, ID_LEN - 1);
	rec->date[ID_LEN - 1] = '\0';
	strncpy(rec->doctorId, doctorId, ID_LEN - 1);
	rec->doctorId[ID_LEN - 1] = '\0';
	rec->next = NULL;

	if (target->viewHead == NULL) {
		target->viewHead = rec;
		target->currViewTail = rec;
	}
	else {
		target->currViewTail->next = rec;
		target->currViewTail = rec;
	}

	savePatientsSystemData(sys);
	return true;
}

// 写入住院病例信息（deptInfo/床位置由调用方传入，details 固定为"入院"）
bool appendStayMedicalRecord(HIS_System* sys, const char* patientId, const char* doctorId, const char* deptInfo, const char* bedId, const char* startDate, const char* duration, const char* endDate, const char* wardId) {
	if (sys == NULL || patientId == NULL || doctorId == NULL || deptInfo == NULL || bedId == NULL || startDate == NULL || duration == NULL || endDate == NULL || wardId == NULL) {
		return false;
	}
	Patient* target = findPatientById(sys, patientId);
	if (target == NULL) {
		return false;
	}
	StayRecord* rec = (StayRecord*)malloc(sizeof(StayRecord));
	if (rec == NULL) {
		printf(">>> 内存不足，住院病例写入失败。\n");
		return false;
	}
	memset(rec, 0, sizeof(StayRecord));
	generateRecordId(rec->recordId, "S", patientId);
	strncpy(rec->startDate, startDate, ID_LEN - 1);
	rec->startDate[ID_LEN - 1] = '\0';
	strncpy(rec->duration, duration, ID_LEN - 1);
	rec->duration[ID_LEN - 1] = '\0';
	strncpy(rec->endDate, endDate, ID_LEN - 1);
	rec->endDate[ID_LEN - 1] = '\0';
	strncpy(rec->deptInfo, deptInfo, STR_LEN - 1);
	rec->deptInfo[STR_LEN - 1] = '\0';
	strncpy(rec->doctorId, doctorId, ID_LEN - 1);
	rec->doctorId[ID_LEN - 1] = '\0';
	strncpy(rec->wardId, wardId, ID_LEN - 1);
	rec->wardId[ID_LEN - 1] = '\0';
	strncpy(rec->bedId, bedId, BED_ID_LEN - 1);
	rec->bedId[BED_ID_LEN - 1] = '\0';
	rec->dischargeApproved = 0;
	strncpy(rec->details, "入院", sizeof(rec->details) - 1);
	rec->details[sizeof(rec->details) - 1] = '\0';
	rec->next = NULL;

	if (target->stayHead == NULL) {
		target->stayHead = rec;
		target->currStayTail = rec;
	}
	else {
		target->currStayTail->next = rec;
		target->currStayTail = rec;
	}

	savePatientsSystemData(sys);
	return true;
}

// 打印处方明细表格（通用函数），解析details字符串并格式化输出，返回处方总计金额
// details格式: drugId:genericName:qty;drugId:genericName:qty;...
static double printPrescriptionDetail(HIS_System* sys, const char* details) {
	if (sys == NULL || details == NULL || details[0] == '\0') {
		return 0.0;
	}
	char buffer[256];
	double prescTotal = 0.0;

	printFormattedStr("序号", 6);
	printFormattedStr("药品编号", 12);
	printFormattedStr("通用名", 20);
	printFormattedStr("数量", 6);
	printFormattedStr("单价", 8);
	printFormattedStr("小计", 10);
	printf("\n");
	printf("-------------------------------------------------------------\n");

	char detailsCopy[512];
	strncpy(detailsCopy, details, sizeof(detailsCopy) - 1);
	detailsCopy[sizeof(detailsCopy) - 1] = '\0';

	// 第一遍：解析并合并相同药品
	struct MergedItem {
		char dId[ID_LEN];
		char dName[STR_LEN];
		int qty;
	} items[100];
	int itemCount = 0;

	char* token = strtok(detailsCopy, ";");
	while (token != NULL) {
		char dId[ID_LEN] = "";
		char dName[STR_LEN] = "";
		int qty = 0;
		if (sscanf(token, "%24[^:]:%49[^:]:%d", dId, dName, &qty) == 3) {
			int found = -1;
			for (int k = 0; k < itemCount; k++) {
				if (strcmp(items[k].dId, dId) == 0) { found = k; break; }
			}
			if (found >= 0) {
				items[found].qty += qty;
			} else if (itemCount < 100) {
				strcpy(items[itemCount].dId, dId);
				strcpy(items[itemCount].dName, dName);
				items[itemCount].qty = qty;
				itemCount++;
			}
		}
		token = strtok(NULL, ";");
	}

	// 第二遍：展示合并后的条目
	for (int i = 0; i < itemCount; i++) {
		Drug* d = findDrugById(sys, items[i].dId);
		double unitPrice = (d != NULL) ? d->price : 0.0;
		double sub = unitPrice * items[i].qty;
		prescTotal += sub;
		sprintf(buffer, "%d", i + 1);
		printFormattedStr(buffer, 6);
		printFormattedStr(items[i].dId, 12);
		printFormattedStr(items[i].dName, 20);
		sprintf(buffer, "%d", items[i].qty);
		printFormattedStr(buffer, 6);
		sprintf(buffer, "%.2f", unitPrice);
		printFormattedStr(buffer, 8);
		sprintf(buffer, "%.2f", sub);
		printFormattedStr(buffer, 10);
		printf("\n");
	}
	printf("-------------------------------------------------------------\n");
	return prescTotal;
}

// 患者查看自己的病例信息
void viewMedicalRecordPat(HIS_System* sys, const char* patientId) {
	if (sys == NULL) {
		printf(">>> 系统未初始化，无法查看病例信息。\n");
		return;
	}
	if (patientId == NULL) {
		if (!is_Patient_Logged_In || getCurrentPatientNode() == NULL || getCurrentPatientId() == NULL) {
			printf(">>> 请先登录患者账户后再查看病例信息。\n");
			return;
		}
		patientId = getCurrentPatientNode()->patientId;
	}

	loadPatientsSystemData(sys);

	Patient* target = findPatientById(sys, patientId);
	if (target == NULL) {
		printf(">>> 未找到患者 %s 的信息。\n", patientId);
		return;
	}

	printf("\n========== 患者病例信息 ==========""\n");
	printf("患者编号: %s\n", target->patientId);
	printf("患者姓名: %s\n", target->name);
	printf("患者性别: %s\n", target->gender);
	printf("患者年龄: %d\n", target->age);
	printf("患者类型: %s\n",
		target->type == PATIENT_VIP ? "VIP" :
		target->type == PATIENT_EMERGENCY ? "急诊" : "普通");

	RegistrationRecord* reg = target->regHead;
	printf("\n--- 挂号记录 ---\n");
	if (reg == NULL) {
		printf(">>> 暂无挂号记录。\n");
	}
	while (reg != NULL) {
		printf("记录编号: %s | 科室: %s | 医生: %s | 日期: %s | 时段: %s\n", reg->recordId, reg->department, reg->doctorId, reg->date, reg->time);
		reg = reg->next;
	}

	ConsultationRecord* view = target->viewHead;
	printf("\n--- 看诊记录 ---\n");
	if (view == NULL) {
		printf(">>> 暂无看诊记录。\n");
	}
	while (view != NULL) {
		printConsultationRecord(view);
		printf("------------------------------\n");
		view = view->next;
	}

	// 处方记录（从药品链表查找单价显示）
	printf("\n--- 处方记录 ---\n");
	if (target->medHead == NULL) {
		printf(">>> 暂无处方记录。\n");
	} else {
		ConsultationRecord* med = target->medHead;
		while (med != NULL) {
			printf("记录编号: %s\n", med->recordId);
			printf("记录日期: %s\n", med->date);
			printf("医生编号: %s\n", med->doctorId);
			printf("明细:\n");
			double prescTotal = printPrescriptionDetail(sys, med->details);
			printf("处方总计: %.2f 元\n", prescTotal);

			med = med->next;
			if (med != NULL) {
				printf("------------------------------\n");
			}
		}
	}

	printf("\n--- 检查记录 ---\n");
	bool hasExam = false;
	ExamOrder* order = sys->examOrderHead;
	while (order != NULL) {
		if (strcmp(order->patientId, target->patientId) == 0) {
			printExamOrderDetail(order);
			hasExam = true;
		}
		order = order->next;
	}
	if (!hasExam) {
		printf(">>> 暂无检查记录。\n");
	}

	printf("\n--- 住院记录 ---\n");
	if (target->stayHead == NULL) {
		printf(">>> 暂无住院记录。\n");
	} else {
		int stayCount = 0;
		StayRecord* s = target->stayHead;
		while (s != NULL) {
			stayCount++;
			s = s->next;
		}
		StayRecord** stays = (StayRecord**)malloc(sizeof(StayRecord*) * stayCount);
		if (stays == NULL) {
			printf(">>> 内存不足，无法展示住院记录。\n");
		} else {
			s = target->stayHead;
			for (int i = 0; i < stayCount; i++) {
				stays[i] = s;
				s = s->next;
			}
			// 按入院日期升序排序（最早的在上），同日保持原链表顺序（稳定排序）
			for (int i = 1; i < stayCount; i++) {
				StayRecord* key = stays[i];
				int j = i - 1;
				while (j >= 0 && strcmp(stays[j]->startDate, key->startDate) > 0) {
					stays[j + 1] = stays[j];
					j--;
				}
				stays[j + 1] = key;
			}
			// 表头
			const int wSeq = 5, wWard = 11, wDept = 12, wBed = 9, wStart = 12, wEnd = 12, wDiag = 20;
			printFormattedStr("序号", wSeq);
			printFormattedStr("病房编号", wWard);
			printFormattedStr("科室", wDept);
			printFormattedStr("床位编号", wBed);
			printFormattedStr("入院日期", wStart);
			printFormattedStr("出院日期", wEnd);
			printFormattedStr("出院诊断", wDiag);
			printf("\n");
			for (int i = 0; i < stayCount; i++) {
				char buf[32];
				sprintf(buf, "%d", i + 1);
				printFormattedStr(buf, wSeq);
				printFormattedStr(stays[i]->wardId, wWard);
				printFormattedStr(stays[i]->deptInfo, wDept);
				printFormattedStr(stays[i]->bedId, wBed);
				printFormattedStr(stays[i]->startDate, wStart);
				bool discharged = (strcmp(stays[i]->endDate, "未出院") != 0 && strcmp(stays[i]->endDate, "待出院") != 0);
				printFormattedStr(discharged ? stays[i]->endDate : "未出院", wEnd);
				printFormattedStr(stays[i]->details, wDiag);
				printf("\n");
			}
			free(stays);
		}
	}
	printf("================================\n");
	pressEnterToContinue();
}

// 根据患者编号查找对应的看诊记录链表头指针，供医生查看病例信息时使用
static ConsultationRecord* findConsultationRecordByPatientId(HIS_System* sys, const char* patientId) {
	if (sys == NULL || patientId == NULL) {
		return NULL;
	}
	Patient* target = findPatientById(sys, patientId);
	if (target == NULL) {
		return NULL;
	}
	return target->viewHead; //返回看诊记录链表头指针，供医生查看病例信息时使用
}

// 医生查看患者病例信息（子菜单）
void viewMedicalRecordDoc(HIS_System* sys, const char* doctorId) {
	if (sys == NULL) {
		printf(">>> 系统未初始化，无法查看病例信息。\n");
		return;
	}
	if (doctorId == NULL) {
		printf(">>> 医生身份无效，无法查看病例信息。\n");
		return;
	}

	int choice;
	while (1) {
		printf("\n====== 查看患者病历 ======\n");
		printf("1. 查看当前叫号患者最新病历\n");
		printf("2. 按日期搜索看诊记录\n");
		printf("3. 查看指定患者全部病历\n");
		printf("0. 返回上一级\n");
		printf("============================\n");
		choice = safeGetInt("请选择操作: ");

		switch (choice) {
		case 1: {
			// 查找当前被本医生叫号的患者
			const char* calledId = findCalledPatientIdByDoctor(doctorId);
			if (calledId == NULL) {
				printf(">>> 当前没有被您叫号的患者。\n");
				break;
			}
			Patient* p = findPatientById(sys, calledId);
			if (p == NULL) {
				printf(">>> 未找到患者 %s 的信息。\n", calledId);
				break;
			}
			printf(">>> 当前叫号患者: %s (%s)，正在加载病历...\n", p->name, calledId);
			viewMedicalRecordPat(sys, calledId);
			break;
		}
		case 2: {
			// 按日期搜索本医生的看诊记录
			char date[DATE_STR_LEN];
			safeGetString("请输入搜索日期(YYYY-MM-DD, 输入 -1 取消): ", date, DATE_STR_LEN);
			if (strcmp(date, "-1") == 0) {
				printf(">>> 已取消搜索。\n");
				break;
			}
			printf("\n--- %s 看诊记录 [%s] ---\n", doctorId, date);
			bool found = false;
			Patient* curr = sys->patientHead;
			while (curr != NULL) {
				ConsultationRecord* rec = curr->viewHead;
				while (rec != NULL) {
					if (strcmp(rec->doctorId, doctorId) == 0
						&& strcmp(rec->date, date) == 0) {
						printf("患者: %s (%s)\n", curr->name, curr->patientId);
						printConsultationRecord(rec);
						printf("------------------------------\n");
						found = true;
					}
					rec = rec->next;
				}
				curr = curr->next;
			}
			if (!found) {
				printf(">>> 该日期暂无看诊记录。\n");
			}
			pressEnterToContinue();
			break;
		}
		case 3: {
			// 输入患者编号查看全部病历（含权限判定）
			char patientId[ID_LEN];
			safeGetString("请输入需要查看的患者编号(输入 -1 取消): ", patientId, ID_LEN);
			if (strcmp(patientId, "-1") == 0) {
				printf(">>> 已取消查看。\n");
				break;
			}
			Patient* targetPatient = findPatientById(sys, patientId);
			if (targetPatient == NULL) {
				printf(">>> 未找到患者 %s 的信息。\n", patientId);
				break;
			}
			// 权限判定：已叫号 或 曾有看诊记录
			if (isPatientCalledByDoctor(patientId, doctorId) || hasPatientCalledByDoctor(patientId, doctorId)) {
				printf(">>> 权限验证通过：患者已由您叫号，正在加载病历...\n");
				viewMedicalRecordPat(sys, patientId);
			} else {
				ConsultationRecord* targetView = findConsultationRecordByPatientId(sys, patientId);
				if (targetView != NULL && strcmp(doctorId, targetView->doctorId) == 0) {
					printf(">>> 权限验证通过：您曾为该患者看诊，正在加载病历...\n");
					viewMedicalRecordPat(sys, patientId);
				} else {
					printf(">>> 权限不足：该患者未被您叫号，且您不是该患者的主治医生。\n");
					printf(">>> 提示：请先通过【排队叫号】叫号该患者，或确认患者编号是否正确。\n");
				}
			}
			break;
		}
		case 0:
			return;
		default:
			printf(">>> 无效选择，请重试。\n");
		}
	}
}

// 根据医生编号查找当前正在看诊的患者编号，供写入诊断时自动选择患者使用
// 优先通过QueueTicket状态查找（已叫号/就诊中的患者），其次通过看诊记录查找
static const char* findCurrentConsultationPatientId(HIS_System* sys, const char* doctorId) {
	if (sys == NULL || doctorId == NULL) {
		return NULL;
	}
	// 优先查找已叫号的患者（QueueTicket状态为CALLED或IN_ROOM）
	const char* calledId = findCalledPatientIdByDoctor
	(doctorId);
	if (calledId != NULL) {
		return calledId;
	}
	// 回退方案：查找医生的历史看诊记录（复诊/随访场景）
	Patient* curr = sys->patientHead;
	while (curr != NULL) {
		ConsultationRecord* rec = curr->viewHead;
		while (rec != NULL) {
			if (strcmp(rec->doctorId, doctorId) == 0) {
				return curr->patientId; //返回第一个找到的患者编号
			}
			rec = rec->next;
		}
		curr = curr->next;
	}
	return NULL;
}


// 医生开具处方药品，返回动态分配的处方文本（调用者需free），未开药返回NULL
// 处方文本格式: drugId:genericName:qty;drugId:genericName:qty;...（不含价格）
static char* prescribeDrugsForPatient(HIS_System* sys, const char* doctorId, const char* patientId) {
	if (sys == NULL || doctorId == NULL || patientId == NULL) {
		return NULL;
	}
	Patient* patient = findPatientById(sys, patientId);
	if (patient == NULL) {
		printf(">>> 未找到患者信息。\n");
		return NULL;
	}

	printf("\n--- 可选药品列表 ---\n");
	// 以药品编号升序显示所有药品，方便医生选择
	sortDrugList(sys->drugDisplayHead, NULL, SORT_BY_ID, ORDER_ASC);
	displayAllDrugsDoc(sys);

	Drug* selDrug[100];
	int selQty[100];
	int itemCount = 0;

	while (1) {
		char drugId[ID_LEN];
		safeGetString(">>> 请输入药品编号(输入 -1 结束开药): ", drugId, ID_LEN);
		if (strcmp(drugId, "-1") == 0) {
			break;
		}

		Drug* drug = sys->drugHead;
		while (drug != NULL) {
			if (strcmp(drug->drugId, drugId) == 0) break;
			drug = drug->next;
		}
		if (drug == NULL) {
			printf(">>> 未找到该药品，请重新输入。\n");
			continue;
		}
		if (drug->stock <= 0) {
			printf(">>> 药品 %s 当前缺货，无法开具。\n", drug->genericName);
			continue;
		}

		printf("药品: %s (%s) | 单价: %.2f 元 | 库存: 有货\n",
			drug->genericName, drug->tradeName, drug->price);

		int qty = safeGetInt(">>> 请输入数量(正整数): ");
		if (qty <= 0) {
			printf(">>> 数量无效，请重新输入。\n");
			continue;
		}
		if (qty > drug->stock) {
			printf(">>> 库存不足！请减小数量或联系仓库增加库存。\n", drug->stock);
			continue;
		}

		selDrug[itemCount] = drug;
		selQty[itemCount] = qty;
		itemCount++;
		printf(">>> 已添加: %s %d盒 x %.2f = %.2f 元\n",
			drug->genericName, qty, drug->price, qty * drug->price);
	}

	if (itemCount == 0) {
		printf(">>> 未选择任何药品，取消开药。\n");
		return NULL;
	}

	// 生成处方文本
	char* result = (char*)malloc(512);
	if (result == NULL) return NULL;
	result[0] = '\0';
	for (int i = 0; i < itemCount; i++) {
		char item[128];
		sprintf(item, "%s:%s:%d%s",
			selDrug[i]->drugId, selDrug[i]->genericName, selQty[i],
			(i < itemCount - 1) ? ";" : "");
		strncat(result, item, 512 - strlen(result) - 1);
	}

	// 处方汇总（使用统一格式化函数）
	printf("\n========== 处方汇总 ===========\n");
	double totalCost = printPrescriptionDetail(sys, result);
	printf("处方总计: %.2f 元\n", totalCost);
	printf("患者余额: %.2f 元\n", getTotalBalance(patient));

	if (getTotalBalance(patient) < totalCost) {
		printf(">>> 警告: 余额不足，开具后余额将为 %.2f 元。\n", getTotalBalance(patient) - totalCost);
	}

	if (!confirmFunc("确认", "开具以上处方")) {
		free(result);
		printf(">>> 已取消处方。\n");
		return NULL;
	}

	// 扣库存、扣余额
	for (int i = 0; i < itemCount; i++) {
		selDrug[i]->stock -= selQty[i];
	}
	deductBalance(patient, totalCost);
	addHospitalRevenue(sys, totalCost);
	saveDrugSystemData(sys);
	printf(">>> 处方开具成功！已扣费 %.2f 元，当前余额: %.2f 元。\n",
		totalCost, getTotalBalance(patient));

	return result;
}

// 医生写入患者病例信息（诊断记录 + 开具处方）
void writeMedicalRecord(HIS_System* sys, const char* doctorId) {
	if (sys == NULL) {
		printf(">>> 系统未初始化，无法写入病例信息。\n");
		return;
	}
	if (doctorId == NULL) {
		printf(">>> 医生身份无效，无法写入病例信息。\n");
		return;
	}
	char patientId[ID_LEN];
	char details[512];
	char date[DATE_STR_LEN];

	int choice = -1;
	printf("\n--- 写入诊断记录 ---\n");
	printf("1. 选择当前正在看诊的患者\n");
	printf("2. 输入患者编号\n");
	printf("3. 处方开具\n");
	printf("-1. 取消写入诊断\n");
	choice = safeGetInt("请选择操作：");

	switch (choice) {
	case 1:
	case 3:
		{
			const char* autoId = findCurrentConsultationPatientId(sys, doctorId);
			if (autoId == NULL) {
				printf(">>> 当前没有正在看诊的患者。\n");
				return;
			}
			else
				printf(">>> 已自动选择当前正在看诊的患者编号: %s\n", autoId);
			strcpy(patientId, autoId);
		}
		if (choice == 3) {
			// 快速处方开具：跳过诊断记录填写，直接开药
			char* prescriptionText = prescribeDrugsForPatient(sys, doctorId, patientId);
			if (prescriptionText != NULL) {
				// 将处方作为独立 M 记录写入
				ConsultationRecord* medRec = (ConsultationRecord*)malloc(sizeof(ConsultationRecord));
				if (medRec != NULL) {
					memset(medRec, 0, sizeof(ConsultationRecord));
					generateRecordId(medRec->recordId, "M", patientId);
					medRec->record = REC_MED;
					strncpy(medRec->details, prescriptionText, sizeof(medRec->details) - 1);
					medRec->details[sizeof(medRec->details) - 1] = '\0';
					strncpy(medRec->date, getCurrentDateStr(), ID_LEN - 1);
					medRec->date[ID_LEN - 1] = '\0';
					strncpy(medRec->doctorId, doctorId, ID_LEN - 1);
					medRec->doctorId[ID_LEN - 1] = '\0';
					medRec->next = NULL;

					Patient* target = findPatientById(sys, patientId);
					if (target != NULL) {
						if (target->medHead == NULL) {
							target->medHead = medRec;
							target->currMedTail = medRec;
						} else {
							target->currMedTail->next = medRec;
							target->currMedTail = medRec;
						}
						savePatientsSystemData(sys);
						printf(">>> 处方已开具并保存！\n");
					}
				}
				free(prescriptionText);
			}
			markTicketAsInRoom(patientId, doctorId);
			return;
		}
		break;
	case 2:
		safeGetString(">>> 请输入患者编号(输入 -1 取消): ", patientId, ID_LEN);
		break;
	case -1:
		printf(">>> 已取消写入。\n");
		return;
	}


	safeGetString(">>> 请输入诊断记录内容(输入 -1 取消): ", details, 512);
	if (strcmp(details, "-1") == 0) {
		printf(">>> 已取消写入。\n");
		return;
	}

	if (TEST_SYSTEM_DEBUG) {
		printf(">>> 测试模式下，可以选择使用自定义日期或当前日期。\n");
		if (confirmFunc("使用", "自定义日期")) {
			safeGetString(">>> 请输入病例日期(YYYY-MM-DD): ", date, DATE_STR_LEN);
		}
		else {
			strcpy(date, getCurrentDateStr());
		}
	}
	else {
		strcpy(date, getCurrentDateStr());
	}

	if (!isValidDate(date)) {
		printf(">>> 日期格式无效，写入失败。\n");
		return;
	}

	// 询问医生是否开具处方
	if (confirmFunc("开具", "处方药品")) {
		char* prescriptionText = prescribeDrugsForPatient(sys, doctorId, patientId);
		if (prescriptionText != NULL) {
			// 将处方作为独立 M 记录写入患者链表
			ConsultationRecord* medRec = (ConsultationRecord*)malloc(sizeof(ConsultationRecord));
			if (medRec != NULL) {
				memset(medRec, 0, sizeof(ConsultationRecord));
				generateRecordId(medRec->recordId, "M", patientId);
				medRec->record = REC_MED;
				strncpy(medRec->details, prescriptionText, sizeof(medRec->details) - 1);
				medRec->details[sizeof(medRec->details) - 1] = '\0';
				strncpy(medRec->date, date, ID_LEN - 1);
				medRec->date[ID_LEN - 1] = '\0';
				strncpy(medRec->doctorId, doctorId, ID_LEN - 1);
				medRec->doctorId[ID_LEN - 1] = '\0';
				medRec->next = NULL;

				Patient* target = findPatientById(sys, patientId);
				if (target != NULL) {
					if (target->medHead == NULL) {
						target->medHead = medRec;
						target->currMedTail = medRec;
					} else {
						target->currMedTail->next = medRec;
						target->currMedTail = medRec;
					}
					// 处方独立于看诊记录，先保存以确保扣费/扣库存已落地
					savePatientsSystemData(sys);
				}
			}
			free(prescriptionText);
		}
		else {
			return;
		}
	}

	if (appendViewMedicalRecord(sys, patientId, doctorId, details, date)) {
		printf(">>> 诊断记录写入成功！\n");
		// 写入诊断后，将患者挂号单状态从CALLED推进为IN_ROOM，表示患者已进入诊室就诊
		markTicketAsInRoom(patientId, doctorId);
	}
	else {
		printf(">>> 诊断记录写入失败，请检查患者编号是否正确。\n");
	}
}

void issueExaminationOrder(HIS_System* sys, const char* doctorId) {
	if (sys == NULL || doctorId == NULL) {
		printf(">>> 医生身份无效，无法开具检查单。\n");
		return;
	}

	// 优先自动填充当前已叫号的患者
	const char* calledId = findCalledPatientIdByDoctor(doctorId);
	if (calledId != NULL) {
		Patient* p = findPatientById(sys, calledId);
		printf("\n>>> 当前叫号患者: %s (%s)\n", p ? p->name : "未知", calledId);
		if (confirmFunc("开具", "当前看诊患者的检查单")) {
			if (createExamOrder(sys, doctorId, calledId)) {
				markTicketAsInRoom(calledId, doctorId); //开具检查单后将患者挂号单状态推进为IN_ROOM，表示患者已进入诊室就诊
			}
			else{
				printf(">>> 检查单开具失败。\n");
			}
			return;
		}
	}
	else {
		printf(">>> 当前没有叫号患者！请手动输入患者编号。\n");
	}

// 手动指定患者
	if (!createExamOrder(sys, doctorId, NULL)) {
		printf(">>> 检查单开具失败。\n");
	}
}

int autoEndCurrentConsultation(HIS_System* sys, const char* doctorId) {
		const char* currentId = findCalledPatientIdByDoctor(doctorId);
		if (currentId == NULL) {
			printf(">>> 当前没有正在就诊的患者。\n");
			return -1;
		}
		Patient* patient = findPatientById(sys, currentId);
		printf(">>> 最后就诊患者: %s (%s)\n", patient ? patient->name : "未知", currentId);
		if (!confirmFunc("结束", "当前看诊")) {
			printf(">>> 已取消结束看诊。\n");
			return 0;
		}
		if (markTicketAsFinished(currentId, doctorId)) {
			printf(">>> 患者 %s 看诊已结束。\n", currentId);
			return 1;
		}
		// 状态不符时强制结束
		QueueTicket* ticket = findTicketByDoctorPatient(doctorId, currentId);
		if (ticket != NULL) {
			ticket->status = STATUS_FINISHED;
			printf(">>> 患者 %s 看诊已结束。\n", currentId);
			return 1;
		}
		printf(">>> 未找到对应的挂号记录，操作失败。\n");
		return -1;
}

// 医生结束看诊，将患者挂号单状态推进为FINISHED
void endConsultation(HIS_System* sys, const char* doctorId) {
	if (sys == NULL || doctorId == NULL) {
		printf(">>> 医生身份无效，无法执行结束看诊操作。\n");
		return;
	}

	while (1) {
		printf("\n--- 结束看诊 ---\n");
		printf("1. 自动结束最后就诊患者\n");
		printf("2. 手动输入患者编号\n");
		printf("0. 返回\n");

		int choice = safeGetInt("请选择操作: ");
		if (choice == 0) {
			return;
		}

		if (choice == 1) {
			int result = autoEndCurrentConsultation(sys, doctorId);
			if (result == 1 || result == 0) {
				return;
			}
			else if (result == -1) {
				continue;
			}
		}

		if (choice == 2) {
			char patientId[ID_LEN];
			safeGetString(">>> 请输入患者编号(输入 -1 取消): ", patientId, ID_LEN);
			if (strcmp(patientId, "-1") == 0) {
				printf(">>> 已取消结束看诊。\n");
				continue;
			}
			Patient* patient = findPatientById(sys, patientId);
			if (patient == NULL) {
				printf(">>> 未找到患者 %s 的信息。\n", patientId);
				continue;
			}
			if (!isPatientInRoomByDoctor(patientId, doctorId)) {
				printf(">>> 该患者当前不在就诊中状态，无法结束看诊。\n");
				continue;
			}
			if (!confirmFunc("结束", "该患者看诊")) {
				printf(">>> 已取消结束看诊。\n");
				continue;
			}
			if (markTicketAsFinished(patientId, doctorId)) {
				printf(">>> 患者 %s 看诊已结束。\n", patient->name);
				return;
			}
			QueueTicket* ticket = findTicketByDoctorPatient(doctorId, patientId);
			if (ticket != NULL) {
				ticket->status = STATUS_FINISHED;
				printf(">>> 患者 %s 看诊已结束。\n", patient->name);
				return;
			}
			printf(">>> 未找到对应的挂号记录，操作失败。\n");
			continue;
		}

		printf(">>> 无效选择，请重试。\n");
	}
	return;
}

// 医生查看就诊历史记录，遍历所有患者列出本医生曾看诊过的患者，支持选择查看详情
void viewConsultationHistory(HIS_System* sys, const char* doctorId) {
	if (sys == NULL || doctorId == NULL) {
		printf(">>> 医生身份无效，无法查看就诊历史。\n");
		return;
	}

	// 收集所有本医生看诊过的患者及其最近看诊日期
	typedef struct {
		Patient* patient;
		char lastDate[DATE_STR_LEN];
		int viewCount;
	} HistoryEntry;
	HistoryEntry entries[256];
	int entryCount = 0;

	Patient* curr = sys->patientHead;
	while (curr != NULL && entryCount < 256) {
		ConsultationRecord* rec = curr->viewHead;
		bool found = false;
		char latestDate[DATE_STR_LEN] = "";
		int count = 0;
		while (rec != NULL) {
			if (strcmp(rec->doctorId, doctorId) == 0) {
				count++;
				if (strlen(latestDate) == 0 || strcmp(rec->date, latestDate) > 0) {
					strcpy(latestDate, rec->date);
				}
				found = true;
			}
			rec = rec->next;
		}
		if (found) {
			entries[entryCount].patient = curr;
			strcpy(entries[entryCount].lastDate, latestDate);
			entries[entryCount].viewCount = count;
			entryCount++;
		}
		curr = curr->next;
	}

	printf("\n--- 就诊历史记录 ---\n");

	if (entryCount == 0) {
		printf(">>> 暂无就诊历史记录。\n");
		pressEnterToContinue();
		return;
	}

	// 按最近看诊日期降序排序（最近的在前面）
	for (int i = 0; i < entryCount - 1; ++i) {
		for (int j = i + 1; j < entryCount; ++j) {
			if (strcmp(entries[i].lastDate, entries[j].lastDate) < 0) {
				HistoryEntry tmp = entries[i];
				entries[i] = entries[j];
				entries[j] = tmp;
			}
		}
	}

	char buffer[256];
	printf("----------------------------------------------------\n");
	// 表头
	printFormattedStr("序号", 6);
	printFormattedStr("患者编号", 12);
	printFormattedStr("姓名", 12);
	printFormattedStr("最近看诊日期", 14);
	printFormattedStr("看诊次数", 8);
	printf("\n");
	printf("----------------------------------------------------\n");
	// 数据行
	for (int i = 0; i < entryCount; ++i) {
		sprintf(buffer, "#%d", i + 1);
		printFormattedStr(buffer, 6);
		printFormattedStr(entries[i].patient->patientId, 12);
		printFormattedStr(entries[i].patient->name, 12);
		printFormattedStr(entries[i].lastDate, 14);
		sprintf(buffer, "%d", entries[i].viewCount);
		printFormattedStr(buffer, 8);
		printf("\n");
	}
	printf("----------------------------------------------------\n");
	printf("共 %d 条就诊历史。\n", entryCount);

	printf("输入左侧序号可查看患者详细病历，输入 0 返回上级菜单。\n");
	int sel = safeGetInt("请选择: ");
	if (sel > 0 && sel <= entryCount) {
		viewMedicalRecordPat(sys, entries[sel - 1].patient->patientId);
		// viewMedicalRecordPat 末尾已有 pressEnterToContinue，此处不再重复
		return;
	}
	if (sel != 0) {
		printf(">>> 无效选择，返回上级菜单。\n");
	}
	pressEnterToContinue();
}


// 更新住院记录的出院日期与时长
bool updateStayRecordEnd(HIS_System* sys, const char* patientId, const char* wardId, const char* endDate, const char* duration) {
	if (sys == NULL || patientId == NULL || wardId == NULL || endDate == NULL || duration == NULL) return false;
	Patient* p = findPatientById(sys, patientId);
	if (p == NULL) return false;
	StayRecord* s = p->stayHead;
	while (s != NULL) {
		if (strcmp(s->wardId, wardId) == 0 && (strcmp(s->endDate, "未出院") == 0 || strcmp(s->endDate, "待出院") == 0)) {
			strncpy(s->endDate, endDate, ID_LEN - 1);
			s->endDate[ID_LEN - 1] = '\0';
			strncpy(s->duration, duration, ID_LEN - 1);
			s->duration[ID_LEN - 1] = '\0';
			return true;
		}
		s = s->next;
	}
	return false;
}

// 执行出院核心流程（释放床位、更新记录、回写病历、保存数据）
void executeDischargePatient(HIS_System* sys, const char* patientId, const char* wardId, const char* bedId) {
	if (sys == NULL || patientId == NULL || wardId == NULL || bedId == NULL) return;

	loadWardSystemData(sys);
	loadPatientsSystemData(sys);

	// 查找活动住院记录（可能已被医生设置 endDate，也可能仍为"未出院"/"待出院"）
	Patient* p = findPatientById(sys, patientId);
	const char* startDate = NULL;
	StayRecord* activeStay = NULL;
	if (p != NULL) {
		StayRecord* s = p->stayHead;
		while (s != NULL) {
			if (strcmp(s->wardId, wardId) == 0 && (strcmp(s->endDate, "未出院") == 0 || strcmp(s->endDate, "待出院") == 0)) {
				activeStay = s;
				startDate = s->startDate;
				break;
			}
			s = s->next;
		}
		// 若未找到"未出院"/"待出院"记录，查找已设置endDate但尚未释放床位的活跃记录
		if (activeStay == NULL) {
			s = p->stayHead;
			while (s != NULL) {
				if (strcmp(s->wardId, wardId) == 0 && s->dischargeApproved == 1
					&& strcmp(s->endDate, "未出院") != 0 && strcmp(s->endDate, "待出院") != 0) {
					activeStay = s;
					startDate = s->startDate;
					// endDate和duration已由医生设置，无需再调用updateStayRecordEnd
					break;
				}
				s = s->next;
			}
		}
	}

	const char* today = getCurrentDateStr();
	char duration[ID_LEN];
	if (startDate != NULL) {
		int days = daysBetweenDates(startDate, today);
		sprintf(duration, "%d天", days);
	} else {
		strcpy(duration, "待定");
	}

	// 1. 释放床位
	freeBed(sys, wardId, bedId);

	// 2. 更新住院记录（仅当endDate尚未由医生设置时才更新）
	if (activeStay != NULL && (strcmp(activeStay->endDate, "未出院") == 0 || strcmp(activeStay->endDate, "待出院") == 0)) {
		updateStayRecordEnd(sys, patientId, wardId, today, duration);
	}

	// 3. 保存所有受影响的数据
	saveWardSystemData(sys);
	savePatientsSystemData(sys);

	if(TEST_SYSTEM_DEBUG)
	printf(">>> 患者 %s 出院流程完成（病床 %s:%s 已释放）。\n", patientId, wardId, bedId);
}

// 患者端办理出院手续
void patientDischargeCheckout(HIS_System* sys, const char* patientId) {
	if (sys == NULL || patientId == NULL || patientId[0] == '\0') {
		printf(">>> 您尚未登录，请先登录后再办理出院手续！\n");
		return;
	}
	if (!isPatientLoggedIn()) {
		printf(">>> 您尚未登录，请先登录后再办理出院手续！\n");
		return;
	}

	loadWardSystemData(sys);
	loadPatientsSystemData(sys);

	// 查找患者当前住院的病房和床位
	Ward* currentWard = findPatientWard(sys, patientId);
	if (currentWard == NULL) {
		printf(">>> 您当前未住院，无需办理出院手续。\n");
		pressEnterToContinue();
		return;
	}

	// 找到对应的床位
	Bed* currentBed = NULL;
	Bed* b = currentWard->bedListHead;
	while (b != NULL) {
		if (b->isOccupied && strcmp(b->patient, patientId) == 0) {
			currentBed = b;
			break;
		}
		b = b->next;
	}
	if (currentBed == NULL) {
		printf(">>> 未找到当前床位信息，请联系管理员。\n");
		pressEnterToContinue();
		return;
	}

	// 查找住院记录，获取入院日期
	Patient* p = findPatientById(sys, patientId);
	if (p == NULL) {
		printf(">>> 患者信息异常，请联系管理员。\n");
		pressEnterToContinue();
		return;
	}

	// 查找活跃住院记录：优先"未出院"/"待出院"，其次已由医生批准且endDate已被设置
	StayRecord* activeStay = NULL;
	StayRecord* s = p->stayHead;
	while (s != NULL) {
		if (strcmp(s->wardId, currentWard->wardId) == 0
			&& (strcmp(s->endDate, "未出院") == 0 || strcmp(s->endDate, "待出院") == 0)) {
			activeStay = s;
			break;
		}
		s = s->next;
	}
	if (activeStay == NULL) {
		// 查找医生已设置endDate且已批准出院的记录
		s = p->stayHead;
		while (s != NULL) {
			if (strcmp(s->wardId, currentWard->wardId) == 0 && s->dischargeApproved == 1
				&& strcmp(s->endDate, "未出院") != 0 && strcmp(s->endDate, "待出院") != 0) {
				activeStay = s;
				break;
			}
			s = s->next;
		}
	}
	if (activeStay == NULL) {
		printf(">>> 未找到住院记录，请联系管理员。\n");
		pressEnterToContinue();
		return;
	}
	if (activeStay->dischargeApproved == 0) {
		printf(">>> 未获医生出院许可，暂无法办理出院手续。\n");
		pressEnterToContinue();
		return;
	}

	double dailyPrice = currentWard->price;
	double deposit = 7.0 * dailyPrice;	// 入院时已预扣7天押金（预收医疗款）

	// 12:00规则：若当前时间在12:00之前，出院当天不计费
	int adjust12 = 0;
	{
		char* timeStr = getCurrentTimeStr();
		int hour = 0;
		if (timeStr != NULL && sscanf(timeStr, "%d:", &hour) == 1) {
			if (hour < 12) adjust12 = 1;
		}
	}

	// 计算有效计费截止日（endDate 扣除12:00调整）
	char effectiveEnd[DATE_STR_LEN];
	if (adjust12) {
		addDaysToDate(activeStay->endDate, -1, effectiveEnd, DATE_STR_LEN);
	} else {
		strncpy(effectiveEnd, activeStay->endDate, DATE_STR_LEN - 1);
		effectiveEnd[DATE_STR_LEN - 1] = '\0';
	}

	int days = daysBetweenDates(activeStay->startDate, effectiveEnd);
	if (days < 1) days = 1;

	// 第一步：结算从上次计费日到有效截止日的每日费用（第8天起，避免与chargeAllInpatientsDaily重复）
	{
		const char* chargeFrom;
		if (activeStay->isChargeDate[0] != '\0') {
			chargeFrom = activeStay->isChargeDate;
		} else {
			char depositEnd[DATE_STR_LEN];
			addDaysToDate(activeStay->startDate, 7, depositEnd, DATE_STR_LEN);
			strncpy(activeStay->isChargeDate, depositEnd, DATE_STR_LEN - 1);
			activeStay->isChargeDate[DATE_STR_LEN - 1] = '\0';
			chargeFrom = activeStay->isChargeDate;
		}
		int chargeDays = daysBetweenDates(chargeFrom, effectiveEnd);
		if (chargeDays > 0) {
			double dailyChargeTotal = chargeDays * dailyPrice;
			deductBalance(p, dailyChargeTotal);
			addHospitalRevenue(sys, dailyChargeTotal);
			strncpy(activeStay->isChargeDate, effectiveEnd, DATE_STR_LEN - 1);
			activeStay->isChargeDate[DATE_STR_LEN - 1] = '\0';
			if (TEST_SYSTEM_DEBUG)
				printf(">>> 每日计费结算: %s → %s, %d天 × %.2f元 = %.2f 元\n",
					chargeFrom, effectiveEnd, chargeDays, dailyPrice, dailyChargeTotal);
		}
	}

	// 第二步：押金结算（预收医疗款，覆盖头7天）
	double actualCost = days * dailyPrice;
	double depositCost = (days < 7) ? actualCost : deposit;

	// 显示出院结算信息
	printf("\n========== 出院结算 ==========\n");
	printf("病房编号: %s\n", currentWard->wardId);
	printf("病房类型: %s\n", wardTypeToStr(currentWard->type));
	printf("所属科室: %s\n", currentWard->department);
	printf("床位编号: %s\n", currentBed->bedId);
	printf("入院日期: %s\n", activeStay->startDate);
	printf("出院日期: %s\n", activeStay->endDate);
	printf("计费截止: %s\n", effectiveEnd);
	printf("实际住院: %d 天\n", days);
	printf("每日价格: %.2f 元\n", dailyPrice);
	printf("实际费用: %.2f 元\n", actualCost);
	printf("已付押金: %.2f 元 (7天预收医疗款)\n", deposit);
	printf("==============================\n");

	if (!confirmFunc("缴费确认", "确认出院结算")) {
		printf(">>> 已取消出院结算。\n");
		pressEnterToContinue();
		return;
	}

	// 押金清算（预收医疗款多退少补）
	if (days < 7) {
		// 实际天数少于7天，退还剩余预收医疗款
		double refund = deposit - depositCost;
		addRealBalance(p, refund);
		addHospitalExpenditure(sys, refund);  // 预收医疗款退回
		printf(">>> 住院费 %.2f 元已由押金抵扣，剩余预收医疗款 %.2f 元已退回。\n", depositCost, refund);
	} else {
		// 押金全额抵扣头7天，第8天起已在每日计费中结算
		printf(">>> 住院费（头7天）已由押金全额抵扣 %.2f 元。\n", deposit);
	}

	printf(">>> 缴费成功！当前余额: %.2f 元。\n", getTotalBalance(p));

	// 确认出院
	if (!confirmFunc("出院确认", "完成出院手续")) {
		printf(">>> 已取消出院结算。\n");
		pressEnterToContinue();
		return;
	}

	// 写入 isChargeDate（患者最终缴费办结日期）
	{
		char chargeDate[DATE_STR_LEN];
		if (TEST_SYSTEM_DEBUG) {
			if (confirmFunc("使用", "自定义缴费办结日期")) {
				safeGetString(">>> 请输入缴费办结日期(YYYY-MM-DD，输入 -1 取消): ", chargeDate, DATE_STR_LEN);
			} else {
				strcpy(chargeDate, getCurrentDateStr());
			}
			if (strcmp(chargeDate, "-1") == 0) {
				printf(">>> 已取消。\n");
				pressEnterToContinue();
				return;
			}

		} 
		else {
			strcpy(chargeDate, getCurrentDateStr());
		}

		if (!isValidDate(chargeDate)) {
			printf(">>> 日期格式无效，操作取消。\n");
			pressEnterToContinue();
			return;
		}
		if (!confirmFunc("确认", "缴费办结日期")) {
			printf(">>> 已取消。\n");
			pressEnterToContinue();
			return;
		}

		// 校验缴费日期不得早于出院日期
		if (strcmp(chargeDate, activeStay->endDate) < 0) {
			printf(">>> 缴费日期不得早于出院日期 (%s)，操作失败。\n", activeStay->endDate);
			pressEnterToContinue();
			return;
		}
		strncpy(activeStay->isChargeDate, chargeDate, DATE_STR_LEN - 1);
		activeStay->isChargeDate[DATE_STR_LEN - 1] = '\0';
	}

	// 执行出院核心流程
	char wardId[ID_LEN], bedId[BED_ID_LEN];
	strcpy(wardId, currentWard->wardId);
	strcpy(bedId, currentBed->bedId);

	savePatientsSystemData(sys);
	executeDischargePatient(sys, patientId, wardId, bedId);

	// 更新叫号状态
	const char* doctorId = activeStay->doctorId;
	if (doctorId != NULL && doctorId[0] != '\0') {
		markTicketAsInRoom(patientId, doctorId);
	}
	printf(">>> 出院手续办理完毕，祝您健康！\n");
	pressEnterToContinue();
}

// 患者个人信息查询与修改菜单
void patientInfoMenu(HIS_System* sys, const char* patientId) {
	if (sys == NULL) {
		printf(">>> 系统未初始化，无法查看患者信息。\n");
		return;
	}
	if (!isPatientLoggedIn() || patientId == NULL || patientId[0] == '\0') {
		printf(">>> 您尚未登录，请先登录后再查看个人信息！\n");
		return;
	}

	loadPatientsSystemData(sys);
	Patient* patient = findPatientById(sys, patientId);
	if (patient == NULL) {
		printf(">>> 未找到患者信息，请联系管理员。\n");
		return;
	}

	int choice;
	while (1) {
		printf("\n========== 患者个人信息 ==========\n");
		printf("患者编号: %s\n", patient->patientId);
		printf("患者姓名: %s\n", patient->name);
		printf("患者性别: %s\n", patient->gender);
		printf("患者年龄: %d\n", patient->age);
		printf("患者类型: %s\n",
			patient->type == PATIENT_VIP ? "VIP" :
			patient->type == PATIENT_EMERGENCY ? "急诊" : "普通");
		printf("账户总余额: %.2f 元\n", getTotalBalance(patient));
		printf("联系电话: %s\n", patient->phone);
		printf("身份证号: %s\n", patient->idCard);
		printf("==================================\n\n");
		printf("1. 修改联系电话\n");
		printf("2. 修改年龄\n");
		printf("3. 修改患者类型(普通/VIP)\n");
		printf("0. 返回上级菜单\n");
		choice = safeGetInt(">>> 请选择操作: ");

		switch (choice) {
		case 1: {
			char newPhone[ID_LEN];
			safeGetString("请输入新的联系电话(输入 -1 取消): ", newPhone, ID_LEN);
			if (strcmp(newPhone, "-1") == 0) {
				printf(">>> 已取消修改。\n");
				break;
			}
			if (confirmFunc("修改", "联系电话")) {
				strcpy(patient->phone, newPhone);
				savePatientsSystemData(sys);
				printf(">>> 联系电话修改成功！\n");
			} else {
				printf(">>> 已取消修改。\n");
			}
			break;
		}
		case 2: {
			int newAge = safeGetInt("请输入新的年龄(输入 -1 取消): ");
			if (newAge == -1) {
				printf(">>> 已取消修改。\n");
				break;
			}
			if (newAge <= 0) {
				printf(">>> 年龄无效，请重新操作。\n");
				break;
			}
			if (confirmFunc("修改", "年龄")) {
				patient->age = newAge;
				savePatientsSystemData(sys);
				printf(">>> 年龄修改成功！\n");
			} else {
				printf(">>> 已取消修改。\n");
			}
			break;
		}
		case 3: {
			if (patient->type == PATIENT_EMERGENCY) {
				printf(">>> 急诊类型仅限医生端修改，暂无法变更。\n");
				break;
			}
			if (patient->type == PATIENT_GENERAL) {
				printf(">>> 提示：升级为 VIP 将扣除 500 元余额，并享受相关权益(优先病房)。\n");
				if (!confirmFunc("升级", "普通患者转为 VIP")) {
					printf(">>> 已取消修改。\n");
					break;
				}
				deductBalance(patient, 500.0);
				addHospitalRevenue(sys, 500.0);
				patient->type = PATIENT_VIP;
				savePatientsSystemData(sys);
				printf(">>> 已升级为 VIP，已扣费 500 元，当前余额: %.2f 元。\n", getTotalBalance(patient));
			} else if (patient->type == PATIENT_VIP) {
				printf(">>> 提示：VIP 转普通将失去相关权益。\n");
				if (!confirmFunc("确认", "VIP 转普通")) {
					printf(">>> 已取消修改。\n");
					break;
				}
				printf(">>> 二次确认：继续将 VIP 转为普通将无法恢复 VIP 权益。\n");
				if (!confirmFunc("确认", "继续将 VIP 转为普通")) {
					printf(">>> 已取消修改。\n");
					break;
				}
				patient->type = PATIENT_GENERAL;
				savePatientsSystemData(sys);
				printf(">>> 患者类型已调整为普通。\n");
			}
			break;
		}
		case 0:
			return;
		default:
			printf(">>> 无效选择，请重试。\n");
		}
	}
}

// 对所有在住患者每日自动扣费
void chargeAllInpatientsDaily(HIS_System* sys) {
	if (sys == NULL) return;

	const char* today = getCurrentDateStr();
	int totalCharged = 0;
	double totalAmount = 0.0;

	Patient* p = sys->patientHead;
	while (p != NULL) {
		StayRecord* s = p->stayHead;
		while (s != NULL) {
			if (strcmp(s->endDate, "未出院") == 0) {
				// 确定计费起始日：isChargeDate有值则从上次计费日起，否则从入院7天后起（押金覆盖前7天）
				const char* chargeFrom;
				if (s->isChargeDate[0] != '\0') {
					chargeFrom = s->isChargeDate;
				} else {
					char depositEnd[DATE_STR_LEN];
					addDaysToDate(s->startDate, 7, depositEnd, DATE_STR_LEN);
					strncpy(s->isChargeDate, depositEnd, DATE_STR_LEN - 1);
					s->isChargeDate[DATE_STR_LEN - 1] = '\0';
					chargeFrom = s->isChargeDate;
				}
				int daysSince = daysBetweenDates(chargeFrom, today);
				if (daysSince <= 0) { s = s->next; continue; }

				Ward* w = sys->wardHead;
				while (w != NULL) {
					if (strcmp(w->wardId, s->wardId) == 0) break;
					w = w->next;
				}
				if (w != NULL) {
					double charge = daysSince * w->price;
					deductBalance(p, charge);
					addHospitalRevenue(sys, charge);
					totalCharged++;
					totalAmount += charge;
					strncpy(s->isChargeDate, today, DATE_STR_LEN - 1);
					s->isChargeDate[DATE_STR_LEN - 1] = '\0';
				}
			}
			s = s->next;
		}
		p = p->next;
	}

	if (TEST_SYSTEM_DEBUG)
		printf(">>> 住院每日计费完成: %d 条记录，合计 %.2f 元。\n", totalCharged, totalAmount);
}

// 余额辅助函数：获取总余额
double getTotalBalance(const Patient* patient) {
	if (patient == NULL) return 0.0;
	return patient->realBalance + patient->bonusBalance;
}

// 余额辅助函数：增加实际余额
void addRealBalance(Patient* patient, double amount) {
	if (patient != NULL && amount > 0)
		patient->realBalance += amount;
}

// 余额辅助函数：增加赠送余额
void addBonusBalance(Patient* patient, double amount) {
	if (patient != NULL && amount > 0)
		patient->bonusBalance += amount;
}

// 余额辅助函数：扣费（优先扣bonusBalance，不足再扣realBalance）
double deductBalance(Patient* patient, double amount) {
	if (patient == NULL || amount <= 0) return 0.0;
	double remaining = amount;
	if (patient->bonusBalance > 0) {
		double fromBonus = (patient->bonusBalance >= remaining) ? remaining : patient->bonusBalance;
		patient->bonusBalance -= fromBonus;
		remaining -= fromBonus;
	}
	if (remaining > 0) {
		patient->realBalance -= remaining;
	}
	return amount;
}

// 指定患者的充值菜单
void patientRechargeMenuForPatient(HIS_System* sys, Patient* patient) {
	if (sys == NULL || patient == NULL) return;

	while (1) {
		printf("\n============== 余额充值 ==============\n");
		printf("当前患者: %s  总余额: %.2f 元\n",
			patient->name, getTotalBalance(patient));
		printf("--------------------------------------\n");

		const int w = 24;
		printFormattedStr("[1] 100元", w);
		printFormattedStr("[2] 200元", w);
		printf("\n");

		printFormattedStr("[3] 500元", w);
		printFormattedStr("[4] 1000元(赠80元)", w);
		printf("\n");

		printFormattedStr("[5] 2000元(赠200元)", w);
		printFormattedStr("[6] 5000元(赠600元)", w);
		printf("\n");

		printFormattedStr("[7] 小额任意(<1000)", w);
		printFormattedStr("[8] 大额任意(>=1000)(可享受8%-12%赠额)", w);
		printf("\n");

		printFormattedStr("0. 返回", w);
		printf("\n");
		printf("--------------------------------------\n");

		int choice = safeGetInt("请选择充值选项: ");
		double amount = 0.0;
		double bonus = 0.0;
		bool valid = true;

		switch (choice) {
		case 0: return;
		case 1: amount = 100.0; break;
		case 2: amount = 200.0; break;
		case 3: amount = 500.0;	break;
		case 4: amount = 1000.0; bonus = 80.0; break;
		case 5: amount = 2000.0; bonus = 200.0; break;
		case 6: amount = 5000.0; bonus = 600.0; break;
		case 7:
			amount = safeGetDouble("请输入充值金额(1-999元，-1取消): ");
			if (amount <= -1.0 + 1e-6 && amount >= -1.0 - 1e-6) { valid = false; break; }
			if (amount < 1.0 || amount >= 1000.0) {
				printf(">>> 小额充值范围为 1-999 元，请重试。\n");
				valid = false;
			}
			break;
		case 8:
			amount = safeGetDouble("请输入充值金额(>=1000元，-1取消): ");
			if (amount <= -1.0 + 1e-6 && amount >= -1.0 - 1e-6) { valid = false; break; }
			if (amount < 1000.0) {
				printf(">>> 大额充值最低 1000 元，请重试。\n");
				valid = false;
			} else if (amount < 1500.0)       { bonus = amount * 0.08; }
			else if (amount < 2000.0)  { bonus = amount * 0.09; }
			else if (amount < 3500.0)  { bonus = amount * 0.10; }
			else if (amount < 5000.0)  { bonus = amount * 0.11; }
			else                       { bonus = amount * 0.12; }
			break;
		default:
			printf(">>> 无效选择，请重试。\n");
			valid = false;
		}

		if (!valid) continue;

		double totalAdd = amount + bonus;
		printf("\n>>> 充值金额: %.2f 元, 合计到账: %.2f 元\n", amount, totalAdd);

		if (!confirmFunc("确认", "以上充值信息")) {
			printf(">>> 已取消充值。\n");
			continue;
		}

		addRealBalance(patient, amount);
		if (bonus > 0) addBonusBalance(patient, bonus);
		savePatientsSystemData(sys);
		printf(">>> 充值成功！当前总余额: %.2f 元。\n", getTotalBalance(patient));
	}
}

// 余额充值入口（需先登录）
void patientRechargeMenu(HIS_System* sys) {
	if (!isPatientLoggedIn()) {
		printf(">>> 请先登录后再进行余额充值！\n");
		return;
	}
	patientRechargeMenuForPatient(sys, getCurrentPatientNode());
}