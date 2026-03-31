#define _CRT_SECURE_NO_WARNINGS
#include"ConfirmFunc.h"
#include"InputUtils.h"

// 确认函数，用于在执行关键操作前获取用户确认
// 操作名称，操作对象描述
bool confirmFunc(const char* function, const char* message) {
	char choice[8];
	while (1) {
		printf("\n>>> 确定要%s%s吗？ ", function, message);
		printf("(输入 Y 确认%s，N 取消%s): ", function, function);
		safeGetString("\n请输入您的选择: ", choice, sizeof(choice));
			if (strcmp(choice, "Y") == 0 || strcmp(choice, "y") == 0) {
				printf("\n>>> 正在%s%s...\n", function, message);
				return true; // 用户确认function
			}
			else if (strcmp(choice, "N") == 0 || strcmp(choice, "n") == 0) {	
				printf("\n>>> 已取消%s%s，正在返回...\n", function,	 message);
				return false; // 用户取消function
			}
			else {
				printf("\n>>> 无效输入，请重新选择！\n");
				continue; // 继续循环，等待有效输入
			}
		printf(">>> 严重错误，请输入 Y 或 N！！！\n");
		clearerr(stdin);// 清除输入错误状态
		continue;
	}
}


