#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdio_ext.h>
#include <sys/stat.h>

#define MAX_INST_LEN 100
#define MAX_MEM 1048576
#define CONST 5
#define EXTENDED 4
#define WORD 3
#define DOUBLE 2
#define BYTE 1
#define DIRECTIVE 0
#define MAX_TRECORD 30

unsigned char v_mem[MAX_MEM];

typedef struct _INST_NODE {
	char opcode[3];
	char mnemonic[10];
	char inst_type[4];
	struct _INST_NODE *next;
}INST_NODE;

typedef struct _HIS_NODE {
	char inst[100];
	struct _HIS_NODE* next;
}HIS_NODE;

typedef struct _SYMB_NODE {
	char symbol[10];
	int adr;
	struct _SYMB_NODE *next;
}SYMB_NODE;

typedef struct _ADR_NODE {
	int adr;
	struct _ADR_NODE *next;
}ADR_NODE;

typedef struct _M_NODE {
	int adr;
	int obj;
	int format;
	struct _M_NODE *next;
}M_NODE;


INST_NODE **op_head, **op_tail;
HIS_NODE *his_head, *his_tail ;
SYMB_NODE **symb_head, **symb_tail;
ADR_NODE *adr_head, *adr_tail;
M_NODE *m_head,*m_tail;  

int hexToDec(char* hex, char cst);

/* creating Date Structure */
INST_NODE* createInstNode(char* opcode, char* mnemonic, char* inst_type);
void createOpTable(INST_NODE* newNode, int idx);
HIS_NODE* createHisNode(char* inst);
void createHistory(HIS_NODE* newNode);
SYMB_NODE* createSymbNode(char* symbol, int adr);
void createSymbTable(SYMB_NODE* newNode, int idx);
ADR_NODE* createAdrNode(int adr);
void createAdrList(ADR_NODE* newNode);
M_NODE* createModNode(int adr,int obj,int format);
void createModList(M_NODE* newNode);




/* shell function */
INST_NODE*opcodeMnemonic(char* inst_tok, char caller);
void opcodelist();
int type(char* filename);
void dir();
void dump(int tok_cnt, char* str_adr, char* end_adr);
void fill(char *str_adr, char* end_adr, char* val);
void quit();

/* assembler function */
int strToDec(char* str);
SYMB_NODE* isValidSymb(char* label);
int rearrangeAsm(char* asm_line, char** label, char** mnemonic, char** symbol, char** para);
void symbol();
int operationRules(int format, char* mnemonic, char* operand, char* para, int NI, int *TA, int line_num, int* MEM_REF);
void printObj(int EXT, int OBJ, char* operand, char*para, int temp_int, FILE* fp);
int assemble(FILE* fp_r, FILE* fp_lst, FILE* fp_obj);
void freeSymbTab();
#pragma once
