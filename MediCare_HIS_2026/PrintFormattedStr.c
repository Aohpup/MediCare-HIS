#define _CRT_SECURE_NO_WARNINGS
#include"PrintFormattedStr.h"

// 终端打印标准化字符串工具 对齐输出

void printFormattedStr(const char* str, int targetWidth) {
    int byteLen = strlen(str);
    int displayWidth = 0;
    int i = 0;
    while (i < byteLen) {
        unsigned char c = (unsigned char)str[i];
        if (c > 127) {
            // 非 ASCII 字符（中文 2宽度）
            displayWidth += 2;

            /* UTF - 8 编码规则：
                 3 字节字符（中文）：以 1110xxxx 开头 →(c & 0xF0) == 0xE0
                 2 字节字符：以 110xxxxx 开头 →(c & 0xE0) == 0xC0
                 4 字节字符（Emoji）：以 11110xxx 开头 →(c & 0xF8) == 0xF0*/

                 // 智能探测编码格式并跳过相应的字节数
            if ((c & 0xF0) == 0xE0) { // c & 1111 0000 == 1110 0000 c为1110 xxxx
                i += 3; // UTF-8 中文 3字节
            }
            else if ((c & 0xE0) == 0xC0) { //c & 1110 0000 == 1100 0000 c为110x xxxx
                i += 2; displayWidth -= 1; // UTF-8 拉丁文 2字节但是宽度为1
            }
            else if ((c & 0xF8) == 0xF0) { //c & 1111 1000 == 1111 0000 c为 1111 0xxx
                i += 4; // UTF-8 Emoji和生僻字 4字节
            }
            else {
                i += 2; // GBK 汉字2字节
            }
        }
        else {
            // ASCII 字符 英文字母/数字
            displayWidth += 1;  //视觉宽度为1
            i += 1;             //占1字节
        }
    }

    // 1. 打印真实的字符串内容
    printf("%s", str);

    // 2. 根据目标宽度与实际视觉宽度的差值，手动补齐空格
    int spaceWidth = targetWidth - displayWidth;
    for (int j = 0; j < spaceWidth; j++) {
        printf(" ");
    }
}