#define _CRT_SECURE_NO_WARNINGS
#include"BufferClear.h"

#include<stdio.h>
void clearBuffer() {
   //printf(">>> 清空输入缓冲区...\n");
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

