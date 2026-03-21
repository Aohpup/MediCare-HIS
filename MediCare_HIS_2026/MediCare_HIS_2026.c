#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>

#include"HIS_System.h"
#include"DrugManage.h"
#include"BufferClear.h"

int main() {
	HIS_System sys;
	initSystem(&sys);
	int choice;
	while (1) {
		printf("\n--- 医院信息系统 ---\n");
		printf("1. 录入新药品\n");
		printf("2. 退出系统\n");
		printf("请选择操作: ");
		scanf("%d", &choice);
		clearBuffer();
		switch (choice) {
			case 1:
				addDrug(&sys);
				break;
			case 2:
				printf("退出系统。再见！\n");
				return 0;
			default:
				printf("无效的选择，请重新输入。\n");
		}
	}
	return 0;
}



