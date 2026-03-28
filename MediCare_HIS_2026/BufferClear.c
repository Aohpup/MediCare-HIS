//警告：已被废弃的函数，已使用InputUtils.h中的标准输入函数替代
#define _CRT_SECURE_NO_WARNINGS
#include"BufferClear.h"

#include<stdio.h>
void clearBuffer() {
   //printf(">>> 清空输入缓冲区...\n");
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

