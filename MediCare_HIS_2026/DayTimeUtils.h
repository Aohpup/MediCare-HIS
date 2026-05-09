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

//将时间字符串转换为对应的时段编号，包含全部时段，没有无效值，超过最后一个时段返回SLOT_NIGHT（晚间急诊时段）
int changeTimeToSlotInAll(char* timeStr);

//验证日期字符串格式和有效性（格式：YYYY-MM-DD或YYYY/MM/DD）
bool isValidDate(const char* dateStr);	

//获取当前系统时间，格式：HH:MM:SS
char* getCurrentTimeStr();

//获取当前系统时间的时、分、秒部分，分别存储在hour、minute、second指向的字符串中
void getCurrentTime(int* hour, int* minute, int* second);

//验证时间字符串格式和有效性（格式：HH:MM:SS）
bool isValidTime(const char* timeStr);

// 将日期转换为自公元元年开始的天数（含闰年处理）
long dateToDays(int y, int m, int d);

// 将自公元元年开始的天数转换为日期
void daysToDate(long totalDays, int* y, int* m, int* d);

// 日期加天数，结果写入 out
void addDaysToDate(const char* dateStr, int addDays, char* out, int outSize);

// 计算两日期之间的天数（使用精确日历算法）
int daysBetweenDates(const char* start, const char* end);

//判断当前时间是否为晚间急诊时段（16:30之后~次日08:00之前，不含午休11:30-13:30）
bool isNightTime(void);

#endif // !DAYTIMEUTILS_H