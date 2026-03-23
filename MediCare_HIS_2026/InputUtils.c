#define _CRT_SECURE_NO_WARNINGS
#include "InputUtils.h"
#include"BufferClear.h"

	// 安全输入工具函数

	// 安全获取字符串输入 (去除换行符)
	//prompt:提示用户输入的信息
	//dest:存储输入结果的目标字符串
	//size:目标字符串的最大长度
	void safeGetString(const char* prompt, char* dest, int size) {
		char buffer[256];
		while (1) {
			printf("%s", prompt);
			char* result = fgets(buffer, sizeof(buffer), stdin);
			if(result == NULL) {
				printf(">>> 错误：输入读取失败，请重试！\n");
				clearerr(stdin); // 清除输入错误状态
				continue;
			}
			else {
				// 移除换行符
				buffer[strcspn(buffer, "\n")] = '\0';
				if (strlen(buffer) > 0) {
					int length = size - 1;
					strncpy(dest, buffer, length);//确保不会超过目标字符串的大小
					dest[length] = '\0'; // strncpy溢出时不会填加'\0',确保目标字符串以 null 结尾
					return;
				}
			}
		}
	}

	// 安全获取整数输入
	//prompt:提示用户输入的信息
	int safeGetInt(const char* prompt) {
		char buffer[256];
		int value;
		while (1) {
			printf("%s", prompt);
			char* result = fgets(buffer, sizeof(buffer), stdin);
			if(result == NULL) {	
				printf(">>> 错误：输入读取失败，请重试！\n");
				clearerr(stdin); // 清除输入错误状态
				continue;
			}
			else {
				if (sscanf(buffer, "%d", &value) == 1) {
					return value;
				}
				printf(">>> 错误：请输入有效的整数！\n");
			}
		}
	}

	// 安全获取浮点数输入
	//prompt:提示用户输入的信息
	double safeGetDouble(const char* prompt) {
		char buffer[256];
		double value;
		while (1) {
			printf("%s", prompt);
			char* result = fgets(buffer, sizeof(buffer), stdin);
			if(result == NULL) {
				printf(">>> 错误：输入读取失败，请重试！\n");
				clearerr(stdin); // 清除输入错误状态
				continue;
			}
			else {
				if (sscanf(buffer, "%lf", &value) == 1) {
					return value;
				}
				printf(">>> 错误：请输入有效的数字！\n");
			}
		}
	}



