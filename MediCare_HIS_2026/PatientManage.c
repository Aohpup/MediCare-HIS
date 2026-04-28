#define _CRT_SECURE_NO_WARNINGS
#include"HIS_System.h"
#include"DrugFileManage.h"
#include"doctorManage.h"
#include"doctorFileManage.h"
#include"DepartmentFileManage.h"
#include"DepartmentManage.h"
#include"WardFileManage.h"
#include"PatientManage.h"
#include"PatientFileManage.h"
#include"ExamManage.h"
#include"QueueManage.h"
#include"PrintFormattedStr.h"
#include"InputUtils.h"
#include"ConfirmFunc.h"
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
	printf("记录编号: %s\n", rec->recordId);
	printf("开始日期: %s\n", rec->startDate);
	printf("住院时长: %s\n", rec->duration);
	printf("结束日期: %s\n", rec->endDate);
	printf("医生编号: %s\n", rec->doctorId);
	printf("病房编号: %s\n", rec->wardId);
	printf("记录内容: %s\n", rec->details);
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
	int slotNo = safeGetInt(">>> 请选择时段编号(1-13): ");
	if (slotNo < 1 || slotNo > SLOT_COUNT) {
		return SLOT_INVALID;
	}
	return (TimeSlot)slotNo;
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

		newPatient->type = safeGetInt("请输入患者类别 (0-普通, 1-VIP, 2-急诊): ");
		if (strcmp(newPatient->name, "-1") == 0) {
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
			newPatient->currRegTail = NULL;
			newPatient->currViewTail = NULL;
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
					printf(">>> 测试模式下，可以选择使用自定义当前时间。\n");
					if (confirmFunc("使用", "自定义日期")) {
						safeGetString(">>> 请输入自定义日期(YYYY-MM-DD): ", usingDate, DATE_STR_LEN);
					}
				}
				else
					strcpy(usingDate, getCurrentDateStr());
			if (checkInQueueTicket(getCurrentPatientNode()->patientId, doctorId, usingDate, slot, signTime)) {
				printSlotQueue(doctorId, usingDate, slot);
			}
			continue;
		}

		char doctorId[ID_LEN];
		safeGetString(">>> 请输入医生编号: ", doctorId, ID_LEN);
		doctor* doctor = findDoctorById(sys, doctorId);
		if (doctor == NULL) {
			printf(">>> 未找到该医生，请检查输入。\n");
			continue;
		}

		TimeSlot slot = SLOT_INVALID;
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
					char testTime[TIME_STR_LEN];
					safeGetString(">>> 请输入自定义当前时间(HH:MM): ", testTime, TIME_STR_LEN);
					sprintf(testTime, "%s:00", testTime);	//将用户输入的时间补全为HH:MM:SS格式，秒部分默认为00
					strcpy(testTime, setTestTime(testTime));
					slot = changeTimeToSlot(testTime);
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
		if (bookQueueTicket(getCurrentPatientNode(), doctor, date, slot, choice == 2)) {
			appendRegistrationRecord(getCurrentPatientNode(), doctor->doctorId, doctor->department, date, slot);
			printf(">>> 挂号成功：医生[%s] 日期[%s] 时段[%s]。\n", doctor->doctorName, date, slot_names[slot - 1]);
			printf(">>> 当前时段剩余号源：%d\n", getDoctorSlotRemain(doctor->doctorId, date, slot));
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
				if (checkInQueueTicket(getCurrentPatientNode()->patientId, doctor->doctorId, date, slot, signTime)) {
					printf(">>> 当场挂号已自动签到，请及时就诊。\n");
					printSlotQueue(doctor->doctorId, date, slot);
				}
			}
		}
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

// 写入住院病例信息（预留接口）
bool appendStayMedicalRecord(HIS_System* sys, const char* patientId, const char* doctorId, const char* details, const char* startDate, const char* duration, const char* endDate, const char* wardId) {
	if (sys == NULL || patientId == NULL || doctorId == NULL || details == NULL || startDate == NULL || duration == NULL || endDate == NULL || wardId == NULL) {
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
	strncpy(rec->details, details, sizeof(rec->details) - 1);
	rec->details[sizeof(rec->details) - 1] = '\0';
	strncpy(rec->startDate, startDate, ID_LEN - 1);
	rec->startDate[ID_LEN - 1] = '\0';
	strncpy(rec->duration, duration, ID_LEN - 1);
	rec->duration[ID_LEN - 1] = '\0';
	strncpy(rec->endDate, endDate, ID_LEN - 1);
	rec->endDate[ID_LEN - 1] = '\0';
	strncpy(rec->doctorId, doctorId, ID_LEN - 1);
	rec->doctorId[ID_LEN - 1] = '\0';
	strncpy(rec->wardId, wardId, ID_LEN - 1);
	rec->wardId[ID_LEN - 1] = '\0';
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
	if (patientId == NULL) {
		printf(">>> 未提供患者编号，无法查看病例信息。\n");
		return;
	}
	Patient* target = findPatientById(sys, patientId);
	if (target == NULL) {
		printf(">>> 未找到患者 %s 的信息。\n", patientId);
		return;
	}

	printf("\n========== 患者病例信息 ==========""\n");
	printf("患者编号: %s\n", target->patientId);
	printf("患者姓名: %s\n", target->name);

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

	StayRecord* stay = target->stayHead;
	printf("\n--- 住院记录 ---\n");
	if (stay == NULL) {
		printf(">>> 暂无住院记录。\n");
	}
	while (stay != NULL) {
		printStayRecord(stay);
		printf("------------------------------\n");
		stay = stay->next;
	}
	printf("================================\n");
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

// 医生查看患者病例信息
void viewMedicalRecordDoc(HIS_System* sys, const char* doctorId) {
	if (sys == NULL) {
		printf(">>> 系统未初始化，无法查看病例信息。\n");
		return;
	}
	if (doctorId == NULL) {
		printf(">>> 医生身份无效，无法查看病例信息。\n");
		return;
	}
	char patientId[ID_LEN];
	safeGetString(">>> 请输入需要查看的患者编号(输入 -1 取消): ", patientId, ID_LEN);

	if (strcmp(patientId, "-1") == 0) {
		printf(">>> 已取消查看。\n");
		return;
	}

	Patient* targetPatient = findPatientById(sys, patientId);
	if (targetPatient == NULL) {
		printf(">>> 未找到患者 %s 的信息。\n", patientId);
		return;
	}

	// 权限判定：
	// 1) 如果该患者已被本医生叫号（QueueTicket状态为CALLED或IN_ROOM），直接允许查看
	if (isPatientCalledByDoctor(patientId, doctorId)) {
		printf(">>> 权限验证通过：患者已由您叫号，正在加载病历...\n");
		viewMedicalRecordPat(sys, patientId);
		return;
	}

	// 2) 如果患者在本医生名下已有看诊记录（复诊/随访场景），允许查看
	ConsultationRecord* targetView = findConsultationRecordByPatientId(sys, patientId);
	if (targetView != NULL) {
		if (strcmp(doctorId, targetView->doctorId) == 0) {
			printf(">>> 权限验证通过：您曾为该患者看诊，正在加载病历...\n");
			viewMedicalRecordPat(sys, patientId);
			return;
		}
	}

	// 3) 都不满足则拒绝查看
	printf(">>> 权限不足：该患者未被您叫号，且您不是该患者的主治医生。\n");
	printf(">>> 提示：请先通过【排队叫号】叫号该患者，或确认患者编号是否正确。\n");
}

// 根据医生编号查找当前正在看诊的患者编号，供写入诊断时自动选择患者使用
// 优先通过QueueTicket状态查找（已叫号/就诊中的患者），其次通过看诊记录查找
static const char* findCurrentConsultationPatientId(HIS_System* sys, const char* doctorId) {
	if (sys == NULL || doctorId == NULL) {
		return NULL;
	}
	// 优先查找已叫号的患者（QueueTicket状态为CALLED或IN_ROOM）
	const char* calledId = findCalledPatientIdByDoctor(doctorId);
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


// 医生写入患者病例信息（诊断记录）
// TODOFIND
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
	printf("-1. 取消写入诊断");
	choice = safeGetInt("请选择操作：");

	switch (choice) {
	case 1:
		{
			const char* autoId = findCurrentConsultationPatientId(sys, doctorId);
			if (autoId == NULL) {
				printf(">>> 当前没有正在看诊的患者。\n");
				return;
			}
			strcpy(patientId, autoId);
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
	if (!createExamOrder(sys, doctorId, NULL)) {
		printf(">>> 检查单开具失败。\n");
	}
}

// 医生结束看诊，将患者挂号单状态推进为FINISHED
void endConsultation(HIS_System* sys, const char* doctorId) {
	if (sys == NULL || doctorId == NULL) {
		printf(">>> 医生身份无效，无法执行结束看诊操作。\n");
		return;
	}
	printf("\n--- 结束看诊 ---\n");
	// 优先查找当前已叫号/就诊中的患者
	const char* currentId = findCalledPatientIdByDoctor(doctorId);
	if (currentId != NULL) {
		Patient* patient = findPatientById(sys, currentId);
		printf(">>> 当前看诊患者: %s (%s)\n", patient ? patient->name : "未知", currentId);
		if (confirmFunc("结束", "当前看诊")) {
			if (markTicketAsFinished(currentId, doctorId)) {
				printf(">>> 患者 %s 看诊已结束，挂号单状态已更新为 FINISHED。\n", currentId);
			}
			else {
				// 如果当前状态不是IN_ROOM（比如还是CALLED），允许强制结束
				QueueTicket* ticket = findTicketByDoctorPatient(doctorId, currentId);
				if (ticket != NULL) {
					ticket->status = STATUS_FINISHED;
					printf(">>> 患者 %s 看诊已结束，挂号单状态已强制更新为 FINISHED。\n", currentId);
				}
			}
		}
		else {
			printf(">>> 已取消结束看诊。\n");
		}
		return;
	}
	// 无自动匹配的患者，手动输入
	printf(">>> 当前没有正在看诊的患者。\n");
	char patientId[ID_LEN];
	safeGetString(">>> 请输入需要结束看诊的患者编号(输入 -1 取消): ", patientId, ID_LEN);
	if (strcmp(patientId, "-1") == 0) {
		printf(">>> 已取消结束看诊。\n");
		return;
	}
	Patient* patient = findPatientById(sys, patientId);
	if (patient == NULL) {
		printf(">>> 未找到患者 %s 的信息。\n", patientId);
		return;
	}
	if (markTicketAsFinished(patientId, doctorId)) {
		printf(">>> 患者 %s 看诊已结束，挂号单状态已更新为 FINISHED。\n", patient->name);
	}
	else {
		// 如果markTicketAsFinished因状态不符失败，尝试强制设置
		QueueTicket* ticket = findTicketByDoctorPatient(doctorId, patientId);
		if (ticket != NULL) {
			ticket->status = STATUS_FINISHED;
			printf(">>> 患者 %s 看诊已强制结束，挂号单状态已更新为 FINISHED。\n", patient->name);
		}
		else {
			printf(">>> 未找到患者 %s 对应的挂号记录，无法结束看诊。\n", patientId);
		}
	}
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
	}
	else if (sel != 0) {
		printf(">>> 无效选择，返回上级菜单。\n");
	}
}