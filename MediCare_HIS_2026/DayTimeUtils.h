#pragma once
#ifndef DAYTIMEUTILS_H
#define DAYTIMEUTILS_H
//日期时间工具函数
#include"HIS_System.h"
#include"time.h"

//设置测试用的日期字符串（格式同验证函数要求），供单元测试调用
char* setTestDate(char* dateStr);

//设置测试用的时间字符串（格式同验证函数要求），供单元测试调用
char* setTestTime(char* timeStr);

//获取当前系统日期，格式：YYYY-MM-DD或YYYY/MM/DD
char* getCurrentDateStr();

//获取当前系统日期的年、月、日部分，分别存储在year、month、day指向的字符串中
void getCurrentDate(char** year, char** month, char** day);	

//将时间字符串转换为对应的时段编号，返回SLOT_INVALID表示无效时间或超过最后一个时段
int changeTimeToSlot(char* timeStr);

//验证日期字符串格式和有效性（格式：YYYY-MM-DD或YYYY/MM/DD）
bool isValidDate(const char* dateStr);	

//获取当前系统时间，格式：HH:MM:SS
char* getCurrentTimeStr();

//获取当前系统时间的时、分、秒部分，分别存储在hour、minute、second指向的字符串中
void getCurrentTime(int* hour, int* minute, int* second);

//验证时间字符串格式和有效性（格式：HH:MM:SS）
bool isValidTime(const char* timeStr);


#endif // !DAYTIMEUTILS_H