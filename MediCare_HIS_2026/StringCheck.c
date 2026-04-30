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
