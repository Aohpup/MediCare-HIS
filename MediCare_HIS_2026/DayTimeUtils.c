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
	int hour = 0, min = 0, sec = 0;

	if (sscanf(timeStr, "%d:%d:%d", &hour, &min, &sec) != 3) {
		return SLOT_INVALID;
	}

	if (!isValidTime(timeStr)) {
		return SLOT_INVALID;
	}

	int totalMin = hour * 60 + min;

	// 上午
	if (totalMin < 8 * 60 + 30)  return SLOT_0800_0830;
	if (totalMin < 9 * 60)       return SLOT_0830_0900;
	if (totalMin < 9 * 60 + 30)  return SLOT_0900_0930;
	if (totalMin < 10 * 60)      return SLOT_0930_1000;
	if (totalMin < 10 * 60 + 30) return SLOT_1000_1030;
	if (totalMin < 11 * 60)      return SLOT_1030_1100;
	if (totalMin < 11 * 60 + 30) return SLOT_1100_1130;

	// 下午（中午跳过）
	if (totalMin < 13 * 60 + 30) return SLOT_INVALID;
	if (totalMin < 14 * 60)      return SLOT_1330_1400;
	if (totalMin < 14 * 60 + 30) return SLOT_1400_1430;
	if (totalMin < 15 * 60)      return SLOT_1430_1500;
	if (totalMin < 15 * 60 + 30) return SLOT_1500_1530;
	if (totalMin < 16 * 60)      return SLOT_1530_1600;
	if (totalMin < 16 * 60 + 30) return SLOT_1600_1630;

	return SLOT_INVALID;
}

//可返回中午休时段（8-11）或晚间急诊时段（18），调用前请根据实际需求确认是否需要排除这些特殊时段
int changeTimeToSlotInAll(char* timeStr) {
	int hour = 0, min = 0, sec = 0;

	if (sscanf(timeStr, "%d:%d:%d", &hour, &min, &sec) != 3) {
		return SLOT_INVALID;
	}

	if (!isValidTime(timeStr)) {
		return SLOT_INVALID;
	}

	int totalMin = hour * 60 + min;
	if (totalMin < 8 * 60 + 30)  return SLOT_0800_0830;
	if (totalMin < 9 * 60)       return SLOT_0830_0900;
	if (totalMin < 9 * 60 + 30)  return SLOT_0900_0930;
	if (totalMin < 10 * 60)      return SLOT_0930_1000;
	if (totalMin < 10 * 60 + 30) return SLOT_1000_1030;
	if (totalMin < 11 * 60)      return SLOT_1030_1100;
	if (totalMin < 11 * 60 + 30) return SLOT_1100_1130;
	if (totalMin < 12 * 60)      return SLOT_1130_1200;
	if (totalMin < 12 * 60 + 30) return SLOT_1200_1230;
	if (totalMin < 13 * 60)      return SLOT_1230_1300;
	if (totalMin < 13 * 60 + 30) return SLOT_1300_1330;
	if (totalMin < 14 * 60)      return SLOT_1330_1400;
	if (totalMin < 14 * 60 + 30) return SLOT_1400_1430;
	if (totalMin < 15 * 60)      return SLOT_1430_1500;
	if (totalMin < 15 * 60 + 30) return SLOT_1500_1530;
	if (totalMin < 16 * 60)      return SLOT_1530_1600;
	if (totalMin < 16 * 60 + 30) return SLOT_1600_1630;
	return SLOT_NIGHT;	// 晚间急诊时段
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

// 闰年判断
static int leapYear(int y) {
	return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

// 某年某月的天数
static int daysInMonth(int y, int m) {
	static const int mdays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
	if (m < 1 || m > 12) return 30;
	if (m == 2 && leapYear(y)) return 29;
	return mdays[m - 1];
}

// 将日期转换为自公元元年1月1日起的天数（精确格里高利历）
long dateToDays(int y, int m, int d) {
	long days = 0;
	for (int i = 1; i < y; ++i) {
		days += leapYear(i) ? 366 : 365;
	}
	for (int i = 1; i < m; ++i) {
		days += daysInMonth(y, i);
	}
	days += d;
	return days;
}

// 将自公元元年1月1日起的天数转换为日期
void daysToDate(long totalDays, int* y, int* m, int* d) {
	*y = 1;
	while (1) {
		int dy = leapYear(*y) ? 366 : 365;
		if (totalDays > dy) {
			totalDays -= dy;
			(*y)++;
		} else {
			break;
		}
	}
	*m = 1;
	while (1) {
		int dm = daysInMonth(*y, *m);
		if (totalDays > dm) {
			totalDays -= dm;
			(*m)++;
		} else {
			break;
		}
	}
	*d = (int)totalDays;
}

void addDaysToDate(const char* dateStr, int addDays, char* out, int outSize) {
	if (dateStr == NULL || out == NULL || outSize <= 0) return;
	int y = 0, m = 0, d = 0;
	if (sscanf(dateStr, "%d-%d-%d", &y, &m, &d) != 3) {
		snprintf(out, outSize, "%s", dateStr ? dateStr : "");
		return;
	}
	long total = dateToDays(y, m, d) + addDays;
	daysToDate(total, &y, &m, &d);
	snprintf(out, outSize, "%d-%02d-%02d", y, m, d);
}

int daysBetweenDates(const char* start, const char* end) {
	if (start == NULL || end == NULL) return 0;
	int y1 = 0, m1 = 0, d1 = 0, y2 = 0, m2 = 0, d2 = 0;
	if (sscanf(start, "%d-%d-%d", &y1, &m1, &d1) != 3) return 0;
	if (sscanf(end, "%d-%d-%d", &y2, &m2, &d2) != 3) return 0;
	long total1 = dateToDays(y1, m1, d1);
	long total2 = dateToDays(y2, m2, d2);
	int diff = (int)(total2 - total1);
	return diff > 0 ? diff : 1;	// 最少算作1天
}

bool isNightTime(void) {
	int hour = 0, minute = 0, second = 0;
	if(TEST_SYSTEM_DEBUG && TEST_NIGHT_TIME) {
		char* testTime = setTestTime("19:00:00");	//测试时间设置为19:00:00，供单元测试调用
		sscanf(testTime, "%d:%d:%d", &hour, &minute, &second);
	}
	else
		getCurrentTime(&hour, &minute, &second);
	int totalMin = hour * 60 + minute;
	// 晚间急诊时段：16:30 之后到次日 08:00 之前（但排除午休 11:30 - 13:30，不过午休在白天区间内，不影响晚间判定）
	// 晚间 = 16:30 (990分钟) 及之后，或 08:00 (480分钟) 之前
	if (totalMin >= 16 * 60 + 30 || totalMin < 8 * 60) {
		return true;
	}
	// 午休期间（11:30-13:30）映射为 SLOT_INVALID，不属于夜间
	return false;
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
