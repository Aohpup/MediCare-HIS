#include"Windows.h"
#include"HIS_System.h"
#include"HIS_StartMenu.h"

int main() {

	SetConsoleOutputCP(65001); // 设置控制台输出编码为UTF-8，以支持中文显示，确保在Windows系统上正确显示中文字符

	HIS_System sys;
	initSystem(&sys);      // 底座初始化

	//TODO:可以在这里选择性地加载数据文件，或者在系统启动时自动加载（如果文件存在）

	// 进入主菜单系统
	showMainMenu(&sys);

	return 0;
}