#define _CRT_SECURE_NO_WARNINGS
#include"HIS_System.h"

//初始化医疗管理系统的所有子模块链表
//sys 医疗管理系统底座指针
void initSystem(HIS_System* sys) {
	// 将所有模块的链表头指针初始化为空
	sys->drugHead = NULL;   
	sys->docHead = NULL;	
	sys->subDeptHead = NULL;
	sys->deptHead = NULL;
	sys->wardHead = NULL;
	sys->patientHead = NULL;
	printf(">>> 医疗管理系统底座初始化完成。\n");
}



