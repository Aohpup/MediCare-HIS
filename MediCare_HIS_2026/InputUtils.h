#pragma once
#ifndef INPUTUTILS_H
#define INPUTUTILS_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// 安全获取字符串输入 (去除换行符)
void safeGetString(const char* prompt, char* dest, int size);
// 安全获取整数输入
int safeGetInt(const char* prompt);
// 安全获取浮点数输入
double safeGetDouble(const char* prompt);

#endif // !INPUTUTILS_H
