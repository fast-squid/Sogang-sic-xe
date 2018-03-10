#include "20121625.h"

INST_NODE* createInstNode(char* opcode, char* mnemonic, char* inst_type) {
	INST_NODE *newNode = (INST_NODE*)malloc(sizeof(INST_NODE));
	strcpy(newNode->opcode, opcode);
	strcpy(newNode->mnemonic, mnemonic);
	strcpy(newNode->inst_type, inst_type);
	newNode->next = NULL;
	return newNode;
}

void createOpTable(INST_NODE* newNode, int idx) {
	if (!op_head[idx]) { op_head[idx] = newNode; op_tail[idx] = newNode; } // if hashtable's row is empty
	else {
		op_tail[idx]->next = newNode;
		op_tail[idx] = newNode;
	}
}

void createMnTable(INST_NODE* newNode, int idx) {
	if (!mn_head[idx]) { mn_head[idx] = newNode; mn_tail[idx] = newNode; } // if hashtable's row is empty
	else {
		mn_tail[idx]->next = newNode;
		mn_tail[idx] = newNode;
	}
}


HIS_NODE* createHisNode(char*inst) {
	HIS_NODE *newNode = (HIS_NODE*)malloc(sizeof(HIS_NODE));
	strcpy(newNode->inst, inst);
	newNode->next = NULL;
	return newNode;
}

void createHistory(HIS_NODE* newNode) {
	if (!his_head) { his_head = newNode; his_tail = newNode; }
	else {
		his_tail->next = newNode;
		his_tail = newNode;
	}
}

SYMB_NODE* createSymbNode(char* symbol, int adr) {
	SYMB_NODE *newNode = (SYMB_NODE*)malloc(sizeof(SYMB_NODE));
	strcpy(newNode->symbol, symbol);
	newNode->adr = adr;
	newNode->next = NULL;
	return newNode;
}

void createSymbTable(SYMB_NODE* newNode, int idx) {
	if (!symb_head[idx]) { symb_head[idx] = newNode; symb_tail[idx] = newNode; } // if hashtable's row is empty
	else {
		symb_tail[idx]->next = newNode;
		symb_tail[idx] = newNode;
	}
}
ADR_NODE* createAdrNode(int adr) {
	ADR_NODE *newNode = (ADR_NODE*)malloc(sizeof(ADR_NODE));
	newNode->adr = adr;
	newNode->next = NULL;
	return newNode;
}

void createAdrList(ADR_NODE* newNode) {
	if (!adr_head) { adr_head = newNode; adr_tail = newNode; } // if hashtable's row is empty
	else {
		adr_tail->next = newNode;
		adr_tail = newNode;
	}
}

M_NODE* createModNode(int adr,int obj,int format){
	M_NODE* newNode = (M_NODE*)malloc(sizeof(M_NODE));
	newNode->adr = adr;
	newNode->obj=obj;
	newNode->format = format;
	newNode->next= NULL;
	return newNode;
}

void createModList(M_NODE* newNode){
	if (!m_head) { m_head = newNode; m_tail = newNode; } // if hashtable's row is empty
	else {
		m_tail->next = newNode;
		m_tail = newNode;
	}
}

ESTAB_NODE* createEstabNode(char* objLine,char REC) {
	char* symb, *ctrlsect;
	int adr, len;
	ESTAB_NODE* newNode = (ESTAB_NODE*)malloc(sizeof(ESTAB_NODE));

	if (REC == 'H') {
		ctrlsect=symb=readFrag(6, &objLine);
		adr = CSADDR;
		objLine += 12;
		CSLTH = len = hexToDec(readFrag(6, &objLine),'a');
	}
	if (REC == 'D') {
		ctrlsect = NULL;
		symb = readFrag(6, &objLine);
		objLine += 6;
		adr = hexToDec(readFrag(6, &objLine),'a')+CSADDR;
		len = -1;
	}
	newNode->adr = adr;
	strcpy(newNode->symb, symb);
	if(REC=='H') strcpy(newNode->ctrlsect, ctrlsect);
	else strcpy(newNode->ctrlsect ,"\0");
	newNode->len = len;
	newNode->next=NULL;
	return newNode;
}

void createEstab(ESTAB_NODE* newNode,int idx) {
	
	if (!est_head[idx]) { est_head[idx] = newNode; est_tail[idx] = newNode; } // if hashtable's row is empty
	else {
		est_tail[idx]->next = newNode;
		est_tail[idx] = newNode;
	}
}

BP_NODE* createBpNode(int adr) {
	BP_NODE* newNode = (BP_NODE*)malloc(sizeof(BP_NODE));
	newNode->adr = adr;
	newNode->next = NULL;
	return newNode;
}

