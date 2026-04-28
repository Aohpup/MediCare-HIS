#pragma once
#ifndef EXAMMANAGE_H
#define EXAMMANAGE_H

#include"HIS_System.h"

// 医生开具检查单（patientId 为空时由函数提示输入）
bool createExamOrder(HIS_System* sys, const char* doctorId, const char* patientId);

// 技师查看待执行检查列表
void listPendingExamOrders(HIS_System* sys);

// 技师填写检查结果
bool fillExamResult(HIS_System* sys, const char* orderId, const char* itemId, const char* result);

// 医生查询自己开具的检查单
void queryExamOrdersByDoctor(HIS_System* sys, const char* doctorId);

// 患者查询自己的检查单
void queryExamOrdersByPatient(HIS_System* sys, const char* patientId);

// 打印检查单详情
void printExamOrderDetail(const ExamOrder* order);

#endif // !EXAMMANAGE_H
