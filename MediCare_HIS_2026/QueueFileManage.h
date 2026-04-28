#pragma once
#ifndef QUEUEFILEMANAGE_H
#define QUEUEFILEMANAGE_H
//排队挂号文件管理模块头文件
#include"HIS_System.h"

extern bool is_Queue_Ticket_File_Loaded; //排队挂号数据是否已加载

// 从txt文件加载排队挂号数据
void loadQueueTicketData(HIS_System* sys);

// 将排队挂号数据保存到txt文件
void saveQueueTicketData(HIS_System* sys);

#endif // !QUEUEFILEMANAGE_H
