#pragma once
#ifndef DAYTIMEUTILS_H
#define DAYTIMEUTILS_H
//日期时间工具函数
#include"HIS_System.h"
#include"time.h"

//设置测试用的日期字符串（格式同验证函数要求），供单元测试调用
char* setTestDate();

//设置测试用的时间字符串（格式同验证函数要求），供单元测试调用
char* setTestTime();

//获取当前系统日期，格式：YYYY-MM-DD或YYYY/MM/DD
char* getCurrentDateStr();

//获取当前系统日期的年、月、日部分，分别存储在year、month、day指向的字符串中
void getCurrentDate(char** year, char** month, char** day);	

//验证日期字符串格式和有效性（格式：YYYY-MM-DD或YYYY/MM/DD）
bool isValidDate(const char* dateStr);	

//获取当前系统时间，格式：HH:MM:SS
char* getCurrentTimeStr();

//获取当前系统时间的时、分、秒部分，分别存储在hour、minute、second指向的字符串中
void getCurrentTime(char** hour, char** minute, char** second);

//验证时间字符串格式和有效性（格式：HH:MM:SS）
bool isValidTime(const char* timeStr);


#endif // !DAYTIMEUTILS_H