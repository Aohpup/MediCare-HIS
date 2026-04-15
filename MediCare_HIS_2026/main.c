#include<stdio.h>
#include<stdlib.h>
#include"HIS_System.h"
#include"DrugFileManage.h"
#include"HIS_StartMenu.h"

int main() {
	HIS_System sys;
	initSystem(&sys);      // 底座初始化

	//TODO:可以在这里选择性地加载数据文件，或者在系统启动时自动加载（如果文件存在）

	// 进入主菜单系统
	showMainMenu(&sys);

	return 0;
}