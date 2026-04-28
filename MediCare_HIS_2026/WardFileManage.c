#define _CRT_SECURE_NO_WARNINGS
#include"WardManage.h"
#include"WardFileManage.h"
#include"WardSort.h"
#include<string.h>
bool is_Ward_File_Loaded = false;	//标记是否加载过病房数据

//TODO:把这个模式推广到医生和科室模块的文件管理中，形成统一的文件格式和读写流程
// 文件格式：
// W wardId wardType department
// B bedId isoccupyupied patientId
// END  (结束当前病房)

//从文件加载病房数据到系统
void loadWardSystemData(HIS_System* sys) {
	if(TEST_SYSTEM_DEBUG)
	printf(">>> 正在从病房文件中加载数据...\n");
	FILE* fp = fopen(WARD_FILE, "r");
	if (!fp) {
		if(!TEST_SYSTEM_DEBUG) {
			printf("严重错误: 病房数据不存在！请确保文件存在或联系管理员。\n");
			exit(EXIT_FAILURE); // 直接退出程序，避免后续操作导致更严重的错误
		}
		printf(">>> 警告: 找不到 %s，将作为新系统启动。\n", WARD_FILE);
		return;
	}

	char dummyLine[512]; // 用于读取和丢弃文件开头的注释行
	fgets(dummyLine, sizeof(dummyLine), fp); // 读取并丢弃第一行注释

	char tag[8] = "0";	//比对行首标签（W/B/END）
	while (fscanf(fp, "%5s", tag) == 1) {	//读标签
		if (strcmp(tag, "W") == 0) {	//病房信息行
			Ward* ward = (Ward*)malloc(sizeof(Ward));
			if (ward == NULL) { printf(">>> 内存分配失败，停止加载！\n"); break; }
			int t = 0;
			if (fscanf(fp, "%s %d %s", ward->wardId, &t, ward->department) != 3) { 
				free(ward);
				printf(">>> 警告: 病房数据格式错误，跳过该条记录。\n");
				continue;
			}
			ward->type = (WardType)t;
			ward->bedListHead = NULL;
			ward->next = sys->wardHead;
			sys->wardHead = ward;
			// 读取床位
			long pos;
			while (1) {
				pos = ftell(fp);   //记录当前位置，以便后续回退
				if (fscanf(fp, "%5s", tag) != 1) break;	//读标签
				if (strcmp(tag, "B") == 0) {	//床位信息行
					Bed* bed = (Bed*)malloc(sizeof(Bed));
					if (bed == NULL) { printf(">>> 床位内存分配失败，停止加载！\n"); break; }
					int occupy = 0;	//占用状态
					if (fscanf(fp, "%s %d %s", bed->bedId, &occupy, bed->patient) != 3) { 
						printf(">>> 警告: 床位数据格式错误，跳过该条记录。\n");
						free(bed);
						continue;
					}
					if (occupy != 0 && occupy != 1) {
						printf(">>> 警告: 床位占用状态错误，跳过该条记录。\n");
						free(bed); 
						continue;
					} //占用状态必须为0或1
					bed->isOccupied = (bool)(occupy != 0);	//转换为布尔值
					
					bed->next = ward->bedListHead;
					ward->bedListHead = bed;
				}
				else if (strcmp(tag, "END") == 0) {
					break;
				}
				else {
					fseek(fp, pos, SEEK_SET);	//标签不匹配，回退到上一个位置，准备读取下一个病房或结束
					break;
				}
			}
		}
	}
	fclose(fp);

	//排序病房内床位链表
	Ward* curr = sys->wardHead;
	while (curr != NULL) {
		sortBedList(&curr->bedListHead);
		curr = curr->next;
	}

	printf(">>> 病房数据加载完成！\n");
	is_Ward_File_Loaded = true;		//标记已加载过病房数据
}

//保存病房数据到文件
void saveWardSystemData(HIS_System* sys) {
	FILE* fp = fopen(WARD_FILE, "w");
	if (!fp) {
		if (!TEST_SYSTEM_DEBUG) {
			printf("严重错误: 无法保存文件！请确保文件权限或联系管理员。\n");
			exit(EXIT_FAILURE); // 直接退出程序，避免数据丢失或后续操作导致更严重的错误
		}
		printf(">>> 错误: 无法打开 %s 进行写入！\n", WARD_FILE);
		return;
	}

	fprintf(fp, "# HIS WARD DATA FILE\n");

	Ward* ward = sys->wardHead;
	while (ward) {
		fprintf(fp, "W %s %d %s\n", ward->wardId, ward->type, ward->department);
		Bed* bed = ward->bedListHead;
		while (bed) {
			fprintf(fp, "B %s %d %s\n", bed->bedId, 
				bed->isOccupied ? 1 : 0,				//占用状态保存为1或0
				bed->isOccupied ? bed->patient : "-"	//如果床位被占用，保存患者编号；否则保存占位符"-"
			);
			bed = bed->next;
		}
		fprintf(fp, "END\n");
		ward = ward->next;
	}
	fclose(fp);
	printf(">>> 病房数据保存完成！\n");
}
