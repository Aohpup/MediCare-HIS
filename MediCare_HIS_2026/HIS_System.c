#define _CRT_SECURE_NO_WARNINGS
#include"HIS_System.h"
#include"ConfirmFunc.h"
#include"DrugFileManage.h"
#include"DocterFileManage.h"
#include"DepartmentFileManage.h"
#include"PatientFileManage.h"
#include"WardFileManage.h"

//git push -u origin main
//初始化医疗管理系统的所有子模块链表
//sys 医疗管理系统底座指针
bool TEST_SYSTEM_DEBUG = true;		//是否启用测试（true启用，false禁用）

int currentPatientId = STARTING_PATIENT_ID;	//当前患者编号计数器（全局变量，初始值为STARTING_PATIENT_ID）

const char* slot_names[SLOT_COUNT] = {
	"08:00-08:30", "08:30-09:00", "09:00-09:30", "09:30-10:00",
	"10:00-10:30", "10:30-11:00", "11:00-11:30",
	"13:30-14:00", "14:00-14:30", "14:30-15:00",
	"15:00-15:30", "15:30-16:00", "16:00-16:30"
};

void initSystem(HIS_System* sys) {
	// 将所有模块的链表头指针初始化为空
	sys->drugHead = NULL;   
	sys->drugDisplayHead = NULL;
	sys->docHead = NULL;	
	sys->subDeptHead = NULL;
	sys->deptHead = NULL;
	sys->wardHead = NULL;
	sys->patientHead = NULL;
	sys->patientTail = NULL;
	currentPatientId = STARTING_PATIENT_ID;	// 初始化患者编号计数器
	printf(">>> 医疗管理系统底座初始化完成。\n");
}

// 从文件保存系统数据
void saveSystemData(HIS_System* sys) {

	if (adminConfirmFunc("保存", "系统数据")) {

		if(is_Drug_File_Loaded)
			saveDrugSystemData(sys);
		else
			printf(">>> 警告: 药品数据未加载，无法保存！\n");

		if(is_Doctor_File_Loaded)
			saveDoctorSystemData(sys);
		else
			printf(">>> 警告: 医生数据未加载，无法保存！\n");

		if(is_Department_File_Loaded)
			saveDepartmentSystemData(sys);
		else
			printf(">>> 警告: 科室数据未加载，无法保存！\n");

		if(is_Ward_File_Loaded)
			saveWardSystemData(sys);
		else
			printf(">>> 警告: 病房数据未加载，无法保存！\n");

		if(is_Patient_File_Loaded)
			savePatientsSystemData(sys);
		else
			printf(">>> 警告: 患者数据未加载，无法保存！\n");

		printf(">>> 所有系统数据已处理完毕。\n");
	}
}

void cleanupSystemMemory(HIS_System* sys) {
	if (sys == NULL) {
		return;
	}
	freeDrugList(sys->drugHead);
	sys->drugHead = NULL;
	freeDrugList(sys->drugDisplayHead);
	sys->drugDisplayHead = NULL;
}
