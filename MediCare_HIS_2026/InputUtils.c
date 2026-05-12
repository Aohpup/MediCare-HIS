#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include "InputUtils.h"
#include "BufferClear.h"

// 安全获取字符串输入 (去除换行符)
//prompt:提示用户输入的信息
//dest:存储输入结果的目标字符串
//size:目标字符串的最大长度
void safeGetString(const char* prompt, char* dest, int size) {
    char buffer[256];
    while (1) {
        printf("%s", prompt);
        char* result = fgets(buffer, sizeof(buffer), stdin);
        if (result == NULL) {
            printf(">>> 错误：输入读取失败，请重试！\n");
            clearerr(stdin);
            continue;
        }
        else {
            buffer[strcspn(buffer, "\n")] = '\0';
            if (strlen(buffer) > 0) {
                int length = size - 1;
                strncpy(dest, buffer, length);
                dest[length] = '\0';
                return;
            }
        }
    }
}

// 检查字符串剩余部分是否全为空白字符
static int isOnlyWhitespace(const char* s) {
    while (*s) {
        if (!isspace((unsigned char)*s))
            return 0;
        s++;
    }
    return 1;
}

// 安全获取整数输入
//prompt:提示用户输入的信息
int safeGetInt(const char* prompt) {
    char buffer[256];
    long val;
    char* endptr;

    while (1) {
        printf("%s", prompt);
        char* result = fgets(buffer, sizeof(buffer), stdin);
        if (result == NULL) {
            printf(">>> 错误：输入读取失败，请重试！\n");
            clearerr(stdin);
            continue;
        }

        buffer[strcspn(buffer, "\n")] = '\0';

        errno = 0;
        val = strtol(buffer, &endptr, 10);

        if (errno == ERANGE || endptr == buffer) {
            printf(">>> 错误：请输入有效的整数！\n");
            continue;
        }

        if (!isOnlyWhitespace(endptr)) {
            printf(">>> 错误：输入包含无效后缀，请只输入整数！\n");
            continue;
        }

        if (val < INT_MIN || val > INT_MAX) {
            printf(">>> 错误：整数超出范围，请重试！\n");
            continue;
        }

        return (int)val;
    }
}

// 安全获取浮点数输入
//prompt:提示用户输入的信息
double safeGetDouble(const char* prompt) {
    char buffer[256];
    double val;
    char* endptr;

    while (1) {
        printf("%s", prompt);
        char* result = fgets(buffer, sizeof(buffer), stdin);
        if (result == NULL) {
            printf(">>> 错误：输入读取失败，请重试！\n");
            clearerr(stdin);
            continue;
        }

        buffer[strcspn(buffer, "\n")] = '\0';

        errno = 0;
        val = strtod(buffer, &endptr);

        if (errno == ERANGE || endptr == buffer) {
            printf(">>> 错误：请输入有效的数字！\n");
            continue;
        }

        if (!isOnlyWhitespace(endptr)) {
            printf(">>> 错误：输入包含无效后缀，请只输入数字！\n");
            continue;
        }

        return val;
    }
}