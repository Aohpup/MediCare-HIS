#include<stdio.h>
#include<stdlib.h>
#include"HIS_System.h"
#include"DrugFileManage.h"
#include"HIS_StartMenu.h"

int main() {
	HIS_System sys;
	initSystem(&sys);      // 底座初始化
	/*
	TODO: 这里可以选择性地调用loadSystemData函数来加载本地数据文件，目前注释掉以便每次运行都从空系统开始，方便测试功能模块
	loadSystemData(&sys);  // 读取本地.txt数据文件
	*/

	// 进入主菜单系统
	showMainMenu(&sys);

	return 0;
}