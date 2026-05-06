#include"Windows.h"
#include"HIS_System.h"
#include"HIS_StartMenu.h"
#include"BackupRestore.h"
#include<signal.h>

int main() {

	SetConsoleOutputCP(65001); // 设置控制台输出编码为UTF-8，以支持中文显示，确保在Windows系统上正确显示中文字符
	signal(SIGINT, SIG_IGN);	// 忽略 Ctrl+C 中断信号，防止用户在操作过程中意外关闭程序导致数据丢失

	// 启动时检测上次退出状态（崩溃恢复）并标记本次运行
	checkAndRestoreOnStartup();

	// 创建数据文件备份
	backupAllDataFiles();

	HIS_System sys;
	initSystem(&sys);      // 底座初始化

	if (!TEST_SYSTEM_DEBUG) {	//非调试模式下，直接加载所有数据；调试模式下，允许单独测试某个模块的数据加载
		loadFileAllData(&sys);
	}

	// 进入主菜单系统
	showMainMenu(&sys);

	// 标记安全关闭
	markSafeShutdown();

	return 0;
}