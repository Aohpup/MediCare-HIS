#define _CRT_SECURE_NO_WARNINGS
#include"PauseUtil.h"

// 暂停并等待用户按回车键，用于在展示信息后让用户有时间阅读
// 调用后光标暂停，直到用户按下回车键才继续执行
void pressEnterToContinue(void) {
	printf(">>> 按回车键返回上一级菜单...");
	fflush(stdout);
	char ch;
	while((ch = getchar()) != '\n' && ch != EOF) {
		// 继续读取直到遇到换行符或文件结束，清除输入缓冲区
	}
}
