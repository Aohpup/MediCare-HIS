#define _CRT_SECURE_NO_WARNINGS
#include"ExamManage.h"
#include"ExamFileManage.h"
#include"InputUtils.h"
#include"ConfirmFunc.h"
#include"PauseUtil.h"
#include"PrintFormattedStr.h"
#include"StringCheck.h"
#include<string.h>
#include<stdlib.h>
#include<time.h>

// 查找检查项目字典中对应编号的项目
static ExamItem* findExamItem(HIS_System* sys, const char* itemId) {
	ExamItem* curr = sys->examItemHead;
	while (curr != NULL) {
		if (strcmp(curr->itemId, itemId) == 0) {
			return curr;
		}
		curr = curr->next;
	}
	return NULL;
}

// 基于字符串计算简单哈希，用作随机种子（同一患者同一项目结果一致）
static unsigned int hashFromStr(const char* str) {
	unsigned int h = 5381;
	while (*str) {
		h = ((h << 5) + h) + (unsigned char)(*str);
		str++;
	}
	return h;
}

// 生成范围内随机整数
static int randInt(int minVal, int maxVal) {
	return minVal + rand() % (maxVal - minVal + 1);
}

// 生成范围内随机浮点数(一位小数)
static double randDouble(double minVal, double maxVal) {
	double r = (double)rand() / (double)RAND_MAX;
	return minVal + r * (maxVal - minVal);
}

// ------------------------------------------------
// 各检查项目的专用结果生成函数
// ------------------------------------------------

static void genBloodRoutine(ExamOrderItem* item, unsigned int seed) {
	srand(seed);
	double wbc  = randDouble(3.8, 10.2);         // 正常 3.5-9.5
	double rbc  = randDouble(4.0,  5.9);          // 正常 4.3-5.8
	int    hgb  = randInt(122, 182);              // 正常 130-175
	int    plt  = randInt(115, 360);              // 正常 125-350
	const char* verdict = (hgb < 130 || plt < 125) ? "提示轻度异常，建议复查。" : "各项指标在正常参考范围内。";

	sprintf(item->result,
		"WBC:%.1fx10^9/L(3.5-9.5) RBC:%.2fx10^12/L(4.3-5.8) "
		"HGB:%dg/L(130-175) PLT:%dx10^9/L(125-350) %s",
		wbc, rbc, hgb, plt, verdict);
}

static void genUrineRoutine(ExamOrderItem* item, unsigned int seed) {
	srand(seed);
	double ph   = randDouble(5.0, 8.0);
	const char* pro = (rand() % 5 == 0) ? "±" : "阴性";
	const char* glu = (rand() % 6 == 0) ? "+" : "阴性";
	const char* ket = (rand() % 8 == 0) ? "±" : "阴性";
	const char* bil = "阴性";
	const char* comment = (strcmp(pro, "阴性") == 0 && strcmp(glu, "阴性") == 0)
		? "镜检未见明显异常。" : "建议结合临床症状进一步分析。";

	sprintf(item->result,
		"pH:%.1f PRO:%s GLU:%s KET:%s BIL:%s %s",
		ph, pro, glu, ket, bil, comment);
}

static void genBiochemistry(ExamOrderItem* item, unsigned int seed) {
	srand(seed);
	double glu = randDouble(3.8, 6.8);            // 空腹 3.9-6.1
	double alt = randDouble(8.0, 55.0);           // 正常 9-50 U/L
	double ast = randDouble(10.0, 45.0);          // 正常 15-40 U/L
	double cre = randDouble(44.0, 115.0);         // 正常 44-133 umol/L
	double bun = randDouble(2.5, 8.5);            // 正常 2.9-8.2 mmol/L
	const char* verdict = (glu > 6.1 || alt > 50 || ast > 40) ? "部分指标偏高，请结合临床。" : "各项指标均在正常参考范围内。";

	sprintf(item->result,
		"GLU:%.1fmmol/L(3.9-6.1) ALT:%.0fU/L(9-50) AST:%.0fU/L(15-40) "
		"Cr:%.0fumol/L(44-133) BUN:%.1fmmol/L(2.9-8.2) %s",
		glu, alt, ast, cre, bun, verdict);
}

static void genECG(ExamOrderItem* item, unsigned int seed) {
	srand(seed);
	int r = rand() % 10;
	const char* rhythm;
	const char* detail;
	if (r < 7) {
		rhythm = "窦性心律";
		detail = "正常心电图。PR间期正常，QRS波群形态正常，ST-T未见明显异常。";
	}
	else if (r < 9) {
		rhythm = "窦性心律";
		detail = "偶发房性早搏，ST-T轻微改变，建议定期复查。";
	}
	else {
		rhythm = "窦性心律";
		detail = "心动过缓(HR:52bpm)，Q-T间期略延长。";
	}
	sprintf(item->result, "HR:%dbpm 节律: %s %s",
		randInt(62, 88), rhythm, detail);
}

static void genChestXRay(ExamOrderItem* item, unsigned int seed) {
	srand(seed);
	int r = rand() % 10;
	const char* findings;
	if (r < 7) {
		findings = "双肺纹理清晰，肺野未见实质性病变。心影大小形态正常。双膈面光滑，肋膈角锐利。";
	}
	else if (r < 9) {
		findings = "双肺纹理增多增粗，肺门影增浓。心影大小正常范围。建议CT进一步检查。";
	}
	else {
		findings = "右肺中叶可见小结节影(d<5mm)，性质待定，建议随访观察。余肺野清晰。";
	}
	sprintf(item->result, "%s", findings);
}

static void genAbdominalUS(ExamOrderItem* item, unsigned int seed) {
	srand(seed);
	int r = rand() % 10;
	const char* findings;
	if (r < 6) {
		findings = "肝脏大小形态正常，实质回声均匀。胆囊大小正常，壁光滑。脾脏大小正常。双肾大小形态正常，集合系统未见分离。未见明显异常。";
	}
	else if (r < 8) {
		findings = "肝内脂肪浸润(轻度)。胆囊壁毛糙。肝内可见一个无回声区(d<8mm)，考虑肝囊肿。双肾未见异常。";
	}
	else {
		findings = "胆囊内可见泥沙样结石(范围约10mm)。肝脏、脾脏、双肾未见明显异常。";
	}
	sprintf(item->result, "%s", findings);
}

static void genCTScan(ExamOrderItem* item, unsigned int seed) {
	srand(seed);
	int r = rand() % 10;
	const char* findings;
	if (r < 7) {
		findings = "平扫各层面未见占位性病变。脑室系统对称，中线结构居中。脑沟裂池未见增宽。未见明显异常。";
	}
	else if (r < 9) {
		findings = "扫描范围内可见轻度脑白质疏松改变。双侧基底节区可见点状低密度影，考虑陈旧性腔隙性梗死。建议MRI复查。";
	}
	else {
		findings = "右侧基底节区可见片状低密度灶(d≈12mm)，边界欠清，CT值约18HU。建议增强扫描进一步明确。";
	}
	sprintf(item->result, "%s", findings);
}

static void genMRIScan(ExamOrderItem* item, unsigned int seed) {
	srand(seed);
	int r = rand() % 10;
	const char* findings;
	if (r < 7) {
		findings = "T1WI、T2WI、FLAIR序列显示脑实质信号均匀，未见异常信号。脑室系统正常。中线结构居中。MRA示颅内主要血管走行正常。未见明显异常。";
	}
	else if (r < 9) {
		findings = "双侧半卵圆中心可见散在斑点状T2WI/FLAIR高信号(d<5mm)，T1WI呈等信号，考虑缺血性脱髓鞘改变。脑室系统未见异常。";
	}
	else {
		findings = "左侧额叶皮层下可见小片状T2WI/FLAIR高信号(d≈8mm)，DWI未见弥散受限，考虑慢性缺血灶。建议随访。";
	}
	sprintf(item->result, "%s", findings);
}

static void genLiverFunction(ExamOrderItem* item, unsigned int seed) {
	srand(seed);
	double alt = randDouble(8.0, 58.0);
	double ast = randDouble(10.0, 48.0);
	double tbil = randDouble(6.0, 24.0);
	double dbil = randDouble(1.5, 7.5);
	double alb = randDouble(32.0, 52.0);
	const char* verdict = (alt > 50 || ast > 40 || tbil > 21)
		? "部分指标轻度异常，建议复查并戒酒。"
		: "肝功能指标均在正常参考范围内。";

	sprintf(item->result,
		"ALT:%.0fU/L(9-50) AST:%.0fU/L(15-40) "
		"TBIL:%.1fumol/L(5.1-21) DBIL:%.1fumol/L(1.7-6.8) "
		"ALB:%.1fg/L(35-52) %s",
		alt, ast, tbil, dbil, alb, verdict);
}

static void genRenalFunction(ExamOrderItem* item, unsigned int seed) {
	srand(seed);
	double bun = randDouble(2.5, 9.2);
	double cre = randDouble(44.0, 130.0);
	double ua  = randDouble(140.0, 480.0);
	const char* verdict = (bun > 8.2 || cre > 133 || ua > 420)
		? "部分指标偏高，建议低蛋白饮食、多饮水并复查。"
		: "肾功能指标均在正常参考范围内。";

	sprintf(item->result,
		"BUN:%.1fmmol/L(2.9-8.2) Cr:%.0fumol/L(44-133) "
		"UA:%.0fumol/L(150-420) %s",
		bun, cre, ua, verdict);
}

// 根据项目编号分发到对应的生成函数
static bool generateOneExamResult(ExamOrderItem* item, const char* patientId) {
	if (item == NULL || item->finished) {
		return false;
	}

	// 种子由患者ID + 项目编号混合，确保同一患者同一项目结果一致
	unsigned int seed = hashFromStr(patientId) + hashFromStr(item->itemId);
	srand(seed);

	if (strcmp(item->itemId, "E001") == 0)      genBloodRoutine(item, seed);
	else if (strcmp(item->itemId, "E002") == 0) genUrineRoutine(item, seed);
	else if (strcmp(item->itemId, "E003") == 0) genBiochemistry(item, seed);
	else if (strcmp(item->itemId, "E004") == 0) genECG(item, seed);
	else if (strcmp(item->itemId, "E005") == 0) genChestXRay(item, seed);
	else if (strcmp(item->itemId, "E006") == 0) genAbdominalUS(item, seed);
	else if (strcmp(item->itemId, "E007") == 0) genCTScan(item, seed);
	else if (strcmp(item->itemId, "E008") == 0) genMRIScan(item, seed);
	else if (strcmp(item->itemId, "E009") == 0) genLiverFunction(item, seed);
	else if (strcmp(item->itemId, "E010") == 0) genRenalFunction(item, seed);
	else {
		sprintf(item->result, "检查已完成，各项指标在正常参考范围内。");
	}

	item->finished = true;
	return true;
}

// ---------- 检查单查找/状态辅助函数（供 autoGenerateExamResults 及后续函数使用） ----------

static ExamOrder* findExamOrder(HIS_System* sys, const char* orderId) {
	ExamOrder* curr = sys->examOrderHead;
	while (curr != NULL) {
		if (strcmp(curr->orderId, orderId) == 0) {
			return curr;
		}
		curr = curr->next;
	}
	return NULL;
}

static bool hasPendingItems(const ExamOrder* order) {
	ExamOrderItem* item = order->itemHead;
	while (item != NULL) {
		if (!item->finished) {
			return true;
		}
		item = item->next;
	}
	return false;
}

static void updateOrderStatus(ExamOrder* order) {
	if (order == NULL) {
		return;
	}
	if (hasPendingItems(order)) {
		strcpy(order->status, "待执行");
	}
	else {
		strcpy(order->status, "已完成");
	}
}

// 判断日期字符串是否早于今天（YYYY-MM-DD 格式可直接字符串比较）
static bool isBeforeToday(const char* dateStr) {
	const char* today = getCurrentDateStr();
	return strcmp(dateStr, today) < 0;
}

// ============================================================
// 公共接口：自动生成指定检查单的所有待完成项目结果
// ============================================================
int autoGenerateExamResults(HIS_System* sys, const char* orderId) {
	if (sys == NULL || orderId == NULL) {
		return 0;
	}

	ExamOrder* order = sys->examOrderHead;
	while (order != NULL) {
		if (strcmp(order->orderId, orderId) == 0) {
			break;
		}
		order = order->next;
	}
	if (order == NULL) {
		printf(">>> 未找到检查单 %s。\n", orderId);
		return 0;
	}

	int count = 0;
	ExamOrderItem* item = order->itemHead;
	while (item != NULL) {
		if (generateOneExamResult(item, order->patientId)) {
			printf("  [OK] %s (%s) 结果已生成。\n", item->itemName, item->itemId);
			count++;
		}
		item = item->next;
	}

	if (count > 0) {
		// 全部填充完成后更新订单状态
		updateOrderStatus(order);
		saveExamOrderData(sys);
		printf(">>> 检查单 %s 共生成 %d 项结果，状态已更新为 %s。\n",
			orderId, count, order->status);
	}
	else {
		printf(">>> 检查单 %s 所有项目已完成，无需生成。\n", orderId);
	}

	return count;
}

// 生成自增检查单编号 X0000001, X0000002...
// 遍历已有检查单链表找到最大编号号，在其基础上+1
static void generateExamOrderId(HIS_System* sys, char* outId) {
	int maxNum = 0;
	ExamOrder* order = sys->examOrderHead;
	while (order != NULL) {
		int num = 0;
		if (sscanf(order->orderId, "X%d", &num) == 1) {
			if (num > maxNum) maxNum = num;
		}
		order = order->next;
	}
	sprintf(outId, "X%07d", maxNum + 1);
}

bool createExamOrder(HIS_System* sys, const char* doctorId, const char* patientId) {
	if (sys == NULL || doctorId == NULL) {
		return false;
	}
	if (sys->examItemHead == NULL) {
		printf(">>> 当前没有检查项目字典，请先维护检查项目。\n");
		return false;
	}
	char selectedPatientId[ID_LEN];
	if (patientId == NULL) {
		safeGetString(">>> 请输入患者编号(输入 -1 取消): ", selectedPatientId, ID_LEN);
		if (strcmp(selectedPatientId, "-1") == 0) {
			printf(">>> 已取消开具检查单。\n");
			return false;
		}
	}
	else {
		strcpy(selectedPatientId, patientId);
	}

	ExamOrder* order = (ExamOrder*)malloc(sizeof(ExamOrder));	//为检查单分配内存
	if (order == NULL) {
		printf(">>> 内存不足，无法创建检查单。\n");	
		return false;
	}
	memset(order, 0, sizeof(ExamOrder));
	generateExamOrderId(sys, order->orderId);
	strcpy(order->patientId, selectedPatientId);
	strcpy(order->doctorId, doctorId);
	strcpy(order->date, getCurrentDateStr());
	strcpy(order->status, "待执行");
	order->itemHead = NULL;
	order->next = NULL;

	// 收集并打印检查项目序号清单
	ExamItem* allItems[256];
	int totalItems = 0;
	ExamItem* ei = sys->examItemHead;
	while (ei != NULL && totalItems < 256) {
		allItems[totalItems++] = ei;
		ei = ei->next;
	}
	printf("\n--- 可选检查项目清单 ---\n");
	for (int i = 0; i < totalItems; i++) {
		printf("  %d. %s  %s\n", i + 1, allItems[i]->itemId, allItems[i]->itemName);
	}
	printf("------------------------\n");
	printf("输入序号(1-%d)/项目编号 或 \"全选\" 添加全部 (输入 -1 结束)\n", totalItems);

	while (1) {
		char itemId[ID_LEN];
		safeGetString(">>> 项目选择: ", itemId, ID_LEN);
		if (strcmp(itemId, "-1") == 0) {
			break;
		}

		// 全选：添加所有项目
		if (strcmp(itemId, "全选") == 0) {
			int added = 0;
			for (int i = 0; i < totalItems; i++) {
				// 跳过已添加的项目
				bool alreadyAdded = false;
				ExamOrderItem* check = order->itemHead;
				while (check != NULL) {
					if (strcmp(check->itemId, allItems[i]->itemId) == 0) {
						alreadyAdded = true;
						break;
					}
					check = check->next;
				}
				if (alreadyAdded) continue;

				ExamOrderItem* newItem = (ExamOrderItem*)malloc(sizeof(ExamOrderItem));
				if (newItem == NULL) { printf(">>> 内存不足，停止全选。\n"); break; }
				memset(newItem, 0, sizeof(ExamOrderItem));
				strcpy(newItem->itemId, allItems[i]->itemId);
				strcpy(newItem->itemName, allItems[i]->itemName);
				newItem->price = allItems[i]->price;
				newItem->finished = false;
				newItem->next = NULL;
				if (order->itemHead == NULL) {
					order->itemHead = newItem;
				} else {
					ExamOrderItem* tail = order->itemHead;
					while (tail->next != NULL) tail = tail->next;
					tail->next = newItem;
				}
				added++;
			}
			printf(">>> 已全选添加 %d 个项目。\n", added);
			continue;
		}

		// 纯数字 → 按序号选择
		ExamItem* dictItem = NULL;
		if (isAllDigits(itemId)) {
			int idx = atoi(itemId);
			if (idx >= 1 && idx <= totalItems) {
				dictItem = allItems[idx - 1];
			} else {
				printf(">>> 序号超出范围（1-%d），请重试。\n", totalItems);
				continue;
			}
		} else {
			// 按项目编号查找
			dictItem = findExamItem(sys, itemId);
			if (dictItem == NULL) {
				printf(">>> 未找到项目编号 %s，请重试。\n", itemId);
				continue;
			}
		}

		// 检查是否已添加
		bool alreadyAdded = false;
		ExamOrderItem* check = order->itemHead;
		while (check != NULL) {
			if (strcmp(check->itemId, dictItem->itemId) == 0) {
				printf(">>> 项目 [%s] %s 已添加，无需重复选择。\n", dictItem->itemId, dictItem->itemName);
				alreadyAdded = true;
				break;
			}
			check = check->next;
		}
		if (alreadyAdded) continue;

		ExamOrderItem* newItem = (ExamOrderItem*)malloc(sizeof(ExamOrderItem));
		if (newItem == NULL) {
			printf(">>> 内存不足，无法添加项目。\n");
			break;
		}
		memset(newItem, 0, sizeof(ExamOrderItem));
		strcpy(newItem->itemId, dictItem->itemId);
		strcpy(newItem->itemName, dictItem->itemName);
		newItem->price = dictItem->price;
		newItem->finished = false;
		newItem->next = NULL;
		if (order->itemHead == NULL) {
			order->itemHead = newItem;
		}
		else {
			ExamOrderItem* tail = order->itemHead;
			while (tail->next != NULL) {
				tail = tail->next;
			}
			tail->next = newItem;
		}
		printf(">>> 已添加项目 [%s] %s\n", dictItem->itemId, dictItem->itemName);
	}

	if (order->itemHead == NULL) {
		printf(">>> 未选择任何检查项目，已取消开单。\n");
		free(order);
		return false;
	}

	if (sys->examOrderHead == NULL) {
		sys->examOrderHead = order;
	}
	else {
		ExamOrder* tail = sys->examOrderHead;
		while (tail->next != NULL) {
			tail = tail->next;
		}
		tail->next = order;
	}

	saveExamOrderData(sys);
	printf(">>> 检查单 %s 已开具。\n", order->orderId);
	return true;
}

void listPendingExamOrders(HIS_System* sys) {
	if (sys == NULL) {
		return;
	}

	const char* today = getCurrentDateStr();
	bool dataChanged = false;
	int timeoutCount = 0;

	// ---- 第一遍：超时检测 ----
	ExamOrder* order = sys->examOrderHead;
	while (order != NULL) {
		if (isBeforeToday(order->date)
			&& strcmp(order->status, "待执行") == 0
			&& hasPendingItems(order)) {
			strcpy(order->status, "超时封停");
			dataChanged = true;
			timeoutCount++;
		}
		order = order->next;
	}

	if (dataChanged) {
		saveExamOrderData(sys);
		printf(">>> 系统提示：检测到 %d 份超期未完成的检查单，已自动封停。\n", timeoutCount);
	}

	// ---- 第二遍：仅展示今日待执行检查单 ----
	printf("\n--- 今日待执行检查单 [%s] ---\n", today);
	order = sys->examOrderHead;
	bool found = false;
	while (order != NULL) {
		if (hasPendingItems(order)
			&& strcmp(order->date, today) == 0
			&& strcmp(order->status, "待执行") == 0) {
			printExamOrderDetail(order);
			found = true;
		}
		order = order->next;
	}
	if (!found) {
		printf(">>> 今日没有待执行的检查单。\n");
		pressEnterToContinue();
		return;
	}

	// 询问是否批量自动生成结果（仅对今日待完成项）
	if (confirmFunc("自动生成", "以上今日检查单的待完成项结果")) {
		printf("\n>>> 开始自动生成检查结果...\n");
		order = sys->examOrderHead;
		int totalGenerated = 0;
		int ordersProcessed = 0;
		while (order != NULL) {
			if (strcmp(order->date, today) == 0
				&& strcmp(order->status, "待执行") == 0
				&& hasPendingItems(order)) {
				int n = autoGenerateExamResults(sys, order->orderId);
				if (n > 0) {
					totalGenerated += n;
					ordersProcessed++;
				}
			}
			order = order->next;
		}
		printf("\n>>> ===== 自动生成完成 =====\n");
		printf(">>> 共处理 %d 份检查单，生成 %d 项检查结果。\n", ordersProcessed, totalGenerated);
	}
	pressEnterToContinue();
}

bool fillExamResult(HIS_System* sys, const char* orderId, const char* itemId, const char* result) {
	if (sys == NULL || orderId == NULL || itemId == NULL || result == NULL) {
		return false;
	}
	ExamOrder* order = findExamOrder(sys, orderId);
	if (order == NULL) {
		printf(">>> 未找到检查单 %s。\n", orderId);
		return false;
	}
	ExamOrderItem* item = order->itemHead;
	while (item != NULL) {
		if (strcmp(item->itemId, itemId) == 0) {
			strncpy(item->result, result, sizeof(item->result) - 1);
			item->result[sizeof(item->result) - 1] = '\0';
			item->finished = true;
			updateOrderStatus(order);
			saveExamOrderData(sys);
			printf(">>> 已保存检查结果。\n");
			return true;
		}
		item = item->next;
	}
	printf(">>> 检查单中未找到项目编号 %s。\n", itemId);
	return false;
}

// 医生端自动生成检查结果辅助函数（仅 TEST_SYSTEM_DEBUG 模式可用）
static void tryAutoGenerateDoctorOrders(HIS_System* sys, const char* doctorId, const char* today) {
	if (!TEST_SYSTEM_DEBUG) {
		return;
	}

	// 检查是否有今日待完成检查单
	ExamOrder* order = sys->examOrderHead;
	bool hasPendingToday = false;
	while (order != NULL) {
		if (strcmp(order->doctorId, doctorId) == 0
			&& strcmp(order->date, today) == 0
			&& strcmp(order->status, "待执行") == 0
			&& hasPendingItems(order)) {
			hasPendingToday = true;
			break;
		}
		order = order->next;
	}
	if (!hasPendingToday) {
		return;
	}

	if (!confirmFunc("自动生成", "上述今日检查单中所有待完成项的结果")) {
		return;
	}

	printf("\n>>> 开始自动生成检查结果...\n");
	order = sys->examOrderHead;
	int totalGenerated = 0;
	while (order != NULL) {
		if (strcmp(order->doctorId, doctorId) == 0
			&& strcmp(order->date, today) == 0
			&& strcmp(order->status, "待执行") == 0
			&& hasPendingItems(order)) {
			int n = autoGenerateExamResults(sys, order->orderId);
			totalGenerated += n;
		}
		order = order->next;
	}
	printf(">>> 共生成 %d 项检查结果。\n", totalGenerated);
}

// 患者执行检查（模拟到检验科/放射科等科室做检查，由技师出具结果）
// 自动为患者所有待完成的检查单生成结果（超时封停的不再允许补做）
void doPatientExamCheck(HIS_System* sys, const char* patientId) {
	if (sys == NULL || patientId == NULL) {
		printf(">>> 患者未登录，无法执行检查。\n");
		return;
	}

	// ---- 第一遍：超时检测（超期未完成的检查单自动封停为废案）----
	const char* today = getCurrentDateStr();
	bool dataChanged = false;
	int timeoutCount = 0;

	ExamOrder* order = sys->examOrderHead;
	while (order != NULL) {
		if (strcmp(order->patientId, patientId) == 0
			&& isBeforeToday(order->date)
			&& strcmp(order->status, "待执行") == 0
			&& hasPendingItems(order)) {
			strcpy(order->status, "超时封停");
			dataChanged = true;
			timeoutCount++;
		}
		order = order->next;
	}

	if (dataChanged) {
		saveExamOrderData(sys);
		printf(">>> 系统提示：检测到 %d 份超期未完成的检查单，已自动封停为废案。\n", timeoutCount);
	}

	// ---- 第二遍：查找该患者所有待执行的检查单（不限日期，只限 Pending 状态）----
	order = sys->examOrderHead;
	int orderCount = 0;
	int pendingCount = 0;

	// 先统计待执行检查单数量
	while (order != NULL) {
		if (strcmp(order->patientId, patientId) == 0
			&& strcmp(order->status, "待执行") == 0
			&& hasPendingItems(order)) {
			pendingCount++;
		}
		order = order->next;
	}

	if (pendingCount == 0) {
		printf(">>> 暂无待执行的检查项目。\n");
		printf(">>> 请先由医生开具检查单后，再前往对应科室执行检查。\n");
		pressEnterToContinue();
		return;
	}

	// 展示待执行的检查单并请求确认
	printf("\n>>> 您共有 %d 份待执行的检查单：\n", pendingCount);
	order = sys->examOrderHead;
	while (order != NULL) {
		if (strcmp(order->patientId, patientId) == 0
			&& strcmp(order->status, "待执行") == 0
			&& hasPendingItems(order)) {
			printExamOrderDetail(order);
		}
		order = order->next;
	}

	if (!confirmFunc("检查执行", "以上待执行检查单的全部项目")) {
		printf(">>> 已取消检查执行。\n");
		pressEnterToContinue();
		return;
	}

	// 执行检查（自动生成结果）
	printf("\n>>> 开始执行检查...\n");
	order = sys->examOrderHead;
	int totalItems = 0;

	while (order != NULL) {
		if (strcmp(order->patientId, patientId) == 0
			&& strcmp(order->status, "待执行") == 0
			&& hasPendingItems(order)) {
			int n = autoGenerateExamResults(sys, order->orderId);
			if (n > 0) {
				orderCount++;
				totalItems += n;
			}
		}
		order = order->next;
	}

	if (totalItems > 0) {
		printf(">>> 检查完成！共完成 %d 份检查单、%d 项检查项目。\n", orderCount, totalItems);
		printf(">>> 请在「查看检查结果」中查看详细报告。\n");
	}
	pressEnterToContinue();
}

void queryExamOrdersByDoctor(HIS_System* sys, const char* doctorId) {
	if (sys == NULL || doctorId == NULL) {
		return;
	}

	// ---- 超时检测：日期早于今天且状态为 Pending 的检查单，自动标记为超时封停 ----
	const char* today = getCurrentDateStr();
	bool dataChanged = false;
	int timeoutCount = 0;
	ExamOrder* order = sys->examOrderHead;
	while (order != NULL) {
		if (strcmp(order->doctorId, doctorId) == 0) {
			if (isBeforeToday(order->date)
				&& strcmp(order->status, "待执行") == 0
				&& hasPendingItems(order)) {
				strcpy(order->status, "超时封停");
				dataChanged = true;
				timeoutCount++;
			}
		}
		order = order->next;
	}
	if (dataChanged) {
		saveExamOrderData(sys);
		printf(">>> 系统提示：检测到 %d 份超期未完成的检查单，已自动封停。\n", timeoutCount);
	}

	// ---- 子菜单 ----
	int choice;
	while (1) {
		printf("\n===== 检查结果查询 =====\n");
		printf("1. 查看最新检查结果\n");
		printf("2. 按日期搜索\n");
		printf("3. 查看全部历史\n");
		printf("0. 返回上一级\n");
		printf("========================\n");
		choice = safeGetInt("请选择操作: ");

		switch (choice) {
		case 1: {
			// 查找本医生最新的检查单（按日期+编号排序）
			ExamOrder* latest = NULL;
			order = sys->examOrderHead;
			while (order != NULL) {
				if (strcmp(order->doctorId, doctorId) == 0) {
					if (latest == NULL
						|| strcmp(order->date, latest->date) > 0
						|| (strcmp(order->date, latest->date) == 0
							&& strcmp(order->orderId, latest->orderId) > 0)) {
						latest = order;
					}
				}
				order = order->next;
			}
			if (latest == NULL) {
				printf(">>> 暂无您开具的检查单。\n");
			} else {
				printExamOrderDetail(latest);
			}
			pressEnterToContinue();
			break;
		}
		case 2: {
			char date[DATE_STR_LEN];
			safeGetString("请输入搜索日期(YYYY-MM-DD, 输入 -1 取消): ", date, DATE_STR_LEN);
			if (strcmp(date, "-1") == 0) {
				printf(">>> 已取消搜索。\n");
				break;
			}
			printf("\n--- 检查单 [%s] ---\n", date);
			bool found = false;
			order = sys->examOrderHead;
			while (order != NULL) {
				if (strcmp(order->doctorId, doctorId) == 0
					&& strcmp(order->date, date) == 0) {
					printExamOrderDetail(order);
					found = true;
				}
				order = order->next;
			}
			if (!found) {
				printf(">>> 该日期暂无检查单。\n");
			}
			// 仅 TEST_SYSTEM_DEBUG 下对当日待完成检查单提供自动生成入口
			tryAutoGenerateDoctorOrders(sys, doctorId, date);
			pressEnterToContinue();
			break;
		}
		case 3: {
			// 显示全部历史，不限日期
			printf("\n--- 全部检查单 ---\n");
			bool found = false;
			order = sys->examOrderHead;
			while (order != NULL) {
				if (strcmp(order->doctorId, doctorId) == 0) {
					printExamOrderDetail(order);
					found = true;
				}
				order = order->next;
			}
			if (!found) {
				printf(">>> 暂无您开具的检查单。\n");
			}
			pressEnterToContinue();
			break;
		}
		case 0:
			return;
		default:
			printf(">>> 无效选择，请重试。\n");
		}
	}
}

void queryExamOrdersByPatient(HIS_System* sys, const char* patientId) {
	if (sys == NULL || patientId == NULL) {
		return;
	}
	ExamOrder* order = sys->examOrderHead;
	bool found = false;
	while (order != NULL) {
		if (strcmp(order->patientId, patientId) == 0) {
			printExamOrderDetail(order);
			found = true;
		}
		order = order->next;
	}
	if (!found) {
		printf(">>> 暂无该患者的检查单。\n");
	}
	pressEnterToContinue();
}

void printExamOrderDetail(const ExamOrder* order) {
	if (order == NULL) {
		return;
	}
	char buffer[256];
	printf("\n--- 检查单 [%s] 状态:%s ---\n", order->orderId, order->status);
	printf("患者:%s 医生:%s 日期:%s\n", order->patientId, order->doctorId, order->date);
	printf("----------------------------------------------------------------------------\n");
	printFormattedStr("序号", 6);
	printFormattedStr("项目编号", 12);
	printFormattedStr("项目名称", 20);
	printFormattedStr("结果", 30);
	printFormattedStr("完成", 8);
	printf("\n");
	printf("----------------------------------------------------------------------------\n");

	ExamOrderItem* item = order->itemHead;
	int idx = 1;
	while (item != NULL) {
		sprintf(buffer, "%d", idx++);
		printFormattedStr(buffer, 6);
		printFormattedStr(item->itemId, 12);
		printFormattedStr(item->itemName, 20);
		printFormattedStr(item->result[0] == '\0' ? "(未填写)" : item->result, 30);
		printFormattedStr(item->finished ? "是" : "否", 8);
		printf("\n");
		item = item->next;
	}
	printf("----------------------------------------------------------------------------\n");
}
