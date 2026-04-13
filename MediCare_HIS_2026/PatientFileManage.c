#define _CRT_SECURE_NO_WARNINGS
#include"PatientManage.h"
#include"PatientFileManage.h"
#include"PatientSort.h"
#include"string.h"
bool is_Patient_File_Loaded = false;	//标记是否加载过患者数据

// 文件格式：
// P patientId name phone id card gender type
// R recordId department doctorId date time  (挂号记录)
// V recordId details date doctorId  (看诊记录)
// E recordId details date doctorId  (检查记录)
// S recordId details startDate duration endDate doctorId wardId  (住院记录)
// END  (结束当前患者)

//尾插法
//patientTail用于快速查找最后一名患者
//RegTail等用于快速查找对应患者对应项目的最后一条项目
void loadPatientsFromFile(HIS_System* sys) {
	printf(">>> 正在从患者文件中加载数据...\n");
	FILE* fp = fopen(PATIENT_FILE, "r");
	if (!fp) {
		printf(">>> 警告: 找不到 %s，将作为新系统启动。\n", PATIENT_FILE);
		return;
	}

	sys->patientTail = sys->patientHead;	//找到当前患者链表的末尾
	while (sys->patientTail != NULL && sys->patientTail->next != NULL) {	//如果链表不空，继续往后找
		sys->patientTail = sys->patientTail->next;
	}
	Patient* currPatient = NULL;	//当前正在处理的患者节点

	char tag[8] = "0";	//比对行首标签（P/R/V/E/S/END）
	while (fscanf(fp, "%7s", tag) == 1) {
		if (strcmp(tag, "P") == 0) {
			Patient* patient = (Patient*)malloc(sizeof(Patient));
			if (patient == NULL) { printf(">>> 内存分配失败，停止加载！\n"); break; }
			int t = 0;
			if (fscanf(fp, "%s %s %s %s %s %d", patient->patientId, patient->name, patient->phone,
				patient->idCard, patient->gender, &t) != 6) {
				printf(">>> 警告: 患者数据格式错误，跳过该条记录。\n");
				free(patient);
				currPatient = NULL;	//重置当前患者指针，确保下一个记录必须是新的患者
				continue;
			}
			patient->type = (PatientType)t;
			patient->regHead = NULL;
			patient->viewHead = NULL;
			patient->examHead = NULL;
			patient->stayHead = NULL;
			patient->currRegTail = NULL;
			patient->currViewTail = NULL;
			patient->currExamTail = NULL;
			patient->currStayTail = NULL;

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
			// 处理看诊记录
			ConsultationRecord* view = (ConsultationRecord*)malloc(sizeof(ConsultationRecord));
			if (view == NULL) {
				printf(">>> 内存分配失败，停止加载！\n");
				break;
			}
			if (fscanf(fp, "%s %s %s %s", view->recordId, view->details, view->date, view->doctorId) != 4) {
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
		else if (strcmp(tag, "E") == 0) {
			// 处理检查记录
			ConsultationRecord* exam = (ConsultationRecord*)malloc(sizeof(ConsultationRecord));
			if (exam == NULL) {
				printf(">>> 内存分配失败，停止加载！\n");
				break;
			}
			if (fscanf(fp, "%s %s %s %s", exam->recordId, exam->details, exam->date, exam->doctorId) != 4) {
				printf(">>> 警告: 检查记录数据格式错误，跳过该条记录。\n");
				free(exam);
				continue;
			}
			exam->record = REC_EXAM;
			exam->next = NULL;
			if(currPatient->examHead == NULL) {	//检查记录链表为空，新记录成为头节点
				currPatient->examHead = exam;
				currPatient->currExamTail = exam;	//初始化末尾指针
			}
			else {	//检查记录链表不空，添加到末尾
				currPatient->currExamTail->next = exam;
				currPatient->currExamTail = exam;	//更新末尾指针
			}
		}
		else if (strcmp(tag, "S") == 0) {
			// 处理住院记录
			StayRecord* stay = (StayRecord*)malloc(sizeof(StayRecord));
			if (stay == NULL) {
				printf(">>> 内存分配失败，停止加载！\n");
				break;
			}
			if (fscanf(fp, "%s %s %s %s %s %s %s", stay->recordId, stay->details, stay->startDate,
				stay->duration, stay->endDate, stay->doctorId, stay->wardId) != 7) {
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

	printf(">>> 患者数据加载完成！\n");
	is_Patient_File_Loaded = true;	//标记已加载患者数据
}


void savePatientsToFile(HIS_System* sys) {
	FILE* fp = fopen(PATIENT_FILE, "w");
	if (!fp) {
		printf(">>> 错误: 无法打开 %s 进行写入！\n", PATIENT_FILE);
		return;
	}
	Patient* patient = sys->patientHead;
	while (patient) {
		fprintf(fp, "P %s %s %s %s %s %d\n", patient->patientId, patient->name, patient->phone,
			patient->idCard, patient->gender, patient->type);
		RegistrationRecord* reg = patient->regHead;
		while (reg) {
			fprintf(fp, "R %s %s %s %s %s\n", reg->recordId, reg->department, reg->doctorId,
				reg->date, reg->time);
			reg = reg->next;
		}
		ConsultationRecord* view = patient->viewHead;
		while (view) {
			fprintf(fp, "V %s %s %s %s\n", view->recordId, view->details, view->date, view->doctorId);
			view = view->next;
		}
		ConsultationRecord* exam = patient->examHead;
		while (exam) {
			fprintf(fp, "E %s %s %s %s\n", exam->recordId, exam->details, exam->date, exam->doctorId);
			exam = exam->next;
		}
		StayRecord* stay = patient->stayHead;
		while (stay) {
			fprintf(fp, "S %s %s %s %s %s %s %s\n", stay->recordId, stay->details, stay->startDate,
				stay->duration, stay->endDate, stay->doctorId, stay->wardId);
			stay = stay->next;
		}
		fprintf(fp, "END\n");
		patient = patient->next;
	}
	fclose(fp);
	printf(">>> 患者数据保存完成！\n");
}