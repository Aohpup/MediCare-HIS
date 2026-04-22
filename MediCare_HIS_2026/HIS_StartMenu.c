#define _CRT_SECURE_NO_WARNINGS
#include"HIS_StartMenu.h"
#include"DrugManage.h"
#include"DrugFileManage.h"
#include"DrugSort.h"
#include"doctorManage.h"
#include"doctorFileManage.h"
#include"DepartmentManage.h"
#include"DepartmentFileManage.h"
#include"WardManage.h"
#include"WardFileManage.h"
#include"WardSort.h"
#include"PatientManage.h"
#include"PatientFileManage.h"
#include"QueueManage.h"
#include"InputUtils.h"
#include"ConfirmFunc.h"
#include <stdio.h>

static void messageBox(const char* message) {
	if (TEST_SYSTEM_DEBUG)
		printf("\n>>> %s\n", message);
	else
		return;
}

// 管理员/系统维护视角菜单
void adminMenu(HIS_System* sys) {
	int choice;
	while (1) {
		printf("\n========== 管理员控制台 ==========\n");
		printf("1. 药品管理系统\n");
		printf("2. 医生管理系统\n");
		printf("3. 科室管理系统\n");
		printf("4. 病房及床位管理\n");
		printf("5. 财务与库存报表统计\n");
		printf("6. 保存系统数据\n");
		printf("0. 返回主菜单\n");
		printf("==================================\n");
		choice = safeGetInt("请选择管理员操作: ");

		//TODO: 这里的功能实现需要访问药品信息链表，医生信息链表，以及病房床位链表等，涉及较多数据结构操作，暂时以提示代替具体实现
		switch (choice) {
		case 1: drugManageMenu(sys); break;
		case 2: doctorManageMenu(sys); break;
		case 3: departmentManageMenu(sys); break;
		case 4: wardManageMenu(sys); break;
		case 5: printf(">>> 模块待开发: 多维数据报表统计...\n"); break;
		case 6: saveSystemData(sys); break;
		case 0:
			if (adminConfirmFunc("退出", "管理员控制台")) {
				saveSystemData(sys);
				printf(">>> 退出成功！正在返回主菜单...\n");
				return;
			}
			else {
				printf(">>> 已取消退出！正在返回操作菜单...\n");
				break;
			}
		default: printf(">>> 无效选择，请重试。\n");
		}
	}
}


// 医生/门诊看病视角菜单
void doctorMenu(HIS_System* sys) {
	loadDoctorSystemData(sys);	//加载系统数据，确保医生登录功能正常使用
	loadDrugSystemData(sys);
	loadPatientsSystemData(sys);
	printf("请先登录医生账号以进入医生工作站！\n");
	if (!logInDoctor(sys)) {
		printf("正在返回主菜单...\n");
		return;
	}
	int choice;
	while (1) {
		printf("\n========== 医生工作站 ==========\n");
		printf("1. 查看挂号候诊队列\n");
		printf("2. 排队叫号\n");
		printf("3. 查看患者病历\n");
		printf("4. 患者看诊与开具处方\n");
		printf("5. 开具检查单(面诊患者检查开单)\n");
		printf("6. 查看患者检查结果\n");
		printf("7. 安排患者住院 (病房分配)\n");
		printf("8. 医生排班管理\n");
		printf("9. 医生信息查询与修改\n");
		printf("10. 药品信息查询\n");
		printf("0. 返回主菜单\n");
		printf("================================\n");
		choice = safeGetInt("请选择医护操作: ");


		switch (choice) {
		case 1: printSlotQueue(getCurrentDoctorId(), getCurrentDateStr(), getCurrentTimeSlot()); break;
		case 2: doctorCallQueueMenu(sys, getCurrentDoctorId()); break;
		case 3: viewMedicalRecordDoc(sys, getCurrentDoctorId()); break;	//TODO:这里需要替换成实际获取当前登录医生ID的逻辑，以限制医生只能查看自己的患者病历信息
		case 4: writeMedicalRecord(sys, getCurrentDoctorId()); break;
		case 5: issueExaminationOrder(sys, getCurrentDoctorId()); break;
		case 6: printf(">>> 模块待开发: 检查结果查询系统...\n"); break;
		case 7: printf(">>> 模块待开发: 住院安排与病房分配系统...\n"); break;
		case 8: doctorScheduleMenu(sys, getCurrentDoctorId()); break; //TODO:这里需要替换成实际获取当前登录医生ID的逻辑
		case 9: doctorManageMenuDoc(sys, getCurrentDoctorId()); break;	//TODO:这里需要替换成实际获取当前登录医生ID的逻辑，以限制医生只能修改自己的信息
		case 10: drugSortMenuDoc(sys); break;
		case 0:
			if (confirmFunc("退出", "医生工作站")) {
				printf(">>> 退出成功！正在返回主菜单...\n");
				return;
			}
			else {
				printf(">>> 已取消退出！正在返回操作菜单...\n");
				break;
			}
			break;
		default: printf(">>> 无效选择，请重试。\n");
		}
	}
}

void patientMenu(HIS_System* sys) {
	printf(">>> 欢迎来到患者服务台！\n\n");
	int choice;
	loadFileAllData(sys);	//加载系统数据，确保患者登录和挂号预约功能正常使用
	while (1) {
		printf("\n========== 患者服务台 ==========\n");
		printf("1. 患者注册\n");
		printf("2. 患者登录\n");
		printf("3. 挂号和签到\n");
		printf("4. 查看病例信息\n");
		printf("5. 住院登记\n");
		printf("6. 病房查询\n");
		printf("7. 医生信息查询\n");
		printf("8. 药品信息查询\n");
		printf("0. 返回主菜单\n");
		printf("==================================\n");
		choice = safeGetInt("请选择患者服务操作: ");
		switch (choice) {
		case 1: registerPatient(sys, NULL); break;
		case 2:	logInPatient(sys); break;
		case 3: registerAppointment(sys); break;
		case 4: viewMedicalRecordPat(sys, getCurrentPatientId()); break;
		case 5: printf(">>> 模块待开发: 住院登记系统...\n"); break;
		case 6: printf(">>> 模块待开发: 病房查询系统...\n"); break;
		case 7: doctorManageMenuPat(sys, getCurrentPatientId()); break;
		case 8: drugManageMenuPat(sys, getCurrentPatientId());	break;
		case 0:
			if (confirmFunc("退出", "患者服务台")) {
				printf(">>> 退出成功！正在返回主菜单...\n");
				return;
			}
			else {
				printf(">>> 已取消退出！正在返回操作菜单...\n");
				break;
			}
		default: printf(">>> 无效选择，请重试。\n");
		}
	}
}

// 主菜单入口
void showMainMenu(HIS_System* sys) {
/*	messageBox("目前仅支持管理员登录功能，医生和患者登录功能待开发。");
	messageBox("请使用管理员登录进入系统，管理员系统只支持药品管理功能、医生管理功能和科室管理功能。");
	messageBox("药品信息格式：ID 国标码 通用名 商品名 别名 库存 价格");
	messageBox("例如：DRG001 86900001000012 阿莫西林胶囊 阿莫仙 阿莫 100 25.50");
	messageBox("医生信息格式：ID 姓名 所在科室 诊号数量");
	messageBox("例如：DOC001 张明伟 内科 50");
	messageBox("科室信息格式：一级科室名称 一级科室代码 二级科室名称 科室编号");
	messageBox("例如：内科 V100201 心内科 A234");*/
	int choice;
	while (1) {
		printf("\n*********** 医疗管理系统 (HIS) ***********\n");
		printf("1. 管理员登录 (系统维护与数据统计)\n");
		printf("2. 医生办公 (门诊看病与病房管理)\n");
		printf("3. 患者服务台 (挂号与信息查询)\n");
		printf("0. 退出系统\n");
		printf("******************************************\n");

		choice = safeGetInt("请选择您的身份: ");

		switch (choice) {
		case 1: adminMenu(sys); break;
		case 2: doctorMenu(sys); break;
		case 3: patientMenu(sys); break;
		case 0:
			saveSystemData(sys);
			if (confirmFunc("退出", "系统")) {
				cleanupSystemMemory(sys);
				printf(">>> 感谢使用，系统已安全退出！\n");
				exit(0);
			}
			break;
		default:
			printf(">>> 无效选择，请重试。\n");
		}
	}
}



