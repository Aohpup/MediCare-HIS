#pragma once
#ifndef HIS_STARTMENU_H
#define HIS_STARTMENU_H
#include"HIS_System.h"
// 主菜单入口
void showMainMenu(HIS_System* sys);

// 管理员/视角菜单
void adminMenu(HIS_System* sys);

// 医生视角菜单
void doctorMenu(HIS_System* sys);

// 患者/用户视角菜单
void patientMenu(HIS_System* sys);

#endif // !HIS_STARTMENU_H
