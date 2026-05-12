#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""HIS 测试数据集生成脚本。在 test_data_01/02/03 中生成完整数据文件。"""

import os, random, shutil

random.seed(42)

BASE_DIR = r"D:\C_code\C_FILES_test\MediCare_HIS_2026\MediCare_HIS_2026"
SRC_DIR  = BASE_DIR  # 固定文件来源

# ======================= 固定文件 =======================
FIXED_FILES = [
    "HIS_departments.txt", "HIS_drugs.txt", "HIS_exam_items.txt",
    "HIS_wards.txt", "HIS_admin.txt", "HIS_status.txt"
]

# ======================= 科室/诊室映射 =======================
# 从 HIS_departments.txt 解析
DEPT_ROOMS = {
    "内科":   ["心内科|A123","心内科|A456","心内科|A999","心内科|A189"],
    "儿科":   ["儿内科|C400","儿内科|C300","儿内科|C200","儿内科|C100"],
    "外科":   ["普外科|B100"],
    "测试科": ["时间测试|T111","顺序测试|T222"],
    "急诊科": ["急诊内科|E990","急诊外科|E991","急诊儿科|E992","急诊神经科|E993","急诊测试科|E999"],
}

ALL_ROOMS = []
for dept, rooms in DEPT_ROOMS.items():
    for r in rooms:
        sub, rid = r.split("|")
        ALL_ROOMS.append((dept, sub, rid))

# 病房映射 wardId -> (dept, price, bedIds)
WARD_INFO = {
    "P101":("眼科",200.00,["P10101","P10102"]), "P102":("外科",250.00,["P10201","P10202"]),
    "P103":("儿科",180.00,["P10301","P10302"]), "P104":("内科",220.00,["P10401","P10402"]),
    "P105":("眼科",200.00,["P10501","P10502"]), "P106":("外科",250.00,["P10601","P10602"]),
    "P107":("儿科",180.00,["P10701","P10702"]), "P108":("内科",220.00,["P10801","P10802"]),
    "P109":("眼科",200.00,["P10901","P10902"]), "P110":("外科",250.00,["P11001","P11002"]),
    "P111":("儿科",180.00,["P11101","P11102"]), "P112":("内科",220.00,["P11201","P11202"]),
    "P113":("眼科",200.00,["P11301","P11302"]), "P114":("外科",250.00,["P11401","P11402"]),
    "P115":("儿科",180.00,["P11501","P11502"]), "P116":("内科",220.00,["P11601","P11602"]),
    "P117":("眼科",200.00,["P11701","P11702"]), "P118":("外科",250.00,["P11801","P11802"]),
    "P119":("儿科",180.00,["P11901","P11902"]), "P120":("内科",220.00,["P12001","P12002"]),
    "P121":("内科",600.00,["P12101"]), "P122":("外科",600.00,["P12201"]),
    "P123":("儿科",600.00,["P12301"]),
    "P200":("测试科",200.00,["P20001"]),
    "P201":("测试科",200.00,["P20101","P20202"]),
    "P202":("测试科",100.00,["P20201","P20202"]),
    "P203":("测试科",99.00,["P20301","P20302"]),
    "P204":("测试科",99.00,["P20401"]),
}

WARDS_BY_DEPT = {}
for wid, (dept, price, beds) in WARD_INFO.items():
    WARDS_BY_DEPT.setdefault(dept, []).append((wid, price, beds))

DRUGS = [
    ("DRG001",25.50),("DRG002",32.80),("DRG003",18.00),("DRG004",45.00),("DRG005",22.50),
    ("DRG006",65.00),("DRG007",38.00),("DRG008",42.00),("DRG009",98.00),("DRG010",35.00),
    ("DRG011",5.00), ("DRG012",15.00),("DRG013",28.00),("DRG014",22.00),("DRG015",12.00),
    ("DRG016",9.00), ("DRG017",58.00),("DRG018",36.00),("DRG019",26.00),("DRG020",8.00),
]

EXAM_ITEMS = [
    ("E001","血常规",25.00),("E002","尿常规",20.00),("E003","生化全套",120.00),
    ("E004","心电图",35.00),("E005","胸部X光",60.00),("E006","腹部B超",80.00),
    ("E007","CT平扫",260.00),("E008","MRI平扫",480.00),("E009","肝功能",55.00),
    ("E010","肾功能",55.00),
]

NAMES_CN = ["张伟","李娜","王芳","刘洋","陈静","杨勇","赵敏","黄强","周洁","吴超",
            "徐蕾","孙涛","马丽","朱军","胡雪","林峰","何萍","郭磊","高婷","罗浩",
            "梁宇","宋雨","唐明","韩冰","冯鑫","董洁","程远","曹磊","袁梅","邓亮",
            "许华","傅蓉","沈刚","曾瑞","彭丽","吕凯","苏瑶","蒋文","蔡颖","贾旭",
            "丁琳","魏然","薛阳","范玲","方旭","石婷","姚飞","谭敏","廖平","邹波",
            "熊艳","金鹏","陆娟","郝杰","段菲","雷明","龙华","万芳","贺强","顾莉",
            "乔宇","秦雪","武峰","钱鑫","陶然","白蕾","崔勇","康欢","邱亮","江涛",
            "史琪","侯敏","邵宇","孟琴","尹飞","戴琳","汤超","田雨","潘旭","任静"]

# ======================= 工具函数 =======================
PID_CTR = [10000000]
def next_pid():
    PID_CTR[0] += 1
    return f"P{PID_CTR[0]}"

DID_CTR = [0]
def next_did():
    DID_CTR[0] += 1
    return f"DOC{DID_CTR[0]:03d}"

XID_CTR = [0]
def next_xid():
    XID_CTR[0] += 1
    return f"X{XID_CTR[0]:07d}"

def rand_date(base="2026-05-01", max_offset=30):
    from datetime import datetime, timedelta
    d = datetime.strptime(base, "%Y-%m-%d") + timedelta(days=random.randint(0, max_offset))
    return d.strftime("%Y-%m-%d")

def add_days(date_str, n):
    from datetime import datetime, timedelta
    return (datetime.strptime(date_str, "%Y-%m-%d") + timedelta(days=n)).strftime("%Y-%m-%d")

def days_between(d1, d2):
    from datetime import datetime
    return (datetime.strptime(d2, "%Y-%m-%d") - datetime.strptime(d1, "%Y-%m-%d")).days

SLOTS_DAYTIME = list(range(1,18))  # 1-17 daytime slots
SLOT_NIGHT   = 18

# ======================= 输出辅助 =======================
class FileWriter:
    def __init__(self, folder):
        self.folder = folder
        os.makedirs(folder, exist_ok=True)
    def write(self, fname, content):
        path = os.path.join(self.folder, fname)
        with open(path, "w", encoding="utf-8") as f:
            f.write(content)
    def copy_fixed(self, fname):
        src = os.path.join(SRC_DIR, fname)
        dst = os.path.join(self.folder, fname)
        shutil.copyfile(src, dst)

# ======================= 场景一：正常常规数据 =======================
def gen_scenario_01():
    fw = FileWriter(os.path.join(BASE_DIR, "test_data_01"))
    for fn in FIXED_FILES:
        fw.copy_fixed(fn)

    # --- 医生 30 名 ---
    doctors = []
    # 急诊科 5 名 (只排晚间)
    for i in range(5):
        did = next_did()
        dept = "急诊科"
        sub, rid = random.choice(DEPT_ROOMS[dept]).split("|")
        doctors.append((did, random.choice(NAMES_CN), dept, sub, rid, True))
    # 其他科室 25 名
    for i in range(25):
        dept = random.choice(["内科","外科","儿科","测试科"])
        sub, rid = random.choice(DEPT_ROOMS[dept]).split("|")
        doctors.append((next_did(), random.choice(NAMES_CN), dept, sub, rid, False))

    doc_lines = ["# HIS doctor DATA FILE"]
    for did, name, dept, sub, rid, is_er in doctors:
        doc_lines.append(f"D {did} {name} {dept} {sub} {rid}")
        doc_lines.append(f"P {did}@new")
        date = rand_date("2026-05-08", 7)
        if is_er:
            doc_lines.append(f"S {date} 18 0")
        else:
            slots = random.sample(SLOTS_DAYTIME, random.randint(3, 8))
            for s in sorted(slots):
                doc_lines.append(f"S {date} {s} {random.randint(0,5)}")
        doc_lines.append("END")
    fw.write("HIS_doctors.txt", "\n".join(doc_lines) + "\n")

    # --- 患者 30 名 ---
    patients = []
    used_ids = set()
    for i in range(30):
        pid = next_pid()
        used_ids.add(pid)
        name = random.choice(NAMES_CN)
        gender = random.choice(["男","女"])
        age = random.randint(18, 80)
        ptype = random.choices([0,1,2], weights=[20,5,5])[0]  # 普通/VIP/急诊
        bal = round(random.uniform(-1500, 10000), 2)
        bonus = round(random.uniform(0, 500), 2) if random.random() < 0.3 else 0.0
        patients.append((pid, name, gender, age, ptype, bal, bonus))
    # 按pid排序
    patients.sort(key=lambda x: x[0])

    # 收集所有挂号记录用于队列文件
    all_tickets = []
    all_exam_orders = []
    total_rev = 0.0
    total_exp = 0.0

    pat_lines = ["# HIS PATIENT DATA FILE"]
    sign_seq = [0]
    for pid, name, gender, age, ptype, bal, bonus in patients:
        phone = f"{random.randint(100,999)}{random.randint(100,999)}"
        idcard = f"{random.randint(100,999)}{random.randint(100000,999999)}{random.randint(100,999)}"
        login_count = random.randint(0, 10)
        pat_lines.append(f"P {pid} {name} {phone} {idcard} {gender} {age} {ptype} {bal:.2f} {bonus:.2f} {login_count}")

        # 挂号记录 0-3 条
        n_reg = random.randint(0, 3)
        reg_docs = random.sample([d for d in doctors if not d[5]], min(n_reg, len([d for d in doctors if not d[5]]))) if n_reg > 0 else []
        for did, dname, ddept, sub, rid, is_er in reg_docs:
            date = rand_date("2026-05-08", 7)
            slot = random.choice([s for s in SLOTS_DAYTIME if s not in (8,9,10,11)])
            rid_str = f"RP{pid[1:]}{random.choice(DEPT_ROOMS[ddept])[1][:3]}"
            pat_lines.append(f"R {rid_str} {ddept} {did} {date} {slot//2:02d}:{('00' if slot%2==1 else '30')}-{(slot+1)//2:02d}:{('00' if (slot+1)%2==1 else '30')}")

            # 票的状态多样
            status = random.choice([0,1,2,3])
            is_onsite = 1 if random.random() < 0.7 else 0
            checked = 1 if status >= 1 else 0
            sign_seq[0] += 1
            late = 0
            if status >= 1:
                late = random.choice([0,0,0,0,30,30,60])
            prio = random.choice([0,0,0,0,0,4])  # 偶尔有优先标记
            all_tickets.append((pid, did, date, slot, is_onsite, checked, sign_seq[0], late, status, prio))

        # 看诊记录 0-2 条
        n_view = random.randint(0, 2)
        for _ in range(n_view):
            did = random.choice(doctors)[0]
            date = rand_date("2026-05-08", 7)
            vid = f"V{random.randint(1,9999):07d}"
            detail = random.choice(["常规检查","复查","体检","头痛","咳嗽","腹痛","发热","高血压随访","糖尿病复查"])
            pat_lines.append(f"{vid}|{date}|{did}|{detail}")

        # 处方记录 0-2 条
        n_med = random.randint(0, 2)
        for _ in range(n_med):
            did = random.choice(doctors)[0]
            date = rand_date("2026-05-08", 7)
            mid = f"M{random.randint(1,9999):07d}"
            nd = random.randint(1, 4)
            items = []
            cost = 0.0
            for _ in range(nd):
                drg, price = random.choice(DRUGS)
                qty = random.randint(1, 5)
                items.append(f"{drg}:{drg}:{qty}")
                cost += price * qty
            pat_lines.append(f"{mid}|{date}|{did}|{';'.join(items)}")
            total_rev += cost

        # 住院记录 0-1 条
        if random.random() < 0.25:
            wdept = random.choice(["内科","外科","儿科","测试科"])
            if wdept in WARDS_BY_DEPT:
                wid, wprice, beds = random.choice(WARDS_BY_DEPT[wdept])
                bed = random.choice(beds)
                sdate = rand_date("2026-05-05", 10)
                days = random.randint(1, 10)
                edate = add_days(sdate, days)
                dur = f"{days}天"
                dept_info = f"科室[{wdept}]"
                did = random.choice(doctors)[0]
                sid = f"S{pid[1:]}{random.randint(1,9999):04d}"
                da = 1  # dischargeApproved
                icd = edate  # isChargeDate
                pat_lines.append(f"S {sid}|{sdate}|{dur}|{edate}|{dept_info}|{did}|{wid}|{bed}|{da}|{icd}")
                total_rev += min(days, 7) * wprice  # 押金已扣
                if days > 7:
                    total_rev += (days - 7) * wprice

        pat_lines.append("END")
    fw.write("HIS_patients.txt", "\n".join(pat_lines) + "\n")

    # --- 排队挂号文件 ---
    tkt_lines = ["# HIS QUEUE TICKET DATA FILE", "# T patientId doctorId date slot isOnsite checkedIn signSeq lateMinutes status priorityOrder"]
    for t in all_tickets:
        tkt_lines.append(f"T {t[0]} {t[1]} {t[2]} {t[3]} {t[4]} {t[5]} {t[6]} {t[7]} {t[8]} {t[9]}")
    fw.write("HIS_queue_tickets.txt", "\n".join(tkt_lines) + "\n")

    # --- 检查申请 10~15 条 ---
    exam_lines = ["# HIS EXAM ORDER DATA FILE"]
    n_exam = random.randint(10, 15)
    for _ in range(n_exam):
        xid = next_xid()
        pid = random.choice(patients)[0]
        did = random.choice(doctors)[0]
        date = rand_date("2026-05-08", 7)
        status = random.choice(["已申请","已执行","已完成","已完成","已完成"])
        exam_lines.append(f"O {xid} {pid} {did} {date} {status}")
        n_items = random.randint(1, 4)
        items = random.sample(EXAM_ITEMS, n_items)
        for eid, ename, eprice in items:
            done = 1 if status in ("已执行","已完成") else 0
            exam_lines.append(f"D {eid} {ename} {eprice:.2f} {done}")
            if done:
                exam_lines.append(f"R 检查结果：{ename}各项指标正常。参考范围无异常。")
            else:
                exam_lines.append("R ")
        exam_lines.append("END")
    fw.write("HIS_exam_orders.txt", "\n".join(exam_lines) + "\n")

    # --- 财务文件 ---
    total_exp = round(total_rev * random.uniform(0.3, 0.7), 2)
    fw.write("HIS_finance.txt", f"R {total_rev:.2f}\nE {total_exp:.2f}\n")

    print(f"test_data_01: {len(patients)} patients, {len(doctors)} doctors, {len(all_tickets)} tickets, {n_exam} exams")

# ======================= 场景二：边界与异常数据 =======================
def gen_scenario_02():
    fw = FileWriter(os.path.join(BASE_DIR, "test_data_02"))
    for fn in FIXED_FILES:
        fw.copy_fixed(fn)

    # 医生 20 名，包含午休时段和晚间
    doctors = []
    for i in range(5):
        did = next_did()
        dept = "急诊科"
        sub, rid = random.choice(DEPT_ROOMS[dept]).split("|")
        doctors.append((did, random.choice(NAMES_CN), dept, sub, rid, True))
    for i in range(15):
        dept = random.choice(["内科","外科","儿科","测试科"])
        sub, rid = random.choice(DEPT_ROOMS[dept]).split("|")
        doctors.append((next_did(), random.choice(NAMES_CN), dept, sub, rid, False))

    doc_lines = ["# HIS doctor DATA FILE"]
    for did, name, dept, sub, rid, is_er in doctors:
        doc_lines.append(f"D {did} {name} {dept} {sub} {rid}")
        doc_lines.append(f"P {did}@new")
        date = rand_date("2026-05-08", 7)
        if is_er:
            doc_lines.append(f"S {date} 18 {random.randint(0,3)}")
        else:
            # 午休时段 (8-11) 和白天时段
            slots = random.sample([8,9,10,11] + SLOTS_DAYTIME, random.randint(5, 10))
            for s in sorted(set(slots)):
                doc_lines.append(f"S {date} {s} {random.randint(0,5)}")
        doc_lines.append("END")
    fw.write("HIS_doctors.txt", "\n".join(doc_lines) + "\n")

    # 患者 20 名，极端余额
    patients = []
    # 特定余额设计
    balance_plan = [
        ("余额临界1","男",30,0,-1999.00,0), ("余额临界2","女",25,0,-2000.00,0), ("余额临界3","男",40,0,-2001.00,0),
        ("高余额测试","女",35,2,50000.00,0), ("零余额1","男",22,0,0.00,0), ("零余额2","女",28,0,0.00,0),
        ("VIP欠费","男",45,2,-500.00,200.00), ("急诊欠费","女",33,3,-1800.00,0),
        ("大量赠送","男",50,2,1000.00,3000.00), ("纯赠送","女",20,0,0.00,500.00),
    ]
    for name, gender, age, ptype, bal, bonus in balance_plan:
        pid = next_pid()
        patients.append((pid, name, gender, age, ptype, bal, bonus))
    for i in range(10):  # 补足到20
        pid = next_pid()
        bal = round(random.uniform(-3000, 8000), 2)
        bonus = round(random.uniform(0, 200), 2) if random.random() < 0.3 else 0.0
        patients.append((pid, random.choice(NAMES_CN), random.choice(["男","女"]),
                        random.randint(18,75), random.choice([0,1,2,3]), bal, bonus))
    patients.sort(key=lambda x: x[0])

    all_tickets = []
    sign_seq = [0]
    pat_lines = ["# HIS PATIENT DATA FILE"]

    # 住院边界场景设计
    stay_scenarios = []  # (pid, startDate, days, dept)
    dept_list = ["内科","外科","儿科","测试科"]
    for i, (pid, name, gender, age, ptype, bal, bonus) in enumerate(patients[:10]):
        if i < 3:
            stay_scenarios.append((pid, "2026-05-10", 0, dept_list[i%4]))   # 刚入院（未出院）
        elif i == 3:
            stay_scenarios.append((pid, "2026-05-10", 1, dept_list[i%4]))   # 住院1天即出院
        elif i == 4:
            stay_scenarios.append((pid, "2026-05-03", 7, dept_list[i%4]))   # 住院正好7天
        elif i == 5:
            stay_scenarios.append((pid, "2026-05-03", 8, dept_list[i%4]))   # 住院8天（超过押金）
        elif i == 6:
            stay_scenarios.append((pid, "2026-05-01", 14, dept_list[i%4]))   # 住院14天
    # 另加几个随机住院
    for i in range(3):
        pid = patients[random.randint(0, len(patients)-1)][0]
        sdate = rand_date("2026-05-01", 12)
        days = random.choice([0, 1, 3, 6, 7, 10, 15])
        wdept = dept_list[random.randint(0,3)]
        stay_scenarios.append((pid, sdate, days, wdept))

    total_rev = 0.0
    for pid, name, gender, age, ptype, bal, bonus in patients:
        phone = f"{random.randint(100,999)}{random.randint(100,999)}"
        idcard = f"{random.randint(100,999)}{random.randint(100000,999999)}{random.randint(100,999)}"
        login_count = random.randint(0, 5)
        pat_lines.append(f"P {pid} {name} {phone} {idcard} {gender} {age} {ptype} {bal:.2f} {bonus:.2f} {login_count}")

        # 住院记录
        for spid, sdate, sdays, sdept in stay_scenarios:
            if spid == pid:
                if sdept in WARDS_BY_DEPT:
                    wid, wprice, beds = random.choice(WARDS_BY_DEPT[sdept])
                    bed = random.choice(beds)
                    dept_info = f"科室[{sdept}]"
                    did = random.choice(doctors)[0]
                    sid = f"S{pid[1:]}{random.randint(1,9999):04d}"

                    if sdays == 0:  # 刚入院，未出院
                        edate = "未出院"
                        dur = "待定"
                        da = 0
                        icd = ""
                        # 押金已扣
                        total_rev += 7 * wprice
                    else:
                        edate = add_days(sdate, sdays)
                        dur = f"{sdays}天"
                        da = 1
                        icd = edate
                        total_rev += min(sdays, 7) * wprice
                        if sdays > 7:
                            total_rev += (sdays - 7) * wprice
                    pat_lines.append(f"S {sid}|{sdate}|{dur}|{edate}|{dept_info}|{did}|{wid}|{bed}|{da}|{icd}")

        # 挂号记录 - 以迟到测试为主
        for (did, dname, ddept, sub, rid, is_er), late_min in [
            (random.choice([d for d in doctors if not d[5]]), 0),
            (random.choice([d for d in doctors if not d[5]]), 30),
            (random.choice([d for d in doctors if not d[5]]), 60),
        ]:
            if random.random() < 0.7:
                date = rand_date("2026-05-08", 5)
                slot = random.choice([s for s in SLOTS_DAYTIME if s not in (8,9,10,11)])
                rid_str = f"RP{pid[1:]}{random.choice(DEPT_ROOMS.get(ddept,[['A123']]))[1][:3]}"
                pat_lines.append(f"R {rid_str} {ddept} {did} {date} {slot//2:02d}:{('00' if slot%2==1 else '30')}-{(slot+1)//2:02d}:{('00' if (slot+1)%2==1 else '30')}")
                sign_seq[0] += 1
                status = 1 if late_min == 0 else (1 if random.random() < 0.5 else 0)
                is_onsite = 1 if random.random() < 0.6 else 0
                all_tickets.append((pid, did, date, slot, is_onsite, 1 if late_min==0 else (1 if status==1 else 0),
                                   sign_seq[0], late_min, status, 0))

        pat_lines.append("END")
    fw.write("HIS_patients.txt", "\n".join(pat_lines) + "\n")

    # 队列文件
    tkt_lines = ["# HIS QUEUE TICKET DATA FILE", "# T patientId doctorId date slot isOnsite checkedIn signSeq lateMinutes status priorityOrder"]
    # 增加模拟迟到>120分钟被拒绝的测试记录
    for pid in [p[0] for p in patients[:3]]:
        did = random.choice(doctors)[0]
        date = rand_date("2026-05-08", 3)
        slot = random.choice([s for s in SLOTS_DAYTIME if s not in (8,9,10,11)])
        sign_seq[0] += 1
        all_tickets.append((pid, did, date, slot, 0, 0, sign_seq[0], 125, 0, 0))
    for t in all_tickets:
        tkt_lines.append(f"T {t[0]} {t[1]} {t[2]} {t[3]} {t[4]} {t[5]} {t[6]} {t[7]} {t[8]} {t[9]}")
    fw.write("HIS_queue_tickets.txt", "\n".join(tkt_lines) + "\n")

    # 检查申请 - 包含超长结果、空结果、未完成
    exam_lines = ["# HIS EXAM ORDER DATA FILE"]
    for i in range(8):
        xid = next_xid()
        pid = random.choice(patients)[0]
        did = random.choice(doctors)[0]
        date = rand_date("2026-05-08", 5)
        status = random.choice(["已申请","已申请","已执行","已完成"])
        exam_lines.append(f"O {xid} {pid} {did} {date} {status}")
        n_items = random.randint(1, 3)
        items = random.sample(EXAM_ITEMS, n_items)
        for eid, ename, eprice in items:
            done = 1 if status == "已完成" else (0 if status == "已申请" else random.choice([0,1]))
            exam_lines.append(f"D {eid} {ename} {eprice:.2f} {done}")
            if done:
                if random.random() < 0.3:  # 超长结果
                    exam_lines.append(f"R 检查项目{ename}：经过详细检测，各项指标均在正常参考范围内。患者无需特殊处理，建议保持健康生活方式，定期体检复查。如有不适请及时就医。本次检查结果可作为临床诊断参考依据。")
                else:
                    exam_lines.append(f"R {ename}：正常。参考范围无异常。")
            else:
                exam_lines.append("R ")
        exam_lines.append("END")
    fw.write("HIS_exam_orders.txt", "\n".join(exam_lines) + "\n")

    total_exp = round(total_rev * 0.5, 2)
    fw.write("HIS_finance.txt", f"R {total_rev:.2f}\nE {total_exp:.2f}\n")
    print(f"test_data_02: {len(patients)} patients, {len(doctors)} doctors, {len(all_tickets)} tickets, 8 exams")

# ======================= 场景三：较大规模压力数据 =======================
def gen_scenario_03():
    fw = FileWriter(os.path.join(BASE_DIR, "test_data_03"))
    for fn in FIXED_FILES:
        fw.copy_fixed(fn)

    # 医生 40-50 名
    n_docs = random.randint(40, 50)
    doctors = []
    for i in range(5):
        did = next_did()
        dept = "急诊科"
        sub, rid = random.choice(DEPT_ROOMS[dept]).split("|")
        doctors.append((did, random.choice(NAMES_CN), dept, sub, rid, True))
    for i in range(n_docs - 5):
        dept = random.choice(["内科","外科","儿科","测试科"])
        sub, rid = random.choice(DEPT_ROOMS[dept]).split("|")
        doctors.append((next_did(), random.choice(NAMES_CN), dept, sub, rid, False))

    doc_lines = ["# HIS doctor DATA FILE"]
    for did, name, dept, sub, rid, is_er in doctors:
        doc_lines.append(f"D {did} {name} {dept} {sub} {rid}")
        doc_lines.append(f"P {did}@new")
        date = rand_date("2026-05-08", 7)
        if is_er:
            doc_lines.append(f"S {date} 18 {random.randint(0,3)}")
        else:
            slots = random.sample(SLOTS_DAYTIME, random.randint(4, 10))
            for s in sorted(slots):
                doc_lines.append(f"S {date} {s} {random.randint(0,5)}")
        doc_lines.append("END")
    fw.write("HIS_doctors.txt", "\n".join(doc_lines) + "\n")

    # 患者 80-100 名
    n_pats = random.randint(80, 100)
    patients = []
    for i in range(n_pats):
        pid = next_pid()
        name = random.choice(NAMES_CN)
        gender = random.choice(["男","女"])
        age = random.randint(18, 80)
        ptype = random.choices([0,1,2], weights=[20,3,3])[0]
        bal = round(random.uniform(-3000, 15000), 2)
        bonus = round(random.uniform(0, 500), 2) if random.random() < 0.25 else 0.0
        patients.append((pid, name, gender, age, ptype, bal, bonus))
    patients.sort(key=lambda x: x[0])

    all_tickets = []
    sign_seq = [0]
    pat_lines = ["# HIS PATIENT DATA FILE"]
    total_rev = 0.0

    for pid, name, gender, age, ptype, bal, bonus in patients:
        phone = f"{random.randint(100,999)}{random.randint(100,999)}"
        idcard = f"{random.randint(100,999)}{random.randint(100000,999999)}{random.randint(100,999)}"
        login_count = random.randint(0, 10)
        pat_lines.append(f"P {pid} {name} {phone} {idcard} {gender} {age} {ptype} {bal:.2f} {bonus:.2f} {login_count}")

        # 挂号记录
        n_reg = random.randint(1, 3)
        reg_candidates = [d for d in doctors if not d[5]]
        if reg_candidates:
            for _ in range(n_reg):
                did, dname, ddept, sub, rid, is_er = random.choice(reg_candidates)
                date = rand_date("2026-05-05", 12)
                slot = random.choice([s for s in SLOTS_DAYTIME if s not in (8,9,10,11)])
                rid_str = f"RP{pid[1:]}{random.choice(DEPT_ROOMS.get(ddept, [['A123']]))[1][:3]}"
                pat_lines.append(f"R {rid_str} {ddept} {did} {date} {slot//2:02d}:{('00' if slot%2==1 else '30')}-{(slot+1)//2:02d}:{('00' if (slot+1)%2==1 else '30')}")
                sign_seq[0] += 1
                status = random.choices([0,1,2,3], weights=[2,3,2,3])[0]
                late = random.choices([0,0,0,0,30,60,120], weights=[6,2,1,1,1,1,1])[0]
                checked = 1 if status >= 1 else 0
                all_tickets.append((pid, did, date, slot, random.choice([0,1]), checked, sign_seq[0], late, status, random.choice([0,0,0,4])))

        # 处方记录
        if random.random() < 0.4:
            did = random.choice(doctors)[0]
            date = rand_date("2026-05-05", 12)
            mid = f"M{random.randint(1,9999):07d}"
            nd = random.randint(1, 3)
            items = []
            cost = 0.0
            for _ in range(nd):
                drg, price = random.choice(DRUGS)
                qty = random.randint(1, 6)
                items.append(f"{drg}:{drg}:{qty}")
                cost += price * qty
            pat_lines.append(f"{mid}|{date}|{did}|{';'.join(items)}")
            total_rev += cost

        # 看诊记录
        if random.random() < 0.3:
            did = random.choice(doctors)[0]
            date = rand_date("2026-05-05", 12)
            vid = f"V{random.randint(1,9999):07d}"
            pat_lines.append(f"{vid}|{date}|{did}|{random.choice(['体检','复查','随访','常规检查','急诊'])}")

        # 住院记录 (约25%有)
        if random.random() < 0.25:
            wdept = random.choice(["内科","外科","儿科","测试科"])
            if wdept in WARDS_BY_DEPT:
                wid, wprice, beds = random.choice(WARDS_BY_DEPT[wdept])
                bed = random.choice(beds)
                sdate = rand_date("2026-05-01", 15)
                days = random.randint(1, 20)
                if random.random() < 0.1:  # 10%未出院
                    edate = "未出院"
                    dur = "待定"
                    da = 0
                    icd = ""
                    total_rev += 7 * wprice
                else:
                    edate = add_days(sdate, days)
                    dur = f"{days}天"
                    da = 1
                    icd = edate
                    total_rev += min(days, 7) * wprice
                    if days > 7:
                        total_rev += (days - 7) * wprice
                dept_info = f"科室[{wdept}]"
                did = random.choice(doctors)[0]
                sid = f"S{pid[1:]}{random.randint(1,9999):04d}"
                pat_lines.append(f"S {sid}|{sdate}|{dur}|{edate}|{dept_info}|{did}|{wid}|{bed}|{da}|{icd}")

        pat_lines.append("END")
    fw.write("HIS_patients.txt", "\n".join(pat_lines) + "\n")

    # 队列 150-200 条
    tkt_lines = ["# HIS QUEUE TICKET DATA FILE", "# T patientId doctorId date slot isOnsite checkedIn signSeq lateMinutes status priorityOrder"]
    for t in all_tickets:
        tkt_lines.append(f"T {t[0]} {t[1]} {t[2]} {t[3]} {t[4]} {t[5]} {t[6]} {t[7]} {t[8]} {t[9]}")
    fw.write("HIS_queue_tickets.txt", "\n".join(tkt_lines) + "\n")

    # 检查申请 50+ 条
    exam_lines = ["# HIS EXAM ORDER DATA FILE"]
    n_exam = random.randint(50, 60)
    for _ in range(n_exam):
        xid = next_xid()
        pid = random.choice(patients)[0]
        did = random.choice(doctors)[0]
        date = rand_date("2026-05-01", 20)
        status = random.choice(["已申请","已申请","已执行","已完成","已完成","已完成"])
        exam_lines.append(f"O {xid} {pid} {did} {date} {status}")
        n_items = random.randint(1, 4)
        items = random.sample(EXAM_ITEMS, n_items)
        for eid, ename, eprice in items:
            done = 1 if status in ("已执行","已完成") else 0
            exam_lines.append(f"D {eid} {ename} {eprice:.2f} {done}")
            if done:
                exam_lines.append(f"R 检查结果：{ename}各项指标在正常参考范围内。未见明显异常。")
            else:
                exam_lines.append("R ")
        exam_lines.append("END")
    fw.write("HIS_exam_orders.txt", "\n".join(exam_lines) + "\n")

    total_exp = round(total_rev * random.uniform(0.4, 0.6), 2)
    fw.write("HIS_finance.txt", f"R {total_rev:.2f}\nE {total_exp:.2f}\n")
    print(f"test_data_03: {len(patients)} patients, {len(doctors)} doctors, {len(all_tickets)} tickets, {n_exam} exams")

# ======================= 主程序 =======================
if __name__ == "__main__":
    random.seed(42)
    PID_CTR[0] = 10000000
    DID_CTR[0] = 0
    XID_CTR[0] = 0
    gen_scenario_01()

    PID_CTR[0] = 20000000
    DID_CTR[0] = 100
    XID_CTR[0] = 100
    gen_scenario_02()

    PID_CTR[0] = 30000000
    DID_CTR[0] = 200
    XID_CTR[0] = 200
    gen_scenario_03()
    print("All test data generated successfully!")
