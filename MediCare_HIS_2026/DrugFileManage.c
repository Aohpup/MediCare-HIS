#define _CRT_SECURE_NO_WARNINGS
#include"HIS_System.h"
#include"DrugFileManage.h"

// 从txt文件加载系统数据
void loadDrugSystemData(HIS_System* sys) {
    printf(">>> 正在从本地 TXT 文件加载数据...\n");
    FILE* fp = fopen(DRUG_FILE, "r");
    if (!fp) {
        printf(">>> 警告: 找不到 %s，将作为新系统启动。\n", DRUG_FILE);
        return;
    }

    char buffer[512];
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        Drug* newDrug = (Drug*)malloc(sizeof(Drug));
        // txt文件中格式为: ID 国标码 通用名 商品名 别名 库存 价格
        if (sscanf(buffer, "%s %s %s %s %s %d %lf",
            newDrug->drugId, newDrug->drugGbCode, 
            newDrug->genericName, newDrug->tradeName, newDrug->alias, 
            &newDrug->stock, &newDrug->price) == 7) {
            newDrug->next = sys->drugHead;
            sys->drugHead = newDrug;
        }
        else {
            free(newDrug);
        }
    }
    fclose(fp);
    printf(">>> 数据加载完成！\n");
}

void saveDrugSystemData(HIS_System* sys) {
    printf(">>> 正在保存药品管理系统数据...\n");
    FILE* fp = fopen(DRUG_FILE, "w");
    if (!fp) {
        printf(">>> 错误: 无法创建或打开保存文件！\n");
        return;
    }

    Drug* curr = sys->drugHead;
    while (curr != NULL) {
        fprintf(fp, "%s %s %s %s %s %d %.2f\n",
            curr->drugId, curr->drugGbCode, 
            curr->genericName, curr->tradeName, curr->alias, 
            curr->stock, curr->price);
        curr = curr->next;
    }
    fclose(fp);
    printf(">>> 系统数据保存成功！\n");
}


