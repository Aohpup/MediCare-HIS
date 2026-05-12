#define _CRT_SECURE_NO_WARNINGS
#include "BackupRestore.h"
#include "HIS_System.h"       // TEST_SYSTEM_DEBUG, confirmFunc (via ConfirmFunc.h)
#include "ProjectLimits.h"    // 文件路径宏
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>           // _mkdir

// 所有需备份的数据文件清单
static const char* dataFiles[] = {
	DRUG_FILE, DOCTOR_FILE, DEPARTMENT_FILE, WARD_FILE,
	PATIENT_FILE, QUEUE_TICKET_FILE, EXAM_ITEM_FILE, EXAM_ORDER_FILE,
	FINANCE_FILE, ADMIN_FILE
};
#define DATA_FILE_COUNT 10

// 逐字节复制文件，返回0成功，-1失败
static int copyFile(const char* src, const char* dst) {
	FILE* in = fopen(src, "rb");
	if (!in) return -1;
	FILE* out = fopen(dst, "wb");
	if (!out) {
		fclose(in);
		return -1;
	}
	char buf[4096];
	size_t n;
	while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
		fwrite(buf, 1, n, out);
	}
	fclose(in);
	fclose(out);
	return 0;
}

// 启动时检测退出状态，异常则交互恢复，最后写入 running
void checkAndRestoreOnStartup(void) {
	FILE* fp = fopen(STATUS_FILE, "r");
	if (!fp) {
		if (TEST_SYSTEM_DEBUG && AUTO_BACKUP_DATA)
			printf(">>> 状态文件不存在，判定为首次运行。\n");
	} else {
		char status[32] = {0};
		if (fscanf(fp, "%31s", status) != 1) {
			// 读取失败（文件为空或格式错误），默认视为异常退出（running）
			strcpy(status, "running");
		}
		fclose(fp);

		if (strcmp(status, "running") == 0) {
			printf("\n>>> 系统检测到上次运行异常退出，是否从备份恢复数据？\n");
			if (confirmFunc("恢复", "备份数据")) {
				int restored = 0;
				for (int i = 0; i < DATA_FILE_COUNT; i++) {
					char backupPath[256];
					sprintf(backupPath, "%s/%s", BACKUP_DIR, dataFiles[i]);
					if (copyFile(backupPath, dataFiles[i]) == 0) {
						restored++;
						if (TEST_SYSTEM_DEBUG && AUTO_BACKUP_DATA)
							printf(">>> 已恢复: %s\n", dataFiles[i]);
					} else {
						if (TEST_SYSTEM_DEBUG && AUTO_BACKUP_DATA)
							printf(">>> 备份文件不存在，跳过: %s\n", dataFiles[i]);
					}
				}
				if (restored > 0)
					printf(">>> 数据恢复成功! (恢复%d个文件)\n", restored);
				else
					printf(">>> 未找到任何备份文件，恢复失败。\n");
			} else {
				printf(">>> 已跳过恢复，当前数据可能不完整。\n");
			}
		} else if (strcmp(status, "closed_safely") == 0) {
			if (TEST_SYSTEM_DEBUG && AUTO_BACKUP_DATA)
				printf(">>> 上次运行正常关闭，无需恢复。\n");
		}
	}

	// 标记本次运行为 running
	fp = fopen(STATUS_FILE, "w");
	if (fp) {
		fprintf(fp, "running\n");
		fclose(fp);
	}
}

// 复制原始数据文件到备份目录
void backupAllDataFiles(void) {
	int ret = _mkdir(BACKUP_DIR);// 尝试创建备份目录，已存在或权限不足会返回非0
	if (ret != 0) {
		if (TEST_SYSTEM_DEBUG && AUTO_BACKUP_DATA)
			printf(">>> 警告: 备份目录创建失败，可能已存在或权限不足。\n");
	} else {
		if (TEST_SYSTEM_DEBUG && AUTO_BACKUP_DATA)
			printf(">>> 备份目录已创建。\n");
	}

	if (TEST_SYSTEM_DEBUG && AUTO_BACKUP_DATA)
		printf(">>> 正在创建数据备份...\n");

	int backed = 0;
	for (int i = 0; i < DATA_FILE_COUNT; i++) {
		char dstPath[256];
		sprintf(dstPath, "%s/%s", BACKUP_DIR, dataFiles[i]);
		if (copyFile(dataFiles[i], dstPath) == 0) {
			backed++;
			if (TEST_SYSTEM_DEBUG && AUTO_BACKUP_DATA)
				printf(">>> 已备份: %s\n", dataFiles[i]);
		} else {
			if (TEST_SYSTEM_DEBUG && AUTO_BACKUP_DATA)
				printf(">>> 跳过(文件不存在): %s\n", dataFiles[i]);
		}
	}
	if (TEST_SYSTEM_DEBUG && AUTO_BACKUP_DATA)
		printf(">>> 数据备份完成! (备份%d/%d个文件)\n\n\n", backed, DATA_FILE_COUNT);
}

// 正常退出前标记安全关闭
void markSafeShutdown(void) {
	FILE* fp = fopen(STATUS_FILE, "w");
	if (fp) {
		fprintf(fp, "closed_safely\n");
		fclose(fp);
		if (TEST_SYSTEM_DEBUG && AUTO_BACKUP_DATA)
			printf(">>> 正常关闭标记已写入。\n");
	}
}
