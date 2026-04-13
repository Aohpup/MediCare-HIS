#define _CRT_SECURE_NO_WARNINGS
#include"HIS_StartMenu.h"
#include"DrugManage.h"
#include"DrugFileManage.h"
#include"DocterManage.h"
#include"DocterFileManage.h"
#include"DepartmentManage.h"
#include"DepartmentFileManage.h"
#include"WardManage.h"
#include"WardFileManage.h"
#include"WardSort.h"
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
			if (confirmFunc("退出", "管理员控制台")) {
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
	int choice;
	while (1) {
		printf("\n========== 医生工作站 ==========\n");
		printf("1. 查看挂号候诊队列\n");
		printf("2. 患者看诊与开具处方 (包含药品搜索)\n");
		printf("3. 开具检查单\n");
		printf("4. 安排患者住院 (病房分配)\n");
		printf("0. 返回主菜单\n");
		printf("================================\n");
		choice = safeGetInt("请选择医护操作: ");

		//TODO: 这里的功能实现需要访问患者记录链表，医生信息链表，以及病房床位链表等，涉及较多数据结构操作，暂时以提示代替具体实现
		switch (choice) {
		case 1: printf(">>> 模块待开发: 读取排队记录...\n"); break;
		case 2: printf(">>> 模块待开发: 生成看诊记录，管理处方发药...\n"); break;
		case 3: printf(">>> 模块待开发: 生成检查记录...\n"); break;
		case 4: printf(">>> 模块待开发: 生成住院记录并关联床位...\n"); break;
		case 0: return;
		default: printf(">>> 无效选择，请重试。\n");
		}
	}
}

// 患者/用户视角菜单
void patientMenu(HIS_System* sys) {
	int choice;
	while (1) {
		printf("\n========== 患者自助服务 ==========\n");
		printf("1. 患者建档/注册\n");
		printf("2. 门诊挂号 (选择科室与医生)\n");
		printf("3. 医疗记录查询 (挂号/看诊/检查/住院)\n");
		printf("4. 费用账单查询\n");
		printf("0. 返回主菜单\n");
		printf("==================================\n");
		choice = safeGetInt("请选择自助操作: ");

		//TODO: 这里的功能实现需要访问患者信息链表，医疗记录链表，以及医生信息链表等，涉及较多数据结构操作，暂时以提示代替具体实现
		switch (choice) {
		case 1: printf(">>> 模块待开发: 录入患者基础信息...\n"); break;
		case 2: printf(">>> 模块待开发: 生成挂号记录...\n"); break;
		case 3: printf(">>> 模块待开发: 遍历该患者的四大记录链表...\n"); break;
		case 4: printf(">>> 模块待开发: 统计所有开销...\n"); break;
		case 0: return;
		default: printf(">>> 无效选择，请重试。\n");
		}
	}
}

// 主菜单入口
void showMainMenu(HIS_System* sys) {
	messageBox("目前仅支持管理员登录功能，医生和患者登录功能待开发。");
	messageBox("请使用管理员登录进入系统，管理员系统只支持药品管理功能、医生管理功能和科室管理功能。");
	messageBox("药品信息格式：ID 国标码 通用名 商品名 别名 库存 价格");
	messageBox("例如：DRG001 86900001000012 阿莫西林胶囊 阿莫仙 阿莫 100 25.50");
	messageBox("医生信息格式：ID 姓名 所在科室 诊号数量");
	messageBox("例如：DOC001 张明伟 内科 50");
	messageBox("科室信息格式：一级科室名称 一级科室代码 二级科室名称 科室编号");
	messageBox("例如：内科 V100201 心内科 A234");
	int choice;
	while (1) {
		printf("\n*********** 医疗管理系统 (HIS) ***********\n");
		printf("1. 管理员登录 (系统维护与数据统计)\n");
		printf("2. 医生登录 (门诊看病与病房管理)\n");
		printf("3. 患者登录 (挂号与信息查询)\n");
		printf("0. 退出系统\n");
		printf("******************************************\n");

		choice = safeGetInt("请选择您的身份: ");

		switch (choice) {
		case 1: adminMenu(sys); break;
	    case 2: printf(">>> 模块待开发: 医生登录...\n"); /* doctorMenu(sys); */ break;
		case 3: printf(">>> 模块待开发: 患者登录...\n"); /* patientMenu(sys); */ break;
		case 0:
			/*test
			saveSystemData(sys);*/
			if (confirmFunc("退出", "系统")) {
				printf(">>> 感谢使用，系统已安全退出！\n");
				exit(0);
			}
			break;
		default:
			printf(">>> 无效选择，请重试。\n");
		}
	}
}



