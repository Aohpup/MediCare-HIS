#define _CRT_SECURE_NO_WARNINGS
#include"DayTimeUtils.h"
#include"string.h"

//atoi():将字符串转换为整数，遇到非数字字符停止转换

char* setTestDate(char* dateStr) {
	if(TEST_SYSTEM_DEBUG)
	return dateStr;	//直接返回输入的测试日期字符串，供单元测试调用
	else {
		printf(">>> 测试系统未启用，无法设置测试日期！已使用当前日期\n");
		return getCurrentDateStr();	//返回当前日期字符串，供正常调用使用
	}
}

char* setTestTime(char* timeStr) {
	if(TEST_SYSTEM_DEBUG)
	return timeStr;	//直接返回输入的测试时间字符串，供单元测试调用
	else {
		printf(">>> 测试系统未启用，无法设置测试时间！已使用当前时间\n");
		return getCurrentTimeStr();	//返回当前时间字符串，供正常调用使用
	}
}

char* getCurrentDateStr() {
	static char dateStr[11];	//静态字符串用于返回当前日期，格式：YYYY-MM-DD
	time_t t = time(NULL);
	struct tm* tm_info = localtime(&t);
	sprintf(dateStr, "%04d-%02d-%02d", tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday);
	return dateStr;
}

void getCurrentDate(char** year, char** month, char** day) {
	time_t t = time(NULL);
	struct tm* tm_info = localtime(&t);
	static char y[5], m[3], d[3];	//静态字符串用于返回年、月、日部分
	sprintf(y, "%04d", tm_info->tm_year + 1900);
	sprintf(m, "%02d", tm_info->tm_mon + 1);
	sprintf(d, "%02d", tm_info->tm_mday);
	*year = y; *month = m; *day = d;
}

char* getCurrentTimeStr() {
	static char timeStr[9];	//静态字符串用于返回当前时间，格式：HH:MM:SS
	time_t t = time(NULL);
	struct tm* tm_info = localtime(&t);
	sprintf(timeStr, "%02d:%02d:%02d", tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
	return timeStr;
}

void getCurrentTime(int* hour, int* minute, int* second) {
	time_t t = time(NULL);
	struct tm* tm_info = localtime(&t);
	*hour = tm_info->tm_hour;
	*minute = tm_info->tm_min;
	*second = tm_info->tm_sec;
}

int changeTimeToSlot(char* timeStr) {
	int hour = -1, min = -1, sec = -1;
	hour = atoi(timeStr);	//提取小时
	min = atoi(timeStr + 3);	//提取分钟
	sec = atoi(timeStr + 6);	//提取秒钟

	if (hour < 8 || (hour == 8 && min < 30)) {
		return SLOT_0800_0830;
	}
	if (hour < 9 || (hour == 9 && min < 30)) {
		return SLOT_0830_0900;
	}
	if (hour < 10 || (hour == 10 && min < 30)) {
		return SLOT_0900_0930;
	}
	if (hour < 11 || (hour == 11 && min < 30)) {
		return SLOT_0930_1000;
	}
	if (hour < 13 || (hour == 13 && min < 30)) {
		return SLOT_1000_1030;
	}
	if (hour < 14 || (hour == 14 && min < 30)) {
		return SLOT_1030_1100;
	}
	if (hour < 15 || (hour == 15 && min < 30)) {
		return SLOT_1100_1130;
	}
	if (hour < 15 || (hour == 15 && min < 30)) {
		return SLOT_1330_1400;
	}
	if (hour < 16 || (hour == 16 && min < 30)) {
		return SLOT_1400_1430;
	}
	if (hour < 17 || (hour == 17 && min < 30)) {
		return SLOT_1430_1500;
	}
	if (hour < 18 || (hour == 18 && min < 30)) {
		return SLOT_1500_1530;
	}
	if (hour < 19 || (hour == 19 && min < 30)) {
		return SLOT_1530_1600;
	}
	return SLOT_INVALID; // 超过最后一个时段
}


bool isValidDate(const char* dateStr) {
	if (strlen(dateStr) != 10) 
		return false;	//长度必须为10

	if ((dateStr[4] != '-' && dateStr[4] != '/') || (dateStr[7] != '-' && dateStr[7] != '/')) 
		return false;	//必须包含两个'-'或 '/'分隔符

	int year = atoi(dateStr);	//提取年份
	int month = atoi(dateStr + 5);	//提取月份
	int day = atoi(dateStr + 8);	//提取日期

	getCurrentDateStr();	//获取当前日期字符串
	int currYear = atoi(dateStr);	//当前年份

	if (year < 2026 || year > currYear + 2) return false;	//年份范围限制（2026年至当前年份+2年）

	if (month < 1 || month > 12) return false;	//月份范围限制

	int daysInMonth[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
	if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) daysInMonth[1] = 29;	//闰年2月29天

	if (day < 1 || day > daysInMonth[month - 1]) return false;	//日期范围限制

	return true;	//通过所有验证，日期有效
}

bool isValidTime(const char* timeStr) {
	if (strlen(timeStr) != 8) 
		return false;	//长度必须为8

	if (timeStr[2] != ':' || timeStr[5] != ':') 
		return false;	//必须包含两个':'分隔符

	int hour = atoi(timeStr);	//提取小时
	int minute = atoi(timeStr + 3);	//提取分钟
	int second = atoi(timeStr + 6);	//提取秒钟

	if (hour < 0 || hour > 23) return false;	//小时范围限制
	if (minute < 0 || minute > 59) return false;	//分钟范围限制
	if (second < 0 || second > 59) return false;	//秒钟范围限制

	return true;	//通过所有验证，时间有效
}
