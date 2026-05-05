#pragma once
#ifndef BACKUPRESTORE_H
#define BACKUPRESTORE_H

// 启动时检测上次退出状态，异常则交互恢复，最后标记 running
void checkAndRestoreOnStartup(void);

// 将原始数据文件备份到 BACKUP_DIR 目录
void backupAllDataFiles(void);

// 正常退出前将状态文件标记为 closed_safely
void markSafeShutdown(void);

#endif // !BACKUPRESTORE_H
