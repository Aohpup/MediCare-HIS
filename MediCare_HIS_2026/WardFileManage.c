#define _CRT_SECURE_NO_WARNINGS
#include"WardManage.h"
#include"WardFileManage.h"
#include<string.h>

// 文件格式：
// W wardId wardType department
// B bedId isOccupied patientId
// END  (结束当前病房)

//从文件加载病房数据到系统
void loadWardData(HIS_System* sys) {
	printf("正在从病房文件中加载数据...\n");
	FILE* fp = fopen(WARD_FILE, "r");
	if (!fp) {
		printf(">>> 警告: 找不到 %s，将作为新系统启动。\n", WARD_FILE);
		return;
	}
	char tag[8];
	while (fscanf(fp, "%7s", tag) == 1) {
		if (strcmp(tag, "W") == 0) {
			Ward* ward = (Ward*)malloc(sizeof(Ward));
			if (!ward) { printf(">>> 内存分配失败，停止加载！\n"); break; }
			int t = 0;
			if (fscanf(fp, "%s %d %s", ward->wardId, &t, ward->department) != 3) { free(ward); break; }
			ward->type = (WardType)t;
			ward->bedListHead = NULL;
			ward->next = sys->wardHead;
			sys->wardHead = ward;
			// 读取床位
			long pos;
			while (1) {
				pos = ftell(fp);
				if (fscanf(fp, "%7s", tag) != 1) break;
				if (strcmp(tag, "B") == 0) {
					Bed* bed = (Bed*)malloc(sizeof(Bed));
					if (!bed) { printf(">>> 床位内存分配失败，停止加载！\n"); break; }
					int occ = 0;
					if (fscanf(fp, "%s %d %s", bed->bedId, &occ, bed->patient) != 3) { free(bed); break; }
					bed->isOccupied = (occ != 0);
					bed->next = ward->bedListHead;
					ward->bedListHead = bed;
				}
				else if (strcmp(tag, "END") == 0) {
					break;
				}
				else {
					fseek(fp, pos, SEEK_SET);
					break;
				}
			}
		}
	}
	fclose(fp);
	printf(">>> 数据加载完成！\n");
}

//保存病房数据到文件
void saveWardData(HIS_System* sys) {
	FILE* fp = fopen(WARD_FILE, "w");
	if (!fp) {
		printf(">>> 错误: 无法打开 %s 进行写入！\n", WARD_FILE);
		return;
	}
	Ward* ward = sys->wardHead;
	while (ward) {
		fprintf(fp, "W %s %d %s\n", ward->wardId, ward->type, ward->department);
		Bed* bed = ward->bedListHead;
		while (bed) {
			fprintf(fp, "B %s %d %s\n", bed->bedId, bed->isOccupied ? 1 : 0, bed->isOccupied ? bed->patient : "-");
			bed = bed->next;
		}
		fprintf(fp, "END\n");
		ward = ward->next;
	}
	fclose(fp);
	printf(">>> 病房数据保存完成！\n");
}
