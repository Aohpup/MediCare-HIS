#define _CRT_SECURE_NO_WARNINGS
#include"HIS_System.h"
#include"QueueManage.h"
#include"DrugFileManage.h"
#include"DocterFileManage.h"
#include"DepartmentFileManage.h"
#include"WardFileManage.h"
#include"PatientManage.h"
#include"PatientFileManage.h"
#include"DepartmentManage.h"
#include"DocterManage.h"
#include"InputUtils.h"
#include"ConfirmFunc.h"
#include"string.h"

bool is_Patient_Logged_In = false;	//标记患者是否已登录

void loadFileAllData(HIS_System* sys) {
	if(TEST_SYSTEM_DEBUG)
		printf(">>> 正在加载所有系统数据...\n");
	loadDepartmentSystemData(sys);
	loadDoctorSystemData(sys);
	loadDrugSystemData(sys);
	loadWardSystemData(sys);
	loadPatientsSystemData(sys);
}

static Patient* currentPatient = NULL;

static Patient* getCurrentPatient(void) {
	return currentPatient;
}

// 设置当前登录的患者信息指针，并根据指针是否为NULL更新登录状态标志
static void setCurrentPatient(Patient* patient) {
	currentPatient = patient;
	is_Patient_Logged_In = (patient != NULL);	//如果传入的患者指针不为NULL，说明患者已登录；如果为NULL，说明患者已退出登录
}

// 根据医生编号在系统中查找对应的医生信息
// 区别于Docter文件内的findDoctorById函数，这里可以返回医生信息指针，供挂号时使用；
// 而Docter文件内的函数主要用于验证医生编号是否存在，无法返回医生信息指针
static Docter* findDoctorById(HIS_System* sys, const char* doctorId) {
	Docter* curr = sys->docHead;
	while (curr != NULL) {
		if (strcmp(curr->docterId, doctorId) == 0) {
			return curr;
		}
		curr = curr->next;
	}
	return NULL;
}

// 打印所有可选的挂号预约时间段列表，并提示患者输入选择的时段编号，返回对应的TimeSlot枚举值
static TimeSlot inputTimeSlotChoice(void) {
	printAllTimeSlots();
	int slotNo = safeGetInt(">>> 请选择时段编号(1-13): ");
	if (slotNo < 1 || slotNo > SLOT_COUNT) {
		return SLOT_INVALID;
	}
	return (TimeSlot)slotNo;
}

// 将新的挂号记录追加到患者的挂号记录链表末尾
static void appendRegistrationRecord(Patient* patient, const char* doctorId, const char* department, const char* date, TimeSlot slot) {
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

		safeGetString("请输入患者姓名: ", newPatient->name, STR_LEN);
		if (strcmp(newPatient->name, "-1") == 0) {
			cancelFlag = true;
			free(newPatient);
			printf(">>> 已取消患者注册！正在返回操作菜单...\n");
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
		}

		newPatient->type = safeGetInt("请输入患者类别 (0-普通, 1-VIP, 2-急诊): ");
		if (strcmp(newPatient->name, "-1") == 0) {
			cancelFlag = true;
			free(newPatient);
			printf(">>> 已取消患者注册！正在返回操作菜单...\n");
		}

		if (!cancelFlag) {
			// 初始化患者记录链表头和末尾指针
			newPatient->regHead = NULL;
			newPatient->viewHead = NULL;
			newPatient->examHead = NULL;
			newPatient->stayHead = NULL;
			newPatient->currRegTail = NULL;
			newPatient->currViewTail = NULL;
			newPatient->currExamTail = NULL;
			newPatient->currStayTail = NULL;
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
			printf(">>> 已登录患者账户 %s，无需重复登录！正在返回患者服务台...\n", getCurrentPatient()->name);
			return;
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
				printf(">>> 登录成功！欢迎您，%s！正在返回患者服务台...\n", curr->name);
				//bool firstLogin = !is_Patient_Logged_In;	//如果之前没有登录过，说明这是第一次登录
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
		printf("3. 预约签到排队\n");
		printf("0. 返回上一级菜单\n");
		int choice = safeGetInt("请选择操作: ");
		if (choice == 0) {
			return;
		}

		if (choice == 3) {
			char doctorId[ID_LEN];
			safeGetString(">>> 请输入预约医生编号: ", doctorId, ID_LEN);
			TimeSlot slot = inputTimeSlotChoice();
			if (slot == SLOT_INVALID) {
				printf(">>> 时段编号无效。\n");
				continue;
			}
			char signTime[TIME_STR_LEN];
				if(TEST_SYSTEM_DEBUG) {
					if (confirmFunc("使用", "自定义时间")) 
						setTestTime(signTime);
					else
						strcpy(signTime, getCurrentTimeStr());
				}
				else
					strcpy(signTime, getCurrentTimeStr());

				//检查当前患者是否有符合条件的预约挂号记录，并且已经签到成功，如果满足条件则打印当前时段的排队情况
				char usingDate[DATE_STR_LEN];
				if (TEST_SYSTEM_DEBUG) {
					printf(">>> 测试模式下，可以选择使用自定义签到时间或当前时间。\n");
					if (confirmFunc("使用", "自定义日期")) {
						safeGetString(">>> 请输入自定义日期(YYYY-MM-DD): ", usingDate, DATE_STR_LEN);
					}
				}
				else
					strcpy(usingDate, getCurrentDateStr());
			if (checkInQueueTicket(getCurrentPatient()->patientId, doctorId, usingDate, slot, signTime)) {
				printSlotQueue(doctorId, usingDate, slot);
			}
			continue;
		}

		char doctorId[ID_LEN];
		safeGetString(">>> 请输入医生编号: ", doctorId, ID_LEN);
		Docter* doctor = findDoctorById(sys, doctorId);
		if (doctor == NULL) {
			printf(">>> 未找到该医生，请检查输入。\n");
			continue;
		}
		TimeSlot slot = inputTimeSlotChoice();
		if (slot <= SLOT_INVALID || slot > SLOT_COUNT) {
			printf(">>> 时段编号无效。\n");
			continue;
		}

		char date[DATE_STR_LEN];
		if (choice == 1) {
			safeGetString(">>> 请输入预约日期(YYYY-MM-DD): ", date, DATE_STR_LEN);
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

		//尝试挂号，如果挂号成功则追加挂号记录并打印挂号成功信息和当前时段剩余号源数量；如果是当场挂号且挂号成功，则自动进行签到并打印相关信息
		if (bookQueueTicket(getCurrentPatient(), doctor, date, slot, choice == 2)) {
			appendRegistrationRecord(getCurrentPatient(), doctor->docterId, doctor->department, date, slot);
			printf(">>> 挂号成功：医生[%s] 日期[%s] 时段[%s]。\n", doctor->docterName, date, slot_names[slot - 1]);
			printf(">>> 当前时段剩余号源：%d\n", getDoctorSlotRemain(doctor->docterId, date, slot));
			if (choice == 2) {
				char signTime[TIME_STR_LEN];
					if(TEST_SYSTEM_DEBUG) {
						if (confirmFunc("使用", "自定义时间")) {
							safeGetString(">>> 请输入自定义时间(HH:MM): ", signTime, TIME_STR_LEN);
						}
						else
							strcpy(signTime, getCurrentTimeStr());
					}
					else
						strcpy(signTime, getCurrentTimeStr());
				if (checkInQueueTicket(getCurrentPatient()->patientId, doctor->docterId, date, slot, signTime)) {
					printf(">>> 当场挂号已自动签到，请及时就诊。\n");
					printSlotQueue(doctor->docterId, date, slot);
				}
			}
		}
	}
}
