#define _CRT_SECURE_NO_WARNINGS
#include"HIS_System.h"
#include"DrugFileManage.h"
bool is_Drug_File_Loaded = false;	//标记是否加载过药品数据

// 从txt文件加载系统数据
void loadDrugSystemData(HIS_System* sys) {
	if(TEST_SYSTEM_DEBUG)
	printf(">>> 正在从药品文件中加载数据...\n");
	FILE* fp = fopen(DRUG_FILE, "r");
	if (!fp) {
		if (!TEST_SYSTEM_DEBUG) {
			printf("严重错误: 药品数据不存在！请确保文件存在或联系管理员。\n");
			exit(EXIT_FAILURE); // 直接退出程序，避免后续操作导致更严重的错误
		}
		printf(">>> 警告: 找不到 %s，将作为新系统启动。\n", DRUG_FILE);
		return;
	}

	char dummyLine[512]; // 用于读取和丢弃文件开头的注释行
	fgets(dummyLine, sizeof(dummyLine), fp); // 读取并丢弃第一行注释

	char buffer[512];
	while (fgets(buffer, sizeof(buffer), fp) != NULL) {
		Drug* newDrug = (Drug*)malloc(sizeof(Drug));
		if (newDrug == NULL) {
			printf(">>> 错误: 内存分配失败，无法加载药品数据！\n");
			fclose(fp);
			return;
		}
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
	if(TEST_SYSTEM_DEBUG)
	printf(">>> 数据加载完成！\n");
	is_Drug_File_Loaded = true;	//标记已加载过药品数据
}

void saveDrugSystemData(HIS_System* sys) {
	FILE* fp = fopen(DRUG_FILE, "w");
	if (!fp) {
		if (!TEST_SYSTEM_DEBUG) {
			printf("严重错误: 无法保存文件！请确保文件权限或联系管理员。\n");
			exit(EXIT_FAILURE); // 直接退出程序，避免数据丢失或后续操作导致更严重的错误
		}
		printf(">>> 错误: 无法创建或打开保存文件！\n");
		return;
	}

	fprintf(fp, "# HIS DRUG DATA FILE\n");

	Drug* curr = sys->drugHead;
	while (curr != NULL) {
		fprintf(fp, "%s %s %s %s %s %d %.2f\n",
			curr->drugId, curr->drugGbCode, 
			curr->genericName, curr->tradeName, curr->alias, 
			curr->stock, curr->price);
		curr = curr->next;
	}
	fclose(fp);
	printf(">>> 药品数据保存成功！\n");
}


