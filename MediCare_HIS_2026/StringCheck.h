#pragma once
#ifndef STRINGCHECK_H
#define STRINGCHECK_H

#include <stdbool.h>

// 判断字符串是否全部为数字字符（0-9），空串/NULL 返回 false
bool isAllDigits(const char* str);

// 判断字符串是否全部为字母字符（A-Z/a-z），空串/NULL 返回 false
bool isAllAlpha(const char* str);

#endif
