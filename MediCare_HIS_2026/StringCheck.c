#define _CRT_SECURE_NO_WARNINGS
#include "StringCheck.h"
#include <ctype.h>
#include <stddef.h>

bool isAllDigits(const char* str) {
	if (str == NULL || *str == '\0') return false;
	while (*str) {
		if (!isdigit((unsigned char)*str)) return false;
		str++;
	}
	return true;
}

bool isAllAlpha(const char* str) {
	if (str == NULL || *str == '\0') return false;
	while (*str) {
		if (!isalpha((unsigned char)*str)) return false;
		str++;
	}
	return true;
}

//防止用户输入中文或其他非ASCII字符导致系统异常，要求输入的字符串必须全部为可打印ASCII字符（32-126），空串/NULL 返回 true
bool isAllPrintableAscii(const char* str) {
	if (!str) return true;
	for (int i = 0; str[i]; i++) {
		unsigned char c = (unsigned char)str[i];
		if (c < 32 || c > 126) return false;  // 排除控制字符和中文
	}
	return true;
}