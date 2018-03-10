#include "20121625.h"

int isValisLine(char* objLine,char REC) {
	int lineLen=strlen(objLine);
	char flag=0;

	switch (REC) {
	case 'H':	if (lineLen != 18) return 0;								//H RECORD 
				flag = 1; break;											//LENGTH is 18
	case 'D':	if ((lineLen) % 12!=0 || lineLen>72) return 0;				//D RECORD
				flag = 1; break;											//MAXLEN is 72
	case 'R':	if (lineLen>72) return 0;									//R RECORD
				while (lineLen>0) {											//MAXLEN is72
					if (hexToDec(readFrag(2, &objLine), 's') < 0) return 0;
					objLine += 8;
					lineLen -= 8;
				}
				flag = 1; break;
	case 'T':	if (lineLen < 8) return 0;									//T RECORD
				objLine += 6;												//
				if (hexToDec(readFrag(2, &objLine), 's') < 0) return 0;		//LENGTH
				flag = 1; break;			
	case 'M':	flag = 1; break;
	case 'E':	if (!(lineLen == 0 || lineLen==6)) return 0;				//E RECORD
				flag = 1; break;
	case '.':	flag = 1; break;
	}
	if (!flag)return 0;
	return 1;
}

char* readFrag(int fSize, char** f) {										
	char* frag=(char*)malloc(sizeof(char)*(fSize+1));						//READ FRAGMENT
	int i;																	//OF STRING
																			//AND RETURN
	for (i = 0; i < fSize; i++)
		frag[i] = (*f)[i];
	frag[i] = '\0';
	strtok(frag, " \n\t");
	return frag;
}

int isDuplicate(char* str,int idx,char REC) {
	ESTAB_NODE *curr;													
	for (curr = est_head[idx]; curr; curr = curr->next) {					//CHECK IF
		if (REC == 'H') {													//THE CTRL SECTION NAME
			if (!strcmp(curr->ctrlsect, str)) return 1;						//OR SYMBOL NAME
		}																	//IS DUPLICATED
		if (REC == 'M') {
			if (!strcmp(curr->symb, str)) return 1;
		}
	}
	return  0;
}

int relocation(char *REFLINE, int file_num,char *line) {
	ESTAB_NODE *est_curr;
	int i,j,sflag=0,pow=1;
	int addr, sign = 1, REFLEN = strlen(REFLINE), halfB, temp = 0, vtemp = 0;
	char *reloc,*symb;

	if ((addr = hexToDec(readFrag(6, &line), 's')) == -1) return 0;			// Address 
	addr+=CSADDR;															// needed to be modified
	line += 6;
	if((halfB = hexToDec(readFrag(2, &line), 's'))==-1) return 0;			// halfbytes
	line += 2;																// to be modified
	if (line[0] == '-') sign = -1;											// positive
	line++;																	// or negative
	reloc = readFrag(6, &line);												// get reference number 
																			// or reference symbol
	while (REFLEN>0) {
		if (!strcmp("01", reloc)) {
			sflag = 1;
			for (i = 0; i < file_num; i++) {
				if (est_head[i]->adr == CSADDR) symb = est_head[i]->symb;
			}
		}
		else if (!strcmp(readFrag(2, &REFLINE), reloc)) {
			sflag = 1;
			REFLINE += 2;
			symb=readFrag(6, &REFLINE);
			break;
		}
		REFLINE += 8;
		REFLEN -= 8;
	}

	if (!sflag) return 0;
	sflag = 0;
	for (i = 0; i < file_num; i++) {
		for (est_curr = est_head[i]; est_curr; est_curr = est_curr->next) {
			if (!strcmp(est_curr->symb, symb)) {
				temp = est_curr->adr;
				for (j = 2; j >= 0; j--) {
					vtemp += v_mem[addr + j] * pow;							
					pow *= 256;
				}
				vtemp += sign*temp;
				for (j = 2; j >= 0; j--) { 
					v_mem[addr + j] = vtemp%256;							//MODIFY LOADED MEMORY
					vtemp /= 256;
				}
				sflag = 1;
			}
		}
	}
	if (!sflag) return 0;
	return 1;
}

int loader(int file_num,char** obj_file){
	FILE* fp[3];
	int i, j;
	int T_ADDR, T_LEN,total_len=0,REL_ADDR;
	char buff[100],REC_TYPE;
	char *linep,line_len,*temp,REFLINE[72];
	ESTAB_NODE *temp_est;
	int hFlag; 

	CSNUM = file_num;

	for (i = 0; i < 10; i++) REG[i] = 0; 									// init register to 0
	/* pass 1 */	
	for (i = 0; i < file_num; i++) {
		if (!(fp[i] = fopen(obj_file[i], "r"))) {
			printf("no program to load on memory!\n");
			return 0;
		}
		hFlag=0;
		while (fgets(buff, 100, fp[i])) {
			linep = buff;
			strtok(buff, "\n\t");
			REC_TYPE = buff[0];
			linep++;
			if (!(isValisLine(linep,REC_TYPE)))
				return 0;
			if (REC_TYPE == 'H') {
				if(hFlag){printf("more than 1 H record\n"); return 0;}
				if(!i) CSADDR = PROGADDR;
				line_len = strlen(linep);
				temp = linep+6;
				REL_ADDR = hexToDec(readFrag(6, &temp), 'a');
				temp_est = createEstabNode(linep, REC_TYPE);
				if (isDuplicate(temp_est->ctrlsect, i, REC_TYPE)) {
					printf("duplicate external symbol\n"); return 0;
				}
				createEstab(temp_est,i);
				hFlag=1;
			}
			else if (REC_TYPE == 'D') {
				line_len = strlen(linep);
				while (line_len) {
					temp_est = createEstabNode(linep, REC_TYPE);
					if (isDuplicate(temp_est->symb, i, REC_TYPE)) {
						printf("duplicate external symbol\n"); return 0;
					}
					createEstab(temp_est, i);
					linep += 12;
					line_len /= 12;
				}
			}
		}
		CSADDR += CSLTH;													// SECTION
	}
	/* pass 2 */
	CSADDR = PROGADDR;
	for (i = 0; i < file_num; i++) {
		fseek(fp[i], 0, 0);													//file pointer reset
		while (fgets(buff, 100, fp[i])) {
			linep = buff;													
			strtok(buff, "\n\t");											
			REC_TYPE = buff[0];												
			linep++;														//pointer forwarding
			if (!(isValisLine(linep, REC_TYPE)))							//check if line is valid
				return 0;
			if (REC_TYPE == 'H' || REC_TYPE == 'D') continue;				
			else if (REC_TYPE == 'R') strcpy(REFLINE ,linep);				//copy R record
			else if (REC_TYPE == 'T') {										
				T_ADDR = hexToDec(readFrag(6, &linep), 's') - REL_ADDR  + CSADDR; linep += 6;
				T_LEN = hexToDec(readFrag(2, &linep), 's'); linep += 2;
				for (j = 0; j < T_LEN; j++) {								// load objcode to memory;
					v_mem[T_ADDR + j] = hexToDec(readFrag(2, &linep),'s');
					linep += 2;
				}
			}
			else if (REC_TYPE == 'M') {
				if (!relocation(REFLINE, file_num, linep)) {				//modification memory
					printf("reference error\n");
					return 0;
				}
			}
			else if (i == file_num - 1 && REC_TYPE == 'E') {				
				CSADDR = PROGADDR;
			}
		}
		CSADDR += est_head[i]->len;
		fclose(fp[i]);
	}
	/* 	PRINT RESULT OF LOAD */
	printf("control\t\tsymbol\t\taddress\t\tlength\n");
	printf("section\t\tname\n");
	printf("-------------------------------------------------------\n");
	for (i = 0; i < file_num; i++) {
		for (temp_est = est_head[i]; temp_est; temp_est = temp_est->next) {
			if (temp_est == est_head[i]) {
				printf("%s\t\t", temp_est->ctrlsect);
				printf("%04X\t\t\t\t", temp_est->adr);
				printf("%04X\n", temp_est->len);
			}
			else {
				printf("\t\t");
				printf("%s\t\t", temp_est->symb);
				printf("%04X\n", temp_est->adr);
			}
		}
		total_len += est_head[i]->len;
		PROGLEN = total_len;
		CSADDR = PROGADDR;
	
	}
	printf("-------------------------------------------------------\n");
	printf("\t\t\t\ttotal length\t%04X\n", PROGLEN);
	return 1;
}

int operate(int fSize,char*frag,char *op,int* XBPE,int NI) {
	int i,temp,shift;
	char rt1[2],rt2[2];
	int DISP,r1,r2;
	int TA,TA_VAL=0;
	/*---------------------------------------------------------*/
	char*opSet[58] = { "18","58","90","40","B4","28","88","A0","24","64","9C",
		"C4","C0","F4","3C","30","34","38","48","00","68","50","70","08","6C",
		"74","04","D0","20","60","98","C8","44","D8","AC","4C","A4","F0","EC",
		"0C","78","54","80","D4","14","7C","E8","84","10","1C","5C","94","B0",
		"E0","F8","2C","B8","DC" };
	/*---------------------------------------------------------*/
	
	if (fSize == 1) {
		rt1[0] = frag[0]; rt1[1] = '\0';
		rt2[0] = frag[1]; rt2[1] = '\0';
		r1 = hexToDec(rt1, 's') % 16;
		r2 = hexToDec(rt2, 's') % 16;
	}
	else if (fSize >= 2) {
		DISP = hexToDec(frag, 's');
		if (frag[0] > '7') {
			DISP = (4096 - DISP)*-1;
		}
		TA = DISP;
		if (XBPE[3])
			TA += REG[1];						// indexing mode
		if (XBPE[2])
			TA += REG[3];						//  BASE relative
		else if (XBPE[1])
			TA += REG[8];						//  PC relative
		if (NI == 1) {								// immediate
			TA_VAL = TA;
		}
		else if (NI == 2) {						// indirect
			TA = v_mem[TA] * 0x10000 + v_mem[TA + 1] * 0x100 + v_mem[TA + 2];
			TA_VAL = v_mem[TA] * 0x10000 + v_mem[TA+1]* 0x100 +v_mem[TA+2];
		}
		else if (NI == 3) {						// simple			
			TA_VAL = v_mem[TA]* 0x10000 + v_mem[TA+1] * 0x100 + v_mem[TA+2];
		}
	}
	TA_VAL %= 0x1000000;
	for (i = 0; i < 58; i++) {
		if (!strcmp(op, opSet[i])) break;
	}
	switch (i) {
	case 0:	REG[0] += TA_VAL; ; break;									//ADD	
	case 1: break;														//ADDF
	case 2: REG[r2] += REG[r1]; break;									//ADDR
	case 3: REG[0] &= TA_VAL; break;									//AND
	case 4: REG[r1] = 0; break;											//CLEAR
	case 5: if (REG[0] == TA_VAL)  REG[9] = '=';						//COMP
			else if (REG[0] < TA_VAL) REG[9] = '<';
			else REG[9] = '>'; break;
	case 6:	break;														//COMPF
	case 7: if (REG[r1]==REG[r2]) REG[9] = '=';							//COMPR
			else if (REG[r1] < REG[r2]) REG[9] = '<';
			else REG[9] = '>'; break;
	case 8: REG[0] /= TA_VAL; break;									//DIV
	case 9: break;														//DIVF
	case 10:REG[r2] /= REG[r1]; break;									//DIVR
	case 11:break;														//FIX
	case 12:break;														//FLOAT
	case 13:break;														//HIO
	case 14:REG[8] = TA; break;					
	case 15:if (REG[9] == '=') REG[8] = TA; break;						//JEQ
	case 16:if (REG[9] == '<') REG[8] = TA; break;						//JGT
	case 17:if (REG[9] == '>') REG[8] = TA; break;						//JLT
	case 18:REG[2] = REG[8]; REG[8] = TA; break;						//JSUB
	case 19:REG[0] = TA_VAL; break;										//LDA
	case 20:REG[3] = TA_VAL; break;										//LDB
	case 21:REG[0] = TA_VAL % 0X100; break;								//LDCH
	case 22:break;														//LDF
	case 23:REG[2] = TA_VAL; break;										//LDL
	case 24:REG[4] = TA_VAL; break;										//LDS
	case 25:REG[5] = TA_VAL; break;										//LDT
	case 26:REG[1] = TA_VAL; break;										//LDX
	case 27:break;														//LPS
	case 28:REG[0] *= TA_VAL;  break;									//MUL
	case 29:break;														//MULF
	case 30:REG[r2] *= REG[r1];  break;									//MULR
	case 31:break;														//NORM
	case 32:REG[0] |= TA_VAL; break;									//OR
	case 33: break;														//RD
	case 34:REG[r2] = REG[r1]; break;									//RMO
	case 35:REG[8] = REG[2]; break;										//RSUB
	case 36:shift=1;													//SHIFTL
			for (i = 0; i <REG_SIZE - (r2+1); i++) {
				shift *= 2;
			}
			temp = (REG[r1] / (0x1000000/shift)) % shift;
			REG[r1] *= shift;
			REG[r1] += temp;
			break;
	case 37:break;														//SIO
	case 38:break;														//SSK
	case 39:temp = REG[0];												//STA
			v_mem[TA + 2] = temp % 0x100; temp /= 0x100;
			v_mem[TA + 1] = temp % 0x100; temp /= 0x100;
			v_mem[TA + 0] = temp % 0x100; break;
	case 40:temp = REG[3];												//STB
			v_mem[TA + 2] = temp % 0x100; temp /= 0x100;
			v_mem[TA + 1] = temp % 0x100; temp /= 0x100;
			v_mem[TA + 0] = temp % 0x100; break;
	case 41:v_mem[TA]= REG[0] % 0x100;									//STCH
	case 42:break;														//STF
	case 43:break;														//STI
	case 44:temp = REG[2];												//STL
			v_mem[TA + 2] = temp % 0x100; temp /= 0x100;
			v_mem[TA + 1] = temp % 0x100; temp /= 0x100;
			v_mem[TA + 0] = temp % 0x100; break;
	case 45:temp = REG[4];												//STS
			v_mem[TA + 2] = temp % 0x100; temp /= 0x100;
			v_mem[TA + 1] = temp % 0x100; temp /= 0x100;
			v_mem[TA + 0] = temp % 0x100; break;
	case 46:temp = REG[9];												//STSW
			v_mem[TA + 2] = temp % 0x100; temp /= 0x100;
			v_mem[TA + 1] = temp % 0x100; temp /= 0x100;
			v_mem[TA + 0] = temp % 0x100; break;
	case 47:temp = REG[5];												//STT
			v_mem[TA + 2] = temp % 0x100; temp /= 0x100;
			v_mem[TA + 1] = temp % 0x100; temp /= 0x100;
			v_mem[TA + 0] = temp % 0x100; break;
	case 48:temp = REG[1];												//STX
			v_mem[TA + 2] = temp % 0x100; temp /= 0x100;
			v_mem[TA + 1] = temp % 0x100; temp /= 0x100;
			v_mem[TA + 0] = temp % 0x100; break;
	case 49:REG[0] -= TA_VAL;											//SUB
	case 50:break;														//SUBF
	case 51:REG[r2] -= REG[r1]; break;									//SUBR
	case 52:break;														//SVC
	case 53:REG[9] = '<'; break;										//TD
	case 54:break;														//TIO
	case 55:REG[1]++;													//TIX
			if (REG[1] == TA_VAL) REG[9] = '=';
			else if(REG[1]<TA_VAL) REG[9] = '<';
			else REG[9] = '>';
			break;
	case 56:															//TIXR
		REG[1]++;
			if (REG[1] == REG[r1])
				REG[9] = '=';
			else if (REG[1]<REG[r1]) REG[9] = '<';
			else REG[9] = '>';
	case 57:break;														//WD
	}
	if(i>57) return 0;
	return 1;
	
}

char* hexToStr(int fSize,int offset,int NI) {
	char *str = (char*)calloc(fSize * 2+1,sizeof(char));
	int i, pow = 1, res, val = 0;

	for (i = fSize-1; i >= 0; i--) {
		val += v_mem[REG[8] + i+offset] * pow;
		pow *= 0x100;
	}
	if(NI)																					
		val -= NI;
	if (fSize == 2) val %= 0x1000;
	else if (fSize == 3) val %=0x100000;
	
	str[strlen(str)] = '\0';

	switch (fSize) {
	case 1:
		res = val % 16;
		if (10 <= res&&res <= 15) str[1]= res - 10 + 'A';
		else if (0 <= res&&res <= 9) str[1] = res + '0';
		val /= 16;
		res = val % 16;
		if (10 <= res&&res <= 15) str[0] = res - 10 + 'A';
		else if (0 <= res&&res <= 9) str[0] = res + '0'; break;
	case 2:
		res = val % 16;
		if (10 <= res&&res <= 15) str[3] = res - 10 + 'A';
		else if (0 <= res&&res <= 9) str[3] = res + '0';
		val /= 16;
		res = val % 16;
		if (10 <= res&&res <= 15) str[2] = res - 10 + 'A';
		else if (0 <= res&&res <= 9) str[2] = res + '0';
		val /= 16;
		
		res = val % 16;
		if (10 <= res&&res <= 15) str[1] = res - 10 + 'A';
		else if (0 <= res&&res <= 9) str[1] = res + '0';
		val /= 16;
		res = val % 16;
		if (10 <= res&&res <= 15) str[0] = res - 10 + 'A';
		else if (0 <= res&&res <= 9) str[0] = res + '0'; break;
	case 3:
		res = val % 16;
		if (10 <= res&&res <= 15) str[5] = res - 10 + 'A';
		else if (0 <= res&&res <= 9) str[5] = res + '0';
		val /= 16;
		res = val % 16;
		if (10 <= res&&res <= 15) str[4] = res - 10 + 'A';
		else if (0 <= res&&res <= 9) str[4] = res + '0';
		val /= 16;

		res = val % 16;
		if (10 <= res&&res <= 15) str[3] = res - 10 + 'A';
		else if (0 <= res&&res <= 9) str[3] = res + '0';
		val /= 16;
		res = val % 16;
		if (10 <= res&&res <= 15) str[2] = res - 10 + 'A';
		else if (0 <= res&&res <= 9) str[2] = res + '0';
		val /= 16;

		res = val % 16;
		if (10 <= res&&res <= 15) str[1] = res - 10 + 'A';
		else if (0 <= res&&res <= 9) str[1] = res + '0';
		val /= 16;
		res = val % 16;
		if (10 <= res&&res <= 15) str[0] = res - 10 + 'A';
		else if (0 <= res&&res <= 9) str[0] = res + '0'; break;
	}

	return str;
}

int run(){
	int endBp;
	char op[3], ta[7];
	INST_NODE * mn_curr;
	int i,temp,NI,XBPE[4],fSize,sum;
	static int first_run = 0;
	
	if (first_run == 0) {											// on the first run
		REG[8] = PROGADDR;											// PC to PROGADDR
		bp_end = bp_head;
		if(bp_end) endBp = bp_head->adr;							// run to break point
		first_run = 1;												// set flag
		REG[2] = PROGADDR + PROGLEN;
	}
	else {															// not first run
		if (!bp_end) {	
			bp_end = bp_head;
			if (bp_end) {
				if (bp_end->adr < REG[8]) {
					
					endBp = PROGADDR + PROGLEN;
				}
				else
					endBp = bp_end->adr;
				
			}
		}
		else endBp = bp_end->adr;									
	}

	if (!bp_head) endBp = PROGADDR + PROGLEN;						// run until end of the program
	if (REG[8]==PROGADDR) REG[2] = PROGADDR+PROGLEN;
	while (REG[8] < endBp) {
		XBPE[3] = XBPE[2] = XBPE[1] = XBPE[0] = 0;					// init XBPE to 0
		
		temp = v_mem[REG[8]];									
		if (!temp) {
			REG[8]++;
			continue;
		}
		NI = temp % 4;
		temp -= NI;
		strcpy(op,hexToStr(1, 0 ,NI));								// get opcode string

		/* DECREPTION *//////////////////////////////////////
		sum = 0;
		for (i = 0; i<strlen(op); i++)
			sum += op[i] + (op[i] - 'A')*(i);									// calculating index of hashtable
		for (mn_curr = mn_head[sum % 20]; mn_curr; mn_curr=mn_curr->next) {		// forwarding linked list
			if (!strcmp(mn_curr->opcode,op)) {									// if found
				if (!strcmp(mn_curr->inst_type, "1")) fSize = 0;				// format 1
				else if (!strcmp(mn_curr->inst_type, "2")) fSize = 1;			// format 2
				else if (!strcmp(mn_curr->inst_type, "3/4")) {					// format 3/4
					temp = (v_mem[REG[8]+1]/16 )%16;							// *
					for (i = 0; i < 4; i++) {									//	get 
						XBPE[i] = temp % 2;										// XBPE 
						temp /= 2;												// bit
					}															// *
					if (XBPE[0]) fSize = 3;										// format 3
					else fSize = 2;												// format 4
				}
				strcpy(ta,hexToStr(fSize, 1,0));								// get displacement string
				REG[8] += fSize + 1;											// PC register increment
				
				if(!operate(fSize,ta,mn_curr->opcode,XBPE,NI)) return 0;		// call SIC/XE operator
				break;
			} 
		}
	}
	/* PRINT REGISTER VALUE *////////////////////////////
	printf("A:%012X\t",REG[0]);	printf("X:%012X\n", REG[1]);
	printf("L:%012X\t", REG[2]);	printf("PC:%012X\n", REG[8]);
	printf("B:%012X\t", REG[3]);	printf("S:%012X\n", REG[4]);
	printf("T:%012X\n", REG[5]);	
	/* SETTING BREAK POINT */
	if (REG[8]<PROGADDR+PROGLEN) {												// program stopped on breakpoint
		printf("Stop at check point[%04X]\n", bp_end->adr);
		if (bp_end) 	bp_end = bp_end->next;
	}
	else {																		// program ended
		for (i = 0; i < 10; i++) REG[i] = 0;
		bp_end = bp_head;														// break point back to first
		printf("End program\n");
	}
	return 1;
}

int breakPoint(char *para) {
	BP_NODE* bp_curr,*bp_prev,*bp_temp;
	int bp_adr,flag=0;
	
	if (!est_head[0]) {
		printf("no program loaded\n");
		return 0;
	}

	if (!para) {
		printf("breakpoint\n");
		printf("----------\n");
		for (bp_curr = bp_head; bp_curr; bp_curr = bp_curr->next) {
			printf("%04X\n", bp_curr->adr);
		}
	}
	else if (!strcmp(para, "clear")) {
		while (bp_head) {
			bp_curr = bp_head;
			bp_head = bp_head->next;
			free(bp_curr);
		}
		printf("[ok] clear all breakpoints\n");
	}
	else{
		bp_adr = hexToDec(para, 'a');
		if (!(PROGADDR < bp_adr&&bp_adr < PROGADDR+PROGLEN)){ printf("unavailable breakpoint\n"); return 0;}
		if (!bp_head) {bp_tail = bp_head = createBpNode(bp_adr);}
		else {
			for (bp_curr = bp_head; bp_curr; bp_curr = bp_curr->next) {
				if (bp_curr->adr == bp_adr) { printf("already existing breakpoint\n"); return 0; }
				else if (bp_curr->adr > bp_adr) {
					bp_temp = createBpNode(bp_adr);
					if (bp_curr == bp_head) {
						bp_temp->next = bp_head;
						bp_head = bp_temp;
					}
					else {
						bp_temp->next = bp_curr;
						bp_prev->next = bp_temp;
					}
					flag = 1;
					break;
				}
				bp_prev = bp_curr;
			}
			if (!flag) {
				bp_tail->next = createBpNode(bp_adr);
				bp_tail = bp_tail->next;
			}	
		}
		printf("[ok] create breakpoint %04X\n", bp_adr);
	}
	return 1;
}

void freeESTAB() {
	int i;
	ESTAB_NODE *est_curr;
	for (i = 0; i<3; i++) {
		while (est_head[i]) {
			est_curr = est_head[i];
			est_head[i] = est_head[i]->next;
			free(est_curr);
		}
	}
}
