#pragma once
#ifndef PAUSE_UTIL_H
#define PAUSE_UTIL_H

#include<stdio.h>

// 暂停并等待用户按回车键，用于在展示信息后让用户有时间阅读
// 调用后光标暂停，直到用户按下回车键才回到上一菜单
void pressEnterToContinue(void);

// 调用后光标暂停，直到用户按下回车键才继续执行
void pressEnterToNext(void);

// 调用后光标暂停，直到用户按下回车键才显示自定义消息
void pressEnterToContinueWithMessage(const char* msg);

#endif // !PAUSE_UTIL_H
