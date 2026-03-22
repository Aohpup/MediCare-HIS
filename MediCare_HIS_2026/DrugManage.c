#define _CRT_SECURE_NO_WARNINGS

#include"DrugManage.h"
#include"HIS_System.h"
#include"BufferClear.h"

//---------------------------------------------------
//药品编号防重复
bool isDrugIdExist(Drug* head, const char* id) {
    Drug* curr = head;
    while (curr != NULL) {
        if (strcmp(curr->drugId, id) == 0) return true;
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
bool isDrugAliNameExist(Drug* head, const char* aliName) {
    Drug* curr = head;
    while (curr != NULL) {
        if (strcmp(curr->alias, aliName) == 0) return true;
        curr = curr->next;
    }
    return false;
}
//-----------------------------------------------------------------

void addDrug(HIS_System* sys) {
    Drug* newDrug = (Drug*)malloc(sizeof(Drug));
    if (!newDrug) {
        printf("内存分配失败！\n");
        return;
    }

    printf("\n--- 录入新药品 ---\n");

    while (1) {
        printf("请输入药品编号: ");
        if (scanf("%s", newDrug->drugId) != 1) {
			clearBuffer();
			printf("输入错误，请重新输入!\n");
            continue;
        };
        clearBuffer();
        if (isDrugIdExist(sys->drugHead, newDrug->drugId)) {
            printf("错误：该药品编号已存在！\n");
            printf("请重新输入。\n");
            continue;
        }
        break;
    }

    while (1) {
        printf("请输入通用名: ");
        if (scanf("%s", newDrug->genericName) != 1) {
			clearBuffer();
			printf("输入错误，请重新输入!\n");
            continue;
        }
        clearBuffer();
        if (isDrugAliNameExist(sys->drugHead, newDrug->genericName) || isDrugTraNameExist(sys->drugHead, newDrug->genericName)) {
            printf("错误：该通用名已存在！\n");
            printf("请重新输入。\n");
            continue;
        }
        break;
    }

    while (1) {
        printf("请输入商品名: ");
        if (scanf("%s", newDrug->tradeName) != 1) {
            clearBuffer();
			printf("输入错误，请重新输入!\n");
			continue;
        }
        clearBuffer();
        if (isDrugAliNameExist(sys->drugHead, newDrug->tradeName) || isDrugGenNameExist(sys->drugHead, newDrug->tradeName)) {
            printf("错误：该商品名已存在！\n");
			printf("请重新输入。\n"); 
            continue;
        }
		break; 
    }

    while (1) {
        printf("请输入拼音别名 (用于快速检索): ");
        if(scanf("%s", newDrug->alias) != 1) {
            clearBuffer();
			printf("输入错误，请重新输入!\n");
			continue;
        }
        clearBuffer();
        if (isDrugAliNameExist(sys->drugHead, newDrug->alias) || isDrugGenNameExist(sys->drugHead, newDrug->alias) || isDrugTraNameExist(sys->drugHead, newDrug->alias)) {
            printf("错误：该别名已存在！\n");
            printf("请重新输入。\n");
            continue;
        }
        break;
    }

    printf("请输入初始库存量: ");
    scanf("%d", &newDrug->stock);
    clearBuffer();

    printf("请输入单价: ");
    scanf("%f", &newDrug->price);
    clearBuffer();

	newDrug->next = sys->drugHead;  //头插法,之后再完善排序功能
    sys->drugHead = newDrug;

    printf(">>> 药品 [%s] 录入成功！\n", newDrug->tradeName);
}