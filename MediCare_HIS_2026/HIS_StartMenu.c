#define _CRT_SECURE_NO_WARNINGS
#include"HIS_StartMenu.h"
#include"DrugManage.h"
#include"DrugFileManage.h"
#include"InputUtils.h"
#include <stdio.h>

// 管理员/系统维护视角菜单
void adminMenu(HIS_System* sys) {
    int choice;
    while (1) {
        printf("\n========== 管理员控制台 ==========\n");
        printf("1. 录入新药品入库\n");
        printf("2. 注册新医生信息\n");
        printf("3. 病房及床位管理 (包含类型关联)\n");
        printf("4. 财务与库存报表统计\n");
        printf("5. 保存系统数据\n");
        printf("0. 返回主菜单\n");
        printf("==================================\n");
        choice = safeGetInt("请选择管理员操作: ");

		//TODO: 这里的功能实现需要访问药品信息链表，医生信息链表，以及病房床位链表等，涉及较多数据结构操作，暂时以提示代替具体实现
        switch (choice) {
        case 1: addDrug(sys); break;
        case 2: printf(">>> 模块待开发: 录入至少20名医生...\n"); break;
        case 3: printf(">>> 模块待开发: 3种病房与床位分配...\n"); break;
        case 4: printf(">>> 模块待开发: 多维数据报表统计...\n"); break;
        case 5: /*test saveSystemData(sys); */break;
        case 0: return;
        default: printf(">>> 无效选择，请重试。\n");
        }
    }
}

/*test

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

*/

// 主菜单入口
void showMainMenu(HIS_System* sys) {
    int choice;
    while (1) {
        printf("\n******** 医疗管理系统 (HIS) ********\n");
        printf("1. 管理员登录 (系统维护与数据统计)\n");
        printf("2. 医生登录 (门诊看病与病房管理)\n");
        printf("3. 患者登录 (挂号与信息查询)\n");
        printf("0. 退出系统\n");
        printf("******************************************\n");
        choice = safeGetInt("请选择您的身份: ");

        switch (choice) {
        case 1: adminMenu(sys); break;
      //case 2: doctorMenu(sys); break;
      //case 3: patientMenu(sys); break;
        case 0:
            /*test
            saveSystemData(sys);*/
            printf(">>> 感谢使用，系统已安全退出！\n");
			exit(0);
        default:
            printf(">>> 无效选择，请重试。\n");
        }
    }
}



