#define _CRT_SECURE_NO_WARNINGS
#include"PatientManage.h"
#include"PatientFileManage.h"
#include"PatientSort.h"
#include"string.h"
bool is_Patient_File_Loaded = false;	//标记是否加载过患者数据

// 去除字符串首尾空白字符
static char* trimStr(char* s) {
	if (s == NULL) {
		return s;
	}
	while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') {
		s++;
	}
	char* end = s + strlen(s);
	while (end > s && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r' || end[-1] == '\n')) {
		end--;
	}
	*end = '\0';
	return s;
}

// 文件格式：
// P patientId name phone id card gender type
// R recordId department doctorId date time  (挂号记录)
// V recordId details date doctorId  (看诊记录)
// M recordId details date doctorId  (处方记录，details格式: drugId:通用名:数量;...)
// S recordId startDate duration endDate deptInfo doctorId wardId bedId details  (住院记录，9字段)
// END  (结束当前患者)

//尾插法
//patientTail用于快速查找最后一名患者
//RegTail等用于快速查找对应患者对应项目的最后一条项目
void loadPatientsSystemData(HIS_System* sys) {
	if (is_Patient_File_Loaded) {
		return;
	}
	if(TEST_SYSTEM_DEBUG)
	printf(">>> 正在从患者文件中加载数据...\n");
	FILE* fp = fopen(PATIENT_FILE, "r");
	if (!fp) {
		printf(">>> 警告: 找不到 %s，将作为新系统启动。\n", PATIENT_FILE);
		return;
	}

	char dummyLine[512]; // 用于读取并判断文件开头的注释行
	long firstPos = ftell(fp);
	if (fgets(dummyLine, sizeof(dummyLine), fp) != NULL) {
		char* firstLine = trimStr(dummyLine);
		if (firstLine[0] != '#') {
			fseek(fp, firstPos, SEEK_SET);
		}
	}

	sys->patientTail = sys->patientHead;	//找到当前患者链表的末尾
	while (sys->patientTail != NULL && sys->patientTail->next != NULL) {	//如果链表不空，继续往后找
		sys->patientTail = sys->patientTail->next;
	}
	Patient* currPatient = NULL;	//当前正在处理的患者节点

	char tag[8] = "0";	//比对行首标签（P/R/V/E/S/END）
	char line[2048];
	while (fscanf(fp, "%7s", tag) == 1) {
		if (strcmp(tag, "P") == 0) {
			Patient* patient = (Patient*)malloc(sizeof(Patient));
			if (patient == NULL) { printf(">>> 内存分配失败，停止加载！\n"); break; }
			// 读取P标签后整行，按空格数判断新旧格式
			char patientLine[512];
			if (fgets(patientLine, sizeof(patientLine), fp) == NULL) {
				free(patient);
				break;
			}
			char* pl = trimStr(patientLine);
			int spaceCnt = 0;
			for (const char* p = pl; *p != '\0'; p++) {
				if (*p == ' ') spaceCnt++;
			}
			int t = 0;
			patient->age = 0;
			patient->balance = 0.0;
			patient->loginCount = 0;
			bool parseOk = false;
			if (spaceCnt >= 8) {
				// 新新格式: patientId name phone idCard gender age type balance loginCount (9字段)
				parseOk = (sscanf(pl, "%s %s %s %s %s %d %d %lf %d",
					patient->patientId, patient->name, patient->phone,
					patient->idCard, patient->gender, &patient->age, &t,
					&patient->balance, &patient->loginCount) >= 8);
			} else if (spaceCnt >= 7) {
				// 新格式: patientId name phone idCard gender age type balance (8字段，无loginCount)
				parseOk = (sscanf(pl, "%s %s %s %s %s %d %d %lf",
					patient->patientId, patient->name, patient->phone,
					patient->idCard, patient->gender, &patient->age, &t,
					&patient->balance) >= 7);
			} else {
				// 旧格式兼容: patientId name phone idCard gender type (6字段)
				parseOk = (sscanf(pl, "%s %s %s %s %s %d",
					patient->patientId, patient->name, patient->phone,
					patient->idCard, patient->gender, &t) == 6);
			}
			if (!parseOk) {
				if(TEST_SYSTEM_DEBUG)
				printf(">>> 警告: 患者数据格式错误，跳过该条记录。\n");
				free(patient);
				currPatient = NULL;
				continue;
			}
			patient->type = (PatientType)t;
			patient->regHead = NULL;
			patient->viewHead = NULL;
			patient->stayHead = NULL;
			patient->medHead = NULL;
			patient->currRegTail = NULL;
			patient->currViewTail = NULL;
			patient->currStayTail = NULL;
			patient->currMedTail = NULL;

			// 检查是否已存在相同患者编号（历史数据可能有重复），若重复则跳过该患者整段记录
			bool isDuplicate = false;
			Patient* exist = sys->patientHead;
			while (exist != NULL) {
				if (strcmp(exist->patientId, patient->patientId) == 0) {
					if(TEST_SYSTEM_DEBUG)
					printf(">>> 警告: 患者 %s 在文件中重复出现，跳过后续重复条目。\n", patient->patientId);
					isDuplicate = true;
					break;
				}
				exist = exist->next;
			}
			if (isDuplicate) {
				free(patient);
				// 跳过该重复患者后续的所有记录直到 END
				while (fscanf(fp, "%7s", tag) == 1) {
					if (strcmp(tag, "END") == 0) {
						break;
					}
					fgets(line, sizeof(line), fp);
				}
				currPatient = NULL;
				continue;
			}

			// 将新患者添加到链表末尾
			if (sys->patientTail == NULL) {	//链表为空，新患者成为头节点
				sys->patientHead = patient;
				sys->patientTail = patient;	//初始化末尾指针
			}
			else {	//链表不空，添加到末尾
				sys->patientTail->next = patient;
				sys->patientTail = patient;	//更新末尾指针
			}
			sys->patientTail->next = NULL;	//确保末尾节点的next为NULL
			currPatient = patient;	//更新当前患者指针

		}
		else if (strcmp(tag, "R") == 0) {
			if (currPatient == NULL) {
				printf(">>> 警告: 挂号记录缺少所属患者，跳过该条记录。\n");
				fgets(line, sizeof(line), fp);
				continue;
			}
			// 处理挂号记录
			RegistrationRecord* reg = (RegistrationRecord*)malloc(sizeof(RegistrationRecord));
			if (reg == NULL) {
				printf(">>> 内存分配失败，停止加载！\n");
				break;
			}
			if (fscanf(fp, "%s %s %s %s %s", reg->recordId, reg->department, reg->doctorId,
				reg->date, reg->time) != 5) {
				printf(">>> 警告: 挂号记录数据格式错误，跳过该条记录。\n");
				free(reg);
				continue;
			}
			reg->next = NULL;
			// 检查是否已存在相同的挂号记录（同医生同日期同时段），若重复则跳过
			bool regDuplicate = false;
			RegistrationRecord* regExist = currPatient->regHead;
			while (regExist != NULL) {
				if (strcmp(regExist->doctorId, reg->doctorId) == 0 &&
					strcmp(regExist->date, reg->date) == 0 &&
					strcmp(regExist->time, reg->time) == 0) {
					printf(">>> 警告: 患者 %s 的挂号记录重复 (医生%s 日期%s 时段%s)，跳过。\n",
						currPatient->patientId, reg->doctorId, reg->date, reg->time);
					regDuplicate = true;
					break;
				}
				regExist = regExist->next;
			}
			if (regDuplicate) {
				free(reg);
				continue;
			}
			if (currPatient->regHead == NULL) {	//挂号记录链表为空，新记录成为头节点
				currPatient->regHead = reg;		//初始化头节点
				currPatient->currRegTail = reg;	//初始化末尾指针
			}
			else {	//挂号记录链表不空，添加到末尾
				currPatient->currRegTail->next = reg;
				currPatient->currRegTail = reg;	//更新末尾指针
			}
		}
		else if (strcmp(tag, "V") == 0) {
			if (currPatient == NULL) {
				printf(">>> 警告: 看诊记录缺少所属患者，跳过该条记录。\n");
				fgets(line, sizeof(line), fp);
				continue;
			}
			// 处理看诊记录
			ConsultationRecord* view = (ConsultationRecord*)malloc(sizeof(ConsultationRecord));
			if (view == NULL) {
				printf(">>> 内存分配失败，停止加载！\n");
				break;
			}
			if (fgets(line, sizeof(line), fp) == NULL) {
				if(TEST_SYSTEM_DEBUG)
				printf(">>> 警告: 看诊记录数据格式错误，跳过该条记录。\n");
				free(view);
				continue;
			}
			// sscanf 一次性解析 4 个字段，格式: recordId|date|doctorId|details
			if (sscanf(trimStr(line), "%24[^|]|%24[^|]|%24[^|]|%511[^\n]",
				view->recordId, view->date, view->doctorId, view->details) != 4) {
				if(TEST_SYSTEM_DEBUG)
				printf(">>> 警告: 看诊记录数据格式错误，跳过该条记录。\n");
				free(view);
				continue;
			}
			view->record = REC_VIEW;
			view->next = NULL;
			if(currPatient->viewHead == NULL) {	//看诊记录链表为空，新记录成为头节点
				currPatient->viewHead = view;
				currPatient->currViewTail = view;	//初始化末尾指针
			}
			else {	//看诊记录链表不空，添加到末尾
				currPatient->currViewTail->next = view;
				currPatient->currViewTail = view;	//更新末尾指针
			}
		}
		else if (strcmp(tag, "M") == 0) {
			if (currPatient == NULL) {
				printf(">>> 警告: 处方记录缺少所属患者，跳过该条记录。\n");
				fgets(line, sizeof(line), fp);
				continue;
			}
			// 处理处方记录，复用ConsultationRecord结构体
			// 文件格式: M recordId|date|doctorId|details (details为药品清单，不含价格)
			ConsultationRecord* med = (ConsultationRecord*)malloc(sizeof(ConsultationRecord));
			if (med == NULL) {
				printf(">>> 内存分配失败，停止加载！\n");
				break;
			}
			if (fgets(line, sizeof(line), fp) == NULL) {
				if(TEST_SYSTEM_DEBUG)
				printf(">>> 警告: 处方记录数据格式错误，跳过该条记录。\n");
				free(med);
				continue;
			}
			if (sscanf(trimStr(line), "%24[^|]|%24[^|]|%24[^|]|%511[^\n]",
				med->recordId, med->date, med->doctorId, med->details) != 4) {
				if(TEST_SYSTEM_DEBUG)
				printf(">>> 警告: 处方记录数据格式错误，跳过该条记录。\n");
				free(med);
				continue;
			}
			med->record = REC_MED;
			med->next = NULL;
			if(currPatient->medHead == NULL) {
				currPatient->medHead = med;
				currPatient->currMedTail = med;
			}
			else {
				currPatient->currMedTail->next = med;
				currPatient->currMedTail = med;
			}
		}
		else if (strcmp(tag, "E") == 0) {
			if (currPatient == NULL) {
				printf(">>> 警告: 检查记录缺少所属患者，跳过该条记录。\n");
				fgets(line, sizeof(line), fp);
				continue;
			}
			// 旧版检查记录已迁移至独立模块，这里仅跳过读取
			fgets(line, sizeof(line), fp);
			printf(">>> 提示: 已忽略旧版检查记录，检查数据请从检查模块文件加载。\n");
		}
		else if (strcmp(tag, "S") == 0) {
			if (currPatient == NULL) {
				printf(">>> 警告: 住院记录缺少所属患者，跳过该条记录。\n");
				fgets(line, sizeof(line), fp);
				continue;
			}
			// 处理住院记录
			StayRecord* stay = (StayRecord*)malloc(sizeof(StayRecord));
			if (stay == NULL) {
				printf(">>> 内存分配失败，停止加载！\n");
				break;
			}
			if (fgets(line, sizeof(line), fp) == NULL) {
				if(TEST_SYSTEM_DEBUG)
				printf(">>> 警告: 住院记录数据格式错误，跳过该条记录。\n");
				free(stay);
				continue;
			}
			// 判断新旧格式：旧格式6个|(7字段)，旧新格式8个|(9字段)，新格式9个|(10字段)
			memset(stay, 0, sizeof(StayRecord));
			int pipes = 0;
			for (const char* p = line; *p != '\0'; p++) {
				if (*p == '|') pipes++;
			}
			bool parseOk = false;
			if (pipes >= 10) {
				// 新新格式: recordId|startDate|duration|endDate|deptInfo|doctorId|wardId|bedId|dischargeApproved|isChargeDate|details
				parseOk = (sscanf(trimStr(line), "%24[^|]|%24[^|]|%24[^|]|%24[^|]|%49[^|]|%24[^|]|%24[^|]|%63[^|]|%d|%19[^|]|%511[^\n]",
					stay->recordId, stay->startDate, stay->duration, stay->endDate,
					stay->deptInfo, stay->doctorId, stay->wardId, stay->bedId, &stay->dischargeApproved, stay->isChargeDate, stay->details) == 11);
			} else if (pipes >= 9) {
				// 新格式: recordId|startDate|duration|endDate|deptInfo|doctorId|wardId|bedId|dischargeApproved|details
				parseOk = (sscanf(trimStr(line), "%24[^|]|%24[^|]|%24[^|]|%24[^|]|%49[^|]|%24[^|]|%24[^|]|%63[^|]|%d|%511[^\n]",
					stay->recordId, stay->startDate, stay->duration, stay->endDate,
					stay->deptInfo, stay->doctorId, stay->wardId, stay->bedId, &stay->dischargeApproved, stay->details) == 10);
			} else if (pipes >= 8) {
				// 旧新格式: recordId|startDate|duration|endDate|deptInfo|doctorId|wardId|bedId|details
				parseOk = (sscanf(trimStr(line), "%24[^|]|%24[^|]|%24[^|]|%24[^|]|%49[^|]|%24[^|]|%24[^|]|%63[^|]|%511[^\n]",
					stay->recordId, stay->startDate, stay->duration, stay->endDate,
					stay->deptInfo, stay->doctorId, stay->wardId, stay->bedId, stay->details) == 9);
				stay->dischargeApproved = 0;
			} else if (pipes >= 6) {
				// 旧格式兼容: recordId|startDate|duration|endDate|doctorId|wardId|details
				parseOk = (sscanf(trimStr(line), "%24[^|]|%24[^|]|%24[^|]|%24[^|]|%24[^|]|%24[^|]|%511[^\n]",
					stay->recordId, stay->startDate, stay->duration, stay->endDate,
					stay->doctorId, stay->wardId, stay->details) == 7);
				// deptInfo 与 bedId 保持 memset 置零的结果（空字符串）
				stay->dischargeApproved = 0;
			} else {
				// 其他字段数均不支持
			}
			if (!parseOk) {
				if(TEST_SYSTEM_DEBUG)
				printf(">>> 警告: 住院记录数据格式错误，跳过该条记录。\n");
				free(stay);
				continue;
			}
			stay->next = NULL;
			if(currPatient->stayHead == NULL) {	//住院记录链表为空，新记录成为头节点
				currPatient->stayHead = stay;
				currPatient->currStayTail = stay;	//初始化末尾指针
			}
			else {	//住院记录链表不空，添加到末尾
				currPatient->currStayTail->next = stay;
				currPatient->currStayTail = stay;	//更新末尾指针
			}
		}
		else if (strcmp(tag, "END") == 0) {
			currPatient = NULL; //重置当前患者指针，确保下一个记录必须是新的患者
		}
	}
	fclose(fp);

	// 根据已加载的患者数据更新 currentPatientId，避免重启后产生重复患者编号
	int maxId = STARTING_PATIENT_ID - 1;
	Patient* scan = sys->patientHead;
	while (scan != NULL) {
		int numId = 0;
		if (sscanf(scan->patientId, "P%d", &numId) == 1) {
			if (numId > maxId) {
				maxId = numId;
			}
		}
		scan = scan->next;
	}
	currentPatientId = maxId + 1;

	if(TEST_SYSTEM_DEBUG)
	printf(">>> 患者数据加载完成！当前患者编号计数器已更新为 %d。\n", currentPatientId);
	is_Patient_File_Loaded = true;	//标记已加载患者数据
}


void savePatientsSystemData(HIS_System* sys) {
	FILE* fp = fopen(PATIENT_FILE, "w");
	if (!fp) {
		printf(">>> 错误: 无法打开 %s 进行写入！\n", PATIENT_FILE);
		return;
	}

	fprintf(fp, "# HIS PATIENT DATA FILE\n");

	Patient* patient = sys->patientHead;
	while (patient) {
		fprintf(fp, "P %s %s %s %s %s %d %d %.2f %d\n", patient->patientId, patient->name, patient->phone,
			patient->idCard, patient->gender, patient->age, patient->type, patient->balance, patient->loginCount);
		RegistrationRecord* reg = patient->regHead;
		while (reg) {
			fprintf(fp, "R %s %s %s %s %s\n", reg->recordId, reg->department, reg->doctorId,
				reg->date, reg->time);
			reg = reg->next;
		}
		ConsultationRecord* view = patient->viewHead;
		while (view) {
			fprintf(fp, "V %s|%s|%s|%s\n", view->recordId, view->date, view->doctorId, view->details);
			view = view->next;
		}
		ConsultationRecord* med = patient->medHead;
		while (med) {
			fprintf(fp, "M %s|%s|%s|%s\n", med->recordId, med->date, med->doctorId, med->details);
			med = med->next;
		}
		StayRecord* stay = patient->stayHead;
		while (stay) {
			fprintf(fp, "S %s|%s|%s|%s|%s|%s|%s|%s|%d|%s|%s\n", stay->recordId, stay->startDate,
				stay->duration, stay->endDate, stay->deptInfo,
				stay->doctorId, stay->wardId, stay->bedId, stay->dischargeApproved, stay->isChargeDate, stay->details);
			stay = stay->next;
		}
		fprintf(fp, "END\n");
		patient = patient->next;
	}
	fclose(fp);
	if(TEST_SYSTEM_DEBUG)
	printf(">>> 患者数据保存完成！\n");
}