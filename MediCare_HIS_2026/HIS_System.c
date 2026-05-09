#define _CRT_SECURE_NO_WARNINGS
#include"HIS_System.h"
#include"ConfirmFunc.h"
#include"DrugFileManage.h"
#include"doctorFileManage.h"
#include"DepartmentFileManage.h"
#include"PatientFileManage.h"
#include"WardFileManage.h"
#include"QueueManage.h"
#include"QueueFileManage.h"
#include"ExamFileManage.h"
#include"PrintFormattedStr.h"
#include"PauseUtil.h"

//初始化医疗管理系统的所有子模块链表
//sys 医疗管理系统底座指针
bool TEST_SYSTEM_DEBUG = true;		//是否启用测试（true启用，false禁用）
bool AUTO_BACKUP_DATA = true;		//自动备份数据到backup目录（true启用，false禁用）
bool TEST_NIGHT_TIME = false;		//测试夜间急诊时段（true启用，false禁用）

long long fixedAsset = 10000000;	//医院固定资产（初始值为1000万，单位：元）

int currentPatientId = STARTING_PATIENT_ID;	//当前患者编号计数器（全局变量，初始值为STARTING_PATIENT_ID）

const char* slot_names[SLOT_COUNT + 1] = {
	"08:00-08:30", "08:30-09:00", "09:00-09:30", "09:30-10:00",
	"10:00-10:30", "10:30-11:00", "11:00-11:30",
	"11:30-12:00", "12:00-12:30", "12:30-13:00", "13:00-13:30",
	"13:30-14:00", "14:00-14:30", "14:30-15:00",
	"15:00-15:30", "15:30-16:00", "16:00-16:30",
	"晚间急诊"
};

void initSystem(HIS_System* sys) {
	// 将所有模块的链表头指针初始化为空
	sys->drugHead = NULL;   
	sys->drugDisplayHead = NULL;
	sys->docHead = NULL;	
	sys->subDeptHead = NULL;
	sys->deptHead = NULL;
	sys->wardHead = NULL;
	sys->examItemHead = NULL;
	sys->examOrderHead = NULL;
	sys->patientHead = NULL;
	sys->patientTail = NULL;
	sys->hospitalRevenue = 0.0;
	sys->hospitalExpenditure = 0.0;
	currentPatientId = STARTING_PATIENT_ID;	// 初始化患者编号计数器
	if(TEST_SYSTEM_DEBUG)
		printf(">>> 医疗管理系统底座初始化完成。\n");
}

// 加载财务数据文件
static void loadFinanceData(HIS_System* sys) {
	FILE* fp = fopen(FINANCE_FILE, "r");
	if (!fp) { return; }
	char line[128];
	while (fgets(line, sizeof(line), fp)) {
		double val = 0.0;
		if (sscanf(line, "R %lf", &val) == 1) sys->hospitalRevenue = val;
		else if (sscanf(line, "E %lf", &val) == 1) sys->hospitalExpenditure = val;
	}
	fclose(fp);
	if (TEST_SYSTEM_DEBUG)
		printf(">>> 财务数据加载完成。\n");
}

// 保存财务数据文件
void saveFinanceData(HIS_System* sys) {
	FILE* fp = fopen(FINANCE_FILE, "w");
	if (!fp) return;
	fprintf(fp, "R %.2f\nE %.2f\n", sys->hospitalRevenue, sys->hospitalExpenditure);
	fclose(fp);
}

// 累加医院收入
void addHospitalRevenue(HIS_System* sys, double amount) {
	if (sys != NULL && amount > 0) 
		sys->hospitalRevenue += amount;
}

// 累加医院支出
void addHospitalExpenditure(HIS_System* sys, double amount) {
	if (sys != NULL && amount > 0) 
		sys->hospitalExpenditure += amount;
}

void loadFileAllData(HIS_System* sys) {
	loadDrugSystemData(sys);
	loadDepartmentSystemData(sys);  // 科室先于医生加载，供医生校验科室引用
	loadDoctorSystemData(sys);
	loadWardSystemData(sys);
	loadPatientsSystemData(sys);
	loadQueueTicketData(sys);
	loadExamItemData(sys);
	loadExamOrderData(sys);
	loadFinanceData(sys);
	chargeAllInpatientsDaily(sys);
	if(TEST_SYSTEM_DEBUG)
		printf(">>> 所有系统数据加载完成。\n");
}

// 从文件保存系统数据
void saveSystemData(HIS_System* sys) {

	if (adminConfirmFunc("保存", "系统数据")) {

		if(is_Drug_File_Loaded)
			saveDrugSystemData(sys);
		else
			if(TEST_SYSTEM_DEBUG)
			printf(">>> 警告: 药品数据未加载，无法保存！\n");

		if(is_Doctor_File_Loaded)
			saveDoctorSystemData(sys);
		else
			if (TEST_SYSTEM_DEBUG)
			printf(">>> 警告: 医生数据未加载，无法保存！\n");

		if(is_Department_File_Loaded)
			saveDepartmentSystemData(sys);
		else
			if (TEST_SYSTEM_DEBUG)
			printf(">>> 警告: 科室数据未加载，无法保存！\n");

		if(is_Ward_File_Loaded)
			saveWardSystemData(sys);
		else
			if (TEST_SYSTEM_DEBUG)
			printf(">>> 警告: 病房数据未加载，无法保存！\n");

		if(is_Patient_File_Loaded)
			savePatientsSystemData(sys);
		else
			if (TEST_SYSTEM_DEBUG)
			printf(">>> 警告: 患者数据未加载，无法保存！\n");

		if(is_Queue_Ticket_File_Loaded)
			saveQueueTicketData(sys);
		else
			if (TEST_SYSTEM_DEBUG)
			printf(">>> 警告: 排队挂号数据未加载，无法保存！\n");

		if(is_Exam_Item_File_Loaded)
			saveExamItemData(sys);
		else
			if (TEST_SYSTEM_DEBUG)
			printf(">>> 警告: 检查项目数据未加载，无法保存！\n");

		if(is_Exam_Order_File_Loaded)
			saveExamOrderData(sys);
		else
			if (TEST_SYSTEM_DEBUG)
			printf(">>> 警告: 检查申请数据未加载，无法保存！\n");

		saveFinanceData(sys);

		if(TEST_SYSTEM_DEBUG)
		printf(">>> 所有系统数据已处理完毕。\n");
	}
}


void showFinanceStatistics(HIS_System* sys) {
	if (sys == NULL) return;

	double totalInventoryValue = 0.0;
	Drug* d = sys->drugHead;
	while (d != NULL) {
		totalInventoryValue += d->stock * d->price;
		d = d->next;
	}

	double totalRealBalance = 0.0;
	double totalBonusBalance = 0.0;
	int patientCount = 0;
	Patient* p = sys->patientHead;
	while (p != NULL) {
		totalRealBalance += p->realBalance;
		totalBonusBalance += p->bonusBalance;
		patientCount++;
		p = p->next;
	}
	double totalPatientBalance = totalRealBalance + totalBonusBalance;

	double revenue = sys->hospitalRevenue;
	double expenditure = sys->hospitalExpenditure;

	const int wLabel = 26;
	const int wValue = 20;
	char buf[64];

	printf("\n");
	printFormattedStr("===== 财务与库存报表 =====", 48);
	printf("\n\n");

	printf("【财务统计】\n");
	for (int i = 0; i < 48; i++) printf("-");
	printf("\n");

	printFormattedStr("医院固定资产:", wLabel);
	sprintf(buf, "%lld 元", fixedAsset);
	printFormattedStr(buf, wValue);
	printf("\n");

	printFormattedStr("全院总收入:", wLabel);
	sprintf(buf, "%.2f 元", revenue);
	printFormattedStr(buf, wValue);
	printf("\n");

	printFormattedStr("全院总支出:", wLabel);
	sprintf(buf, "%.2f 元", expenditure);
	printFormattedStr(buf, wValue);
	printf("\n");

	printFormattedStr("净利润:", wLabel);
	sprintf(buf, "%.2f 元", revenue - expenditure);
	printFormattedStr(buf, wValue);
	printf("\n\n");

	printf("【库存统计】\n");
	for (int i = 0; i < 48; i++) printf("-");
	printf("\n");

	printFormattedStr("药品库存总价值:", wLabel);
	sprintf(buf, "%.2f 元", totalInventoryValue);
	printFormattedStr(buf, wValue);
	printf("\n\n");

	printf("【患者统计】\n");
	for (int i = 0; i < 48; i++) printf("-");
	printf("\n");

	printFormattedStr("患者总数:", wLabel);
	sprintf(buf, "%d 人", patientCount);
	printFormattedStr(buf, wValue);
	printf("\n");

	printFormattedStr("患者实际余额合计:", wLabel);
	sprintf(buf, "%.2f 元", totalRealBalance);
	printFormattedStr(buf, wValue);
	printf("\n");

	printFormattedStr("患者赠送余额合计:", wLabel);
	sprintf(buf, "%.2f 元", totalBonusBalance);
	printFormattedStr(buf, wValue);
	printf("\n");

	printFormattedStr("患者总余额:", wLabel);
	sprintf(buf, "%.2f 元", totalPatientBalance);
	printFormattedStr(buf, wValue);
	printf("\n");

	for (int i = 0; i < 48; i++) printf("-");
	printf("\n");

	pressEnterToContinue();
}

void cleanupSystemMemory(HIS_System* sys) {
	if (sys == NULL) {
		return;
	}
	freeDrugList(sys->drugHead);
	sys->drugHead = NULL;
	freeDrugList(sys->drugDisplayHead);
	sys->drugDisplayHead = NULL;
	is_Drug_File_Loaded = false;
}
