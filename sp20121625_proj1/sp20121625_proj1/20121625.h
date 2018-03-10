#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdio_ext.h>
#include <sys/stat.h>

#define MAX_INST_LEN 100
#define MAX_MEM 1048576

unsigned char v_mem[MAX_MEM] = { 0 };

typedef struct _HIS_NODE {
	char inst[100];
	struct _HIS_NODE* next;
}HIS_NODE;

typedef struct _INST_NODE {
	char opcode[3];
	char mnemonic[10];
	char inst_type[4];
	struct _INST_NODE *next;
}INST_NODE;

HIS_NODE *his_head = NULL, *his_tail = NULL;
INST_NODE **op_head, **op_tail;

#pragma once
