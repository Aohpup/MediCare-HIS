$ErrorActionPreference = "Stop"
$OutputPath = "D:\C_code\C_FILES_test\MediCare_HIS_2026\MediCare_HIS_2026\患者看诊完整流程图.docx"

$word = New-Object -ComObject Word.Application
$word.Visible = $false
$doc = $word.Documents.Add()

$doc.PageSetup.TopMargin = 50
$doc.PageSetup.BottomMargin = 50
$doc.PageSetup.LeftMargin = 50
$doc.PageSetup.RightMargin = 50
$doc.PageSetup.Orientation = 1

$nl = [char]10

function AddBox($l, $t, $w, $h, $txt, $clr) {
    $s = $doc.Shapes.AddShape(1, $l, $t, $w, $h)
    $s.Fill.ForeColor.RGB = $clr
    $s.Line.ForeColor.RGB = 0x333333
    $s.Line.Weight = 1.5
    $s.TextFrame.TextRange.Text = $txt
    $s.TextFrame.TextRange.Font.Name = "微软雅黑"
    $s.TextFrame.TextRange.Font.Size = 9
    $s.TextFrame.TextRange.Font.Bold = $false
    $s.TextFrame.TextRange.ParagraphFormat.Alignment = 1
    $s.TextFrame.WordWrap = $true
    $s.TextFrame.AutoSize = 0
    return $s
}

function AddDmd($l, $t, $w, $h, $txt, $clr) {
    $s = $doc.Shapes.AddShape(4, $l, $t, $w, $h)
    $s.Fill.ForeColor.RGB = $clr
    $s.Line.ForeColor.RGB = 0x333333; $s.Line.Weight = 1.5
    $s.TextFrame.TextRange.Text = $txt
    $s.TextFrame.TextRange.Font.Name = "微软雅黑"
    $s.TextFrame.TextRange.Font.Size = 8
    $s.TextFrame.TextRange.Font.Bold = $false
    $s.TextFrame.TextRange.ParagraphFormat.Alignment = 1
    $s.TextFrame.WordWrap = $true
    return $s
}

function AddRnd($l, $t, $w, $h, $txt, $clr) {
    $s = $doc.Shapes.AddShape(5, $l, $t, $w, $h)
    $s.Fill.ForeColor.RGB = $clr
    $s.Line.ForeColor.RGB = 0x333333; $s.Line.Weight = 1.5
    $s.TextFrame.TextRange.Text = $txt
    $s.TextFrame.TextRange.Font.Name = "微软雅黑"
    $s.TextFrame.TextRange.Font.Size = 8
    $s.TextFrame.TextRange.Font.Bold = $false
    $s.TextFrame.TextRange.ParagraphFormat.Alignment = 1

    $s.TextFrame.WordWrap = $true
    return $s
}

function AddOval($l, $t, $w, $h, $txt, $clr) {
    $s = $doc.Shapes.AddShape(13, $l, $t, $w, $h)
    $s.Fill.ForeColor.RGB = $clr
    $s.Line.ForeColor.RGB = 0x333333; $s.Line.Weight = 1.5
    $s.TextFrame.TextRange.Text = $txt
    $s.TextFrame.TextRange.Font.Name = "微软雅黑"
    $s.TextFrame.TextRange.Font.Size = 10
    $s.TextFrame.TextRange.Font.Bold = $true
    $s.TextFrame.TextRange.ParagraphFormat.Alignment = 1

    return $s
}

function DownArr($x, $y1, $y2) {
    $c = $doc.Shapes.AddConnector(1, $x, $y1, $x, $y2)
    $c.Line.EndArrowheadStyle = 2
    $c.Line.ForeColor.RGB = 0x555555; $c.Line.Weight = 1.5
}

# Colors: RGB values
$Blue   = 0xCCE5FF
$Green  = 0xD4EDDA
$Teal   = 0xD1ECF1
$Orange = 0xFFE5CC
$Red    = 0xF8D7DA
$Purple = 0xE2D9F3
$Yellow = 0xFFF3CD
$Pink   = 0xFFD6E7
$White  = 0xFFFFFF

# ===== TITLE =====
$s = AddBox 50 20 740 35 "MediCare HIS - 患者挂号看诊完整流程图" $White
$s.TextFrame.TextRange.Font.Name = "黑体"
$s.TextFrame.TextRange.Font.Size = 16
$s.TextFrame.TextRange.Font.Bold = $true
$s.Line.Visible = $false

# ===== LAYOUT CONSTANTS =====
$cx = 180; $cw = 180; $w = 160; $h = 45; $sh = 36; $gap = 8
$dx = 400; $dw = 180
$sx = 650; $sw = 140

# ===== COLUMN 1: PATIENT =====
$s = AddRnd 50 70 $cw 28 "[患者端: 患者服务台]" $Blue
$s.TextFrame.TextRange.Font.Bold = $true

$top = 110
$ptop = $top
AddBox ($cx-80) $ptop $w $h ("1. 患者注册" + $nl + "registerPatient") $Blue
$ptop += ($h + $gap); DownArr $cx ($ptop-$gap) $ptop
AddBox ($cx-80) $ptop $w $h ("2. 患者登录" + $nl + "logInPatient") $Blue
$ptop += ($h + $gap); DownArr $cx ($ptop-$gap) $ptop
AddBox ($cx-80) $ptop $w $h ("3. 选择医生" + $nl + "(按科室筛选医生列表)") $Blue
$ptop += ($h + $gap); DownArr $cx ($ptop-$gap) $ptop
AddBox ($cx-80) $ptop $w $h ("4. 选择日期时段" + $nl + "bookQueueTicket") $Blue
$ptop += ($h + $gap); DownArr $cx ($ptop-$gap) $ptop
AddBox ($cx-80) $ptop $w $sh ("5. 挂号单创建" + $nl + "status=WAITING, 写入R记录") $Teal
$ptop += ($sh + $gap); DownArr $cx ($ptop-$gap) $ptop
AddBox ($cx-80) $ptop $w $h ("6. 现场签到" + $nl + "checkInQueueTicket" + $nl + "设置signSeq/g_signSeq++") $Teal
$ptop += ($h + $gap); DownArr $cx ($ptop-$gap) $ptop
AddBox ($cx-80) $ptop $w $sh ("7. 进入候诊队列" + $nl + "refreshSlotQueue" + $nl + "按三级优先级排序") $Teal
$ptop += ($sh + $gap); DownArr $cx ($ptop-$gap) $ptop
AddBox ($cx-80) $ptop $w $h ("8. 等候叫号" + $nl + "STATUS_WAITING") $Yellow

# ===== COLUMN 2: DOCTOR =====
$s = AddRnd 290 70 $cw 28 "[医生端: 医生工作站]" $Green
$s.TextFrame.TextRange.Font.Bold = $true

$dtop = 110
AddBox ($dx-80) $dtop $dw $h ("D1. 查看候诊队列" + $nl + "printSlotQueue") $Green
$dtop += ($h + $gap); DownArr $dx ($dtop-$gap) $dtop
AddBox ($dx-80) $dtop $dw $h ("D2. 排队叫号" + $nl + "doctorCallQueueMenu" + $nl + "callNextPatient") $Green
$dtop += ($h + $gap); DownArr $dx ($dtop-$gap) $dtop
AddBox ($dx-80) $dtop $dw $sh ("D3. 患者被叫号" + $nl + "STATUS_WAITING -> CALLED") $Teal
$dtop += ($sh + $gap); DownArr $dx ($dtop-$gap) $dtop
AddRnd ($dx-80) $dtop $dw $h ("D4. 进入诊室就诊" + $nl + "markTicketAsInRoom" + $nl + "CALLED -> IN_ROOM") $Teal
$dtop += ($h + $gap); DownArr $dx ($dtop-$gap) $dtop
AddBox ($dx-80) $dtop $dw $h ("D5. 查看患者病历" + $nl + "viewMedicalRecordDoc") $Green
$dtop += ($h + $gap); DownArr $dx ($dtop-$gap) $dtop

# Diamond: choice
AddDmd ($dx-70) $dtop 140 50 "D6. 选择操作" $Yellow
$dtop += (50 + $gap)

# Three branches
$la = $dx - 110; $ca = $dx; $ra = $dx + 110
$ltop1 = $dtop; $ctop1 = $dtop; $rtop1 = $dtop

DownArr $la ($dtop-$gap) $ltop1
AddBox ($la-80) $ltop1 $dw $sh ("D7a. 写诊断" + $nl + "writeMedicalRecord" + $nl + "写入V记录") $Orange
$ltop2 = $ltop1 + ($sh + $gap)
DownArr $la ($ltop1+$sh) $ltop2
AddBox ($la-80) $ltop2 $dw $sh ("D7a-2. 开处方" + $nl + "prescribeDrugsForPatient" + $nl + "写入M记录/扣费") $Orange

DownArr $ca ($dtop-$gap) $ctop1
AddBox ($ca-80) $ctop1 $dw $sh ("D7b. 开检查单" + $nl + "issueExaminationOrder" + $nl + "写入O/D记录") $Orange

DownArr $ra ($dtop-$gap) $rtop1
AddBox ($ra-80) $rtop1 $dw $sh ("D7c. 住院管理" + $nl + "doctorWardMenu" + $nl + "分配病房/写S记录") $Orange

# Merge
$dtop_m = $ltop2 + ($sh + $gap)
DownArr $dx ($dtop_m-$gap) $dtop_m
AddBox ($dx-80) $dtop_m $dw $sh ("D8. 结束看诊" + $nl + "endConsultation") $Green
$dtop_f = $dtop_m + ($sh + $gap)
DownArr $dx ($dtop_f-$gap) $dtop_f
AddBox ($dx-80) $dtop_f $dw $h ("D9. 看诊结束" + $nl + "STATUS_IN_ROOM -> FINISHED") $Red

# ===== COLUMN 3: STATE =====
$s = AddRnd 530 70 $cw 28 "[挂号单状态流转]" $Purple
$s.TextFrame.TextRange.Font.Bold = $true

$stop = 110
AddOval ($sx-70) $stop $sw 40 "WAITING" $Yellow
$stop += (40 + 12); DownArr $sx ($stop-12) $stop
AddOval ($sx-70) $stop $sw 40 "CALLED" $Orange
$stop += (40 + 12); DownArr $sx ($stop-12) $stop
AddOval ($sx-70) $stop $sw 40 "IN_ROOM" $Green
$stop += (40 + 12); DownArr $sx ($stop-12) $stop
AddOval ($sx-70) $stop $sw 40 "FINISHED" $Red

# ===== BOTTOM: EXIT HANDLER =====
$bx = 400; $btop = 460
AddRnd ($bx-120) $btop 240 36 ("退出医生工作站时" + $nl + "autoEndCurrentConsultation") $Pink
DownArr $bx ($btop+36) ($btop+48)
AddBox ($bx-120) ($btop+48) 240 30 "自动将 IN_ROOM 推进为 FINISHED" $Pink

# ===== LEGEND =====
$ltop = 515
function AddLG($l, $t, $c, $txt) {
    $s = AddBox $l $t 16 12 "" $c; $s.Line.Visible = $false
    $s = AddBox ($l+20) $t 80 12 $txt $White; $s.Line.Visible = $false
    $s.TextFrame.TextRange.Font.Size = 7
}
AddLG 50 $ltop $Blue   "患者操作"
AddLG 160 $ltop $Green  "医生操作"
AddLG 270 $ltop $Teal   "队列/签到"
AddLG 380 $ltop $Orange "看诊/处分"
AddLG 490 $ltop $Red    "状态终结"
AddLG 600 $ltop $Purple "状态机"

# ===== SAVE =====
$doc.SaveAs([ref]$OutputPath)
$doc.Close()
$word.Quit()

[System.Runtime.Interopservices.Marshal]::ReleaseComObject($doc) | Out-Null
[System.Runtime.Interopservices.Marshal]::ReleaseComObject($word) | Out-Null
[System.GC]::Collect()
[System.GC]::WaitForPendingFinalizers()

Write-Output "OK"
