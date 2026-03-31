#pragma once
#ifndef confirmFunc_H
#define confirmFunc_H

#include<stdbool.h>
#include<stdio.h>

// 确认函数，用于在执行关键操作前获取用户确认
// 操作名称，操作对象描述
bool confirmFunc(const char* function, const char* message);

#endif // !confirmFunc_H




