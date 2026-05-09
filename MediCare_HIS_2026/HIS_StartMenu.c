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
#include"ExamManage.h"
#include"ExamFileManage.h"
#include"QueueManage.h"
#include"QueueFileManage.h"
#include"InputUtils.h"
#include"ConfirmFunc.h"

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
		case 5: showFinanceStatistics(sys); break;
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
	loadQueueTicketData(sys);	//加载排队挂号数据，确保叫号、结束看诊等功能正常使用
	loadExamItemData(sys);
	loadExamOrderData(sys);
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
		printf("7. 住院管理\n");
		printf("8. 结束看诊\n");
		printf("9. 查看就诊历史\n");
		printf("10. 医生排班管理\n");
		printf("11. 医生信息查询与修改\n");
		printf("12. 药品信息查询\n");
		printf("0. 返回主菜单\n");
		printf("================================\n");
		choice = safeGetInt("请选择医护操作: ");


		switch (choice) {
		case 1:
			if (isNightTime()) {
				printNightQueue(getCurrentDoctorId(), getCurrentDateStr());
			}
			else {
				TimeSlot curSlot = (TimeSlot)changeTimeToSlot(getCurrentTimeStr());
				if (curSlot == SLOT_INVALID) {
					printf(">>> 当前不是门诊时段，无法查看队列。\n");
				}
				else {
					printSlotQueue(getCurrentDoctorId(), getCurrentDateStr(), curSlot);
				}
			}
			break;
		case 2:
			if (isNightTime()) {
				callNextNightPatient(getCurrentDoctorId(), getCurrentDateStr());
			}
			else {
				doctorCallQueueMenu(sys, getCurrentDoctorId());
			}
			break;
		case 3: viewMedicalRecordDoc(sys, getCurrentDoctorId()); break;	
		case 4: writeMedicalRecord(sys, getCurrentDoctorId()); break;
		case 5: issueExaminationOrder(sys, getCurrentDoctorId()); break;
		case 6: queryExamOrdersByDoctor(sys, getCurrentDoctorId()); break;
		case 7: doctorWardMenu(sys, getCurrentDoctorId()); break;
		case 8: endConsultation(sys, getCurrentDoctorId()); break;
		case 9: viewConsultationHistory(sys, getCurrentDoctorId()); break;
		case 10: doctorScheduleMenu(sys, getCurrentDoctorId()); break;
		case 11: doctorManageMenuDoc(sys, getCurrentDoctorId()); break;
		case 12: drugSortMenuDoc(sys); break;
		case 0:
			// 检查当前是否有在诊患者
		{
			const char* inConsultId = findCalledPatientIdByDoctor(getCurrentDoctorId());
			if (inConsultId != NULL) {
				Patient* inConsultPat = findPatientById(sys, inConsultId);
				if (inConsultPat != NULL) {
					int result = autoEndCurrentConsultation(sys, getCurrentDoctorId());
					if (result == 0) {
						printf(">>> 仍保留患者: %s (%s)在诊状态，准备退出。\n", inConsultPat->name, inConsultId);
					}
				}
			}
		}
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
		printf("4. 查看病历信息\n");
		printf("5. 进行检查\n");
		printf("6. 查看检查结果\n");
		printf("7. 查看住院信息\n");
		printf("8. 办理出院手续\n");
		printf("9. 病房查询\n");
		printf("10. 医生信息查询\n");
		printf("11. 药品信息查询\n");
		printf("12. 患者信息(查询与修改)\n");
		printf("13. 余额充值\n");
		printf("0. 返回主菜单\n");
		printf("==================================\n");
		choice = safeGetInt("请选择患者服务操作: ");
		switch (choice) {
		case 1: registerPatient(sys, NULL); break;
		case 2:	logInPatient(sys); break;
		case 3: registerAppointment(sys); break;
		case 4: viewMedicalRecordPat(sys, getCurrentPatientId()); break;
		case 5: doPatientExamCheck(sys, getCurrentPatientId()); break;
		case 6: queryExamOrdersByPatient(sys, getCurrentPatientId()); break;
		case 7: patientViewStayInfo(sys, getCurrentPatientId()); break;
		case 8: patientDischargeCheckout(sys, getCurrentPatientId()); break;
		case 9: wardQueryMenuPat(sys, getCurrentPatientId()); break;
		case 10: doctorManageMenuPat(sys, getCurrentPatientId()); break;
		case 11: drugManageMenuPat(sys, getCurrentPatientId());	break;
		case 12: patientInfoMenu(sys, getCurrentPatientId()); break;
		case 13: patientRechargeMenu(sys); break;
		case 0:
			if (confirmFunc("退出", "患者服务台")) {
				if (is_Patient_Logged_In) 
				logOutPatient();
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
	int choice;
	while (1) {
		printf("\n\n*********** 医疗管理系统 (HIS) ***********\n");
		printf("1. 管理员登录\n");
		printf("2. 医生办公\n");
		printf("3. 患者服务台\n");
		printf("0. 退出系统\n");
		printf("******************************************\n");

		choice = safeGetInt("请选择您的身份: ");

		switch (choice) {
		case 1: adminMenu(sys); break;
		case 2: doctorMenu(sys); break;
		case 3: patientMenu(sys); break;
		case 0:
			if (confirmFunc("退出", "系统")) {
				saveSystemData(sys);
				cleanupSystemMemory(sys);
				printf(">>> 感谢使用，系统已安全退出！\n");
				return;
			}
			break;
		default:
			printf(">>> 无效选择，请重试。\n");
		}
	}
}



