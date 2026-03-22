#define _CRT_SECURE_NO_WARNINGS

#include"DrugManage.h"
#include"HIS_System.h"
#include"BufferClear.h"
#include"InputUtils.h"
#include"ProjectLimits.h"

//---------------------------------------------------
//药品编号防重复
static bool isDrugIdExist(Drug* head, const char* id) {
    Drug* curr = head;
    while (curr != NULL) {
        if (strcmp(curr->drugId, id) == 0) return true;
        curr = curr->next;
    }
    return false;
}

//药品国标码防重复
static bool isDrugGbCodeExist(Drug* head, const char* gbCode) {
    Drug* curr = head;
    while (curr != NULL) {
        if (strcmp(curr->drugGbCode, gbCode) == 0) return true;
        curr = curr->next;
    }
    return false;
}

//药品通用名防重复
static bool isDrugGenNameExist(Drug* head, const char* genName) {
    Drug* curr = head;
    while (curr != NULL) {
        if (strcmp(curr->genericName, genName) == 0) return true;
        curr = curr->next;
    }
    return false;
}
//药品商品名防重复
static bool isDrugTraNameExist(Drug* head, const char* traName) {
    Drug* curr = head;
    while (curr != NULL) {
        if (strcmp(curr->tradeName, traName) == 0) return true;
        curr = curr->next;
    }
    return false;
}
//药品别名防重复
static bool isDrugAliNameExist(Drug* head, const char* aliName) {
    Drug* curr = head;
    while (curr != NULL) {
        if (strcmp(curr->alias, aliName) == 0) return true;
        curr = curr->next;
    }
    return false;
}

//-----------------------------------------------------------------

//录入新药品
void addDrug(HIS_System* sys) {
    Drug* newDrug = (Drug*)malloc(sizeof(Drug));
    if (!newDrug) {
        printf(">>> 错误：内存分配失败！\n");
        return;
    }

    printf("\n--- 录入新药品 ---\n");

    //输入并验证药品编号
    while (1) {
        safeGetString("请输入药品编号:", newDrug->drugId, ID_LEN);
        if (isDrugIdExist(sys->drugHead, newDrug->drugId)) {
            printf(">>> 错误：该药品编号已存在，请重新输入！\n");
            continue;
        }
        break;
    }

	//输入并验证国标码
    while (1) {
        safeGetString("请输入国家药品本位码(14位国标码): ", newDrug->drugGbCode, 14);
        if (isDrugGbCodeExist(sys->drugHead, newDrug->drugGbCode)) {
            printf(">>> 错误：该国标码对应药品已存在，请重新输入！\n");
            continue;
        }
        break;
    }

	//输入并验证通用名、商品名、别名，确保不与系统内各类已有名称冲突
    while (1) {
        safeGetString("请输入通用名: ", newDrug->genericName, STR_LEN);
		if (isDrugGenNameExist(sys->drugHead, newDrug->genericName) ||
            isDrugTraNameExist(sys->drugHead, newDrug->genericName) ||
            isDrugAliNameExist(sys->drugHead, newDrug->genericName)) {
            printf(">>> 错误：该名称与系统内已有通用名冲突！\n");
            continue;
        }
        break;
    }

    while (1) {
        safeGetString("请输入商品名: ", newDrug->tradeName, STR_LEN);
        if (isDrugGenNameExist(sys->drugHead, newDrug->genericName) ||
            isDrugTraNameExist(sys->drugHead, newDrug->genericName) ||
            isDrugAliNameExist(sys->drugHead, newDrug->genericName)) {
            printf(">>> 错误：该名称与系统内已有商品名冲突！\n");
            continue;
        }
        break;
    }

    while (1) {
        safeGetString("请输入拼音别名 (用于快速检索): ", newDrug->alias, STR_LEN);
        if (isDrugGenNameExist(sys->drugHead, newDrug->genericName) ||
            isDrugTraNameExist(sys->drugHead, newDrug->genericName) ||
            isDrugAliNameExist(sys->drugHead, newDrug->genericName)) {
            printf(">>> 错误：该别名已存在！\n");
            continue;
        }
        break;
    }

    while(1){
        newDrug->stock = safeGetInt("请输入初始库存量: ");
        if (newDrug->stock < 0) {
            printf(">>> 错误：库存量不能为负数！\n");
            continue;
        }
        break;
	}

    while (1) {
        newDrug->price = safeGetDouble("请输入单价: ");
        if(newDrug->price < 0) {
            printf(">>> 错误：价格不能为负数！\n");
            continue;
		}
        break;
    }

    // 头插法
    newDrug->next = sys->drugHead;
    sys->drugHead = newDrug;

    printf(">>> 药品 <%s>(%s) 录入成功！\n", newDrug->genericName, newDrug->tradeName);
}