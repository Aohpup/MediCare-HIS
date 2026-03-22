#define _CRT_SECURE_NO_WARNINGS
#include"ConfirmExit.h"

bool confirmExit() {
	char choice[4];
	while (1) {
		printf(">>> 确定要退出系统吗？ ");
		printf("(输入 Y 确认退出，N 取消退出): ");
		if (fgets(choice, sizeof(choice), stdin) != NULL) {
			// 移除换行符
			choice[strcspn(choice, "\n")] = '\0';
			if (strcmp(choice, "Y") == 0 || strcmp(choice, "y") == 0) {
				printf("\n>>> 正在退出系统...\n");
				return true; // 用户确认退出
			}
			else if (strcmp(choice, "N") == 0 || strcmp(choice, "n") == 0) {	
				printf("\n>>> 已取消退出，返回系统...\n");
				return false; // 用户取消退出
			}
			else {
				printf("\n>>> 无效输入，请重新选择！\n");
				continue; // 继续循环，等待有效输入
			}
		}
		printf(">>> 无效输入，请输入 Y 或 N。\n");
	}
}


