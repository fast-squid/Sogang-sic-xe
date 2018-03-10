#include "20121625.h"

int strToDec(char* str) {
	int i,dec=0,pow=1;
	
	if(!str) return -1;
	for (i = strlen(str) - 1; i >= 0; i--) {
		if ('0' <= str[i] && str[i] <= '9') { dec += (str[i] - '0')*pow; }
		else return -1;
		pow *= 10;
	}
	return dec;
}

SYMB_NODE* isValidSymb(char* label) {
	SYMB_NODE *curr;

	if(!label) return NULL; 	// NULL string
	if (!('A' <= label[0] && label[0] <= 'Z')) return NULL;
	for (curr = symb_head[label[0]-'A']; curr; curr = curr->next) {
		if (!strcmp(curr->symbol, label)) {
			return curr;
		}
	}
	return NULL; 				// symbol not found
}

int rearrangeAsm(char* asm_line, char** label, char** mnemonic, char** symbol, char** para) {
	int len = strlen(asm_line);
	int i, c_cnt = 0, a_cnt = 0, a_pos[2] = { 0 },c_pos;
	char* temp=NULL, *a_p[2],*c_p;
	for (i = 0; i < len; i++) {
		if (asm_line[i] == ',' ){
			if(a_cnt == 1 && a_pos[0] < i && a_pos[1] == 0) continue;	// case of ' .. , ..'
			if(c_cnt>=1) return 0;										// else if several ','
			c_p=&asm_line[i];
			asm_line[i] = ' ';
			c_cnt++;
			c_pos = i;
		}
		else if (asm_line[i] == 39) {									
			if(a_cnt>=2) return 0;				
			a_p[a_cnt]=&asm_line[i];
			asm_line[i] = ' ';
			a_pos[a_cnt] = i;
			a_cnt++;
			if(a_cnt==2) temp=&asm_line[i+1];							// 5th token
		}
	}
	if(c_cnt==1&& a_cnt==2){
		if(!(a_pos[0]<c_pos&&c_pos<a_pos[1])) return 0;
	}
	*label = strtok(asm_line, " \t\r\n");
	*mnemonic = strtok(NULL, " \t\r\n");
	*symbol = strtok(NULL, " \t\r\n");
	if (a_pos[0]) {
		if(a_cnt!=2) return 0;
		asm_line[a_pos[1]] = '\0';
		//if (!(*symbol + strlen(*symbol))[0] == asm_line[a_pos[0]-1]) return 0;
		*para = strtok(NULL, "\r\n");
	}
	else *para = strtok(NULL, " \t\r\n");

	if((temp=strtok(temp," \t\r\n"))) return 0;
	/*rearrangement*//////////////////////////////////////////////////
	if (!(*symbol)) {
		*para = *symbol;	*symbol = *mnemonic;	*mnemonic = *label; *label = NULL;
	}
	else if ((a_cnt||c_cnt) && !(*para)) {
		*para = *symbol;	*symbol = *mnemonic;	*mnemonic = *label; *label = NULL;
	}
	if(*label){
		for(i=0;i<strlen(*label);i++){ 			// constraints for label name
			if(i==0){
				if(!(('A'<=(*label)[i] && (*label)[i]<='Z')
				||('a'<=(*label)[i] && (*label)[i]<='z'))) return 0;
			}
			else if(!(('A'<=(*label)[i] && (*label)[i]<='Z')
			||('a'<=(*label)[i] && (*label)[i]<='z') 
			||('0'<=(*label)[i] && (*label)[i]<='9'))) return 0;
		}
	}
	
	if(c_cnt){
		if(!(*symbol||*para)) return 0;
		if(!(&(*symbol)[strlen(*symbol)-1]<=c_p && c_p<=&(*para)[0])) return 0;												// location of ,
	}
	if(a_cnt){
		if(!(*symbol||*para)) return 0; 
		if(!(&(*symbol)[strlen(*symbol)-1]<=a_p[0] && a_p[0]<=&(*para)[0]&& &(*para)[strlen(*para)-1]<=a_p[1])) return 0; 	// location of '
	}
	return 1;
}

void symbol() {
	SYMB_NODE *curr_i,*arr,temp;
	int i,j,curr,cnt=0;
	for (i = 0; i < 26; i++) { // count for memory allocation
		for (curr_i = symb_head[i]; curr_i; curr_i = curr_i->next) cnt++;
	}
	if(!cnt){
		printf("empty symbol table\n");
	}
	arr = (SYMB_NODE*)malloc(sizeof(SYMB_NODE)*cnt);
	cnt = 0;
	for (i = 0; i < 26; i++) {	// assign values from symbol table to array
		for (curr_i = symb_head[i]; curr_i; curr_i = curr_i->next) {
			arr[cnt].adr=curr_i->adr;
			strcpy(arr[cnt++].symbol, curr_i->symbol);
		}
	}
	for (i = 0; i < cnt; i++) { // selection sort
		curr = i;
		for (j = i + 1; j < cnt; j++) {
			if (strcmp(arr[curr].symbol, arr[j].symbol)<0) {
				curr = j;
			}
			temp.adr = arr[curr].adr;	strcpy(temp.symbol, arr[curr].symbol);
			arr[curr].adr=arr[i].adr;	strcpy(arr[curr].symbol,arr[i].symbol);
			arr[i].adr = temp.adr;		strcpy(arr[i].symbol, temp.symbol);
		}
		printf("\t%s\t%04X\n", arr[i].symbol, arr[i].adr);
	}
	free(arr);
}

int operationRules(int format, char* mnemonic, char* operand, char* para, int NI,int *TA, int line_num,int* MEM_REF) {
	char *FORMAT2[11] = { "ADDR","COMPR", "DIVR","MULR", "RMO","SUBR", // 0~5 : r1 r2 
		"SHIFTL","SHIFTR","TIXR","CLEAR","SVC"}; // 6~7 : r1  n / 8 ~ 9: r1 / 9 : n 
	char *REG_NUM[10] = { "A","X","L","B","S","T","F","overhead","PC","SW" };
	SYMB_NODE* temp;
	int i,j,REG[2];
	
	switch (format) {
	case BYTE: if (operand) { printf("unnecessary operand for format 1 on line %d\n",line_num); return -1; }
			   break;
	case DOUBLE:for (i = 0; i < 10; i++) {
					if (!strcmp(mnemonic, FORMAT2[i])) break;
				}
				if(!operand) { printf("not enough operands on line %d\n", line_num); return -1; }
				for (j = 0; j < 10; j++) {
					if (!strcmp(operand, REG_NUM[j])) { REG[0] = j; break; }
				}
				if (j == 7 || j > 9) { printf("not available on operand line %d\n", line_num); return -1; }
				if (0 <= i && i <= 5) { 
					if (!para) { printf("not enough operands for on line %d\n", line_num); return -1; }
					for (j = 0; j < 10; j++) {
						if (!strcmp(para, REG_NUM[j])) { REG[1] = j; break; }
					}
					if (j == 7 || j > 9) { printf("not available on line %d\n", line_num); return -1; }
				}
				else if ( 6 == i || i == 7) {
					if (!para) { printf("not enough operands on line %d\n", line_num); return -1; }
					REG[1]=strToDec(para);
					if(!(1<=REG[1]&&REG[1]<=16)) { printf("unavailable value on line %d\n", line_num); return -1; }
					REG[1]--;
				}
				else if (i == 8 || i == 9) {
					if (para) { printf("too much operands on line %d\n", line_num); return -1; }
					REG[1] = 0;
				}
				else if (i == 10) {
					if (para) { printf("too much operands on line %d\n", line_num); return -1; }
					if (!(0<=REG[0]&&REG[0] <= 15)) { printf("unavailable value on line %d\n", line_num); return -1; }
				}
				if(REG[0]>15||REG[1]>15) { printf("unavailable value on line %d\n", line_num); return -1; }
				*TA = REG[0] * 16 + REG[1] * 1;
				break;
	case WORD:
	case EXTENDED: 	if (!strcmp(mnemonic, "RSUB")) {
						if (operand) { printf("unnecessary operand on line %d\n", line_num); return -1; }
						else break;
					}
					else if (!operand) { printf("not enough operand on line %d\n", line_num); return -1; }
				   	else {																												// immediate addressing mode	
						if((NI==1 || NI==2)&&para) { printf("unnecessary operand on line %d\n", line_num); return -1; }
					    if (!(temp=isValidSymb(operand))) {																				// constant operands
							if ((*TA=strToDec(operand)) < 0) { printf("unavailable value on line %d\n", line_num); return -1; }
							if (format == WORD) { 																						// op #c / op @c
								if(!(0<=*TA&& *TA<=4095)) { printf("unavailable value on line %d\n", line_num); return -1; } 			// contant value 0~4095				
							}
						   	if (format == EXTENDED) {																					// +op #m / +op @m
							   	if (!( 4096<= *TA && *TA <= 1048575)) { printf("unavailable value on line %d\n",line_num); return -1; } // contant value 4096~1048575
							}
							*MEM_REF = CONST;
							if(NI==1) *MEM_REF = -1;
					   	}
					   	else {																											// no need for relocation but relative
							*TA = temp->adr;
							if(NI==1) *MEM_REF = 1;
						}
				   	}
				   	break;
	}
	return 1;
}

void printObj(int EXT, int OBJ, char* operand,char*para, int temp_int,FILE* fp) {
	int i;
	if (EXT == EXTENDED) fprintf(fp, "%08X",(unsigned int) OBJ);
	else if (EXT == WORD) {
		if (!operand) fprintf(fp, "%06X", OBJ);
		else fprintf(fp, "%06X", OBJ);
	}
	else if (EXT == DOUBLE) fprintf(fp, "%04X", OBJ);
	else if (EXT == BYTE) fprintf(fp, "%02X", OBJ);
	else if (EXT == CONST) {	// if DIRECTIVE BYTE
		if (operand[0] == 'X') {
			if (temp_int % 2) fprintf(fp, "0");
			for(i=0;i<temp_int;i++){
				if('a'<=para[i]&&para[i]<='z') para[i] +='A' - 'a';
			}
			fprintf(fp, "%s", para);
		}
		else if (operand[0] == 'C') {
			for (i = 0; i< temp_int; i++) {
				fprintf(fp, "%02X", para[i]);
			}
		}
	}
}

int assemble(FILE* fp_r, FILE* fp_lst, FILE* fp_obj) {
	char prog_name[7];
	char buf[200], full[120], *label, *mnemonic, *operand, *para;
	int start_adr, prog_len, loc_ctr = -1, line_ctr = 0, rec_len;
	int i, temp_int;
	char *DIRECTIVES[8] = { "WORD","RESW","RESB","BYTE","BASE","NOBASE","END","START" };
	/////////////////////////////////////////////////////////////////////////////
	int TA, EXT, OP, PC, BASE, NI, XBPE, MEM_REF;
	unsigned int OBJ;
	INST_NODE *temp;
	ADR_NODE *adr_temp;
	SYMB_NODE* symb_temp;
	M_NODE* m_temp;

	if (!fp_r) { printf("file open error\n"); return 0; }

	/* pass 1 *//////////////////////////////////////////////////////
	while (fgets(buf, 200, fp_r)) {
		/* initailize variables *////////////////////////////////////
		para = label = mnemonic = operand = NULL;
	    EXT = 0;
		line_ctr += 5;
		/////////////////////////////////////////////////////////////
		if (buf[0] == '.') continue;  									// continue if comment
		if (!rearrangeAsm(buf, &label, &mnemonic, &operand, &para)) { 	//rearrange instruction in proper form
			printf("file format error on line %d\n", line_ctr); return 0;
		}
		if (!(label || mnemonic || operand || para)) continue;			// continue if empty line
		if (mnemonic[0] == '+') { mnemonic++;  EXT = EXTENDED; } 		// '+'prefix for format4
		
		if (loc_ctr == -1) { 											// start of the program
			if (!strcmp(mnemonic, DIRECTIVES[7])) { 					// prog starts with DIRECTIVES "START"
				strcpy(prog_name, label);
				loc_ctr = PC = start_adr = hexToDec(operand, 'a');
			}
			else {														// prog starts with no DIRECTIVES "START"
				strcpy(prog_name, "");
				loc_ctr = PC = start_adr = 0;
			}
			rec_len = 0;
		}
		
		if (label) {
			if (isValidSymb(label)) { printf("duplicate symbol on line %d\n", line_ctr); return 0; } 				// duplicate symbol error
			if((temp_int = label[0] - 'A')<0){printf("unavailable instruction on line %d\n",line_ctr); return 0;};	// unauthorized index
			createSymbTable(createSymbNode(label, PC), temp_int); 													// create symbol table
		}
		/* asm directives *//////////////////////////////////////////
		for (i = 0; i < 8; i++) {
				if (!strcmp(mnemonic, DIRECTIVES[i]))break;
		}
		if(0<=i&&i<=7){ 											
			switch (i) {
			case 0:PC += 3; break;
			case 1:	if ((temp_int = strToDec(operand)) < 0) { printf("unavailable value on line %d\n", line_ctr); return 0; }
					PC += temp_int * WORD; EXT = CONST; break;
			case 2:	if ((temp_int = strToDec(operand)) < 0) { printf("unavailable value on line %d\n", line_ctr); return 0; }
					PC += temp_int * BYTE; EXT = CONST; break;
			case 3:	if (!strcmp(operand, "X")) {
						temp_int = strlen(para);
						for (i = 0; i < temp_int; i++) {
							if (para[i] != '0') break;
						}
						if (i == temp_int) para += (i - 1);
						else para += (i);
						if (60 < (temp_int = strlen(para))) { printf("unavailable operand on line %d\n", line_ctr); return 0; }
						for (i = 0; i < temp_int; i++) {
							if (!(('A' <= para[i] && para[i] <= 'F') ||
								('a' <= para[i] && para[i] <= 'f') ||
								('0' <= para[i] && para[i] <= '9'))) {
								printf("unavailable operand on line %d\n", line_ctr); return 0;
							}
						}
						PC += temp_int / 2 + temp_int % 2;
					}
					else if (!strcmp(operand, "C")) {
						if ((temp_int = strlen(para))>30) { printf("unavailable value on line %d\n", line_ctr); return 0; }
						PC += temp_int * 1;
					}
					else { printf("unavailable operand on line %d\n", line_ctr); return 0; }
					EXT = BYTE;
					break;
			case 4: 
			case 5:continue;
			}
			if (!strcmp(mnemonic, DIRECTIVES[6])) {
				if((symb_temp=isValidSymb(operand))) {
					if(symb_temp->adr!=start_adr){ 
						printf("END should specify the start of the program... line %d\n", line_ctr); 	// if END operand do not indicate start of prog
						return 0;
					}
				}
				else{
					printf("no operand for END on line %d\n",line_ctr); 									// if NO operand for END
					return 0;
				}
			}
			if (!strcmp(mnemonic, DIRECTIVES[7]) && loc_ctr!=start_adr) {								// if START middle of the program
				printf("unavailable directive on line %d\n", line_ctr);
				return 0;
			}
		}	
		/* mnemonic found in opcode table *//////////////////////////
		else if ((temp = opcodeMnemonic(mnemonic, 'a'))) { 
			if (!strcmp(temp->inst_type, "1")) { PC += BYTE; EXT = BYTE; }
			else if (!strcmp(temp->inst_type, "2")) { PC += DOUBLE; EXT = DOUBLE; }
			else if (!strcmp(temp->inst_type, "3/4")) {
				PC += 3;
				if (EXT == EXTENDED)PC++;
				else EXT = WORD;
			}
		}	
		/* Not a directive nor mnemonic */////////////////////////////
		else{
			printf("unavailable instruction on line %d\n",line_ctr);
			return 0;
		}

		/* save record length *//////////////////////////////////////
		if (rec_len + (PC - loc_ctr) > MAX_TRECORD) {
			createAdrList(createAdrNode(rec_len));
			if ((rec_len = PC - loc_ctr)>MAX_TRECORD) rec_len = 0;
		}
		else if(rec_len + (PC-loc_ctr)==MAX_TRECORD){
			rec_len += PC-loc_ctr;
			createAdrList(createAdrNode(rec_len));
			rec_len = 0;
		}
		else if (DIRECTIVE<EXT&&EXT<CONST) rec_len += PC - loc_ctr;
		loc_ctr = PC;
	}
	createAdrList(createAdrNode(rec_len)); 			// create Address node list
	adr_temp = adr_head; 							// point first of the list
	
	prog_len = PC - start_adr;	

	fseek(fp_r, 0, 0); 								// reset file pointer
	
	BASE = PC = loc_ctr = start_adr;
	rec_len = line_ctr = 0;
	
	/* pass 2 */////////////////////////////////////////////////////////
	fprintf(fp_obj, "H%-6s%06X%06X\n", prog_name, start_adr, prog_len); // write Header Record
	while (fgets(buf, 90, fp_r)&&EXT!=-1) {	
		/*initialize variables*/////////////////////////////////////////
		para = label = mnemonic = operand = NULL; 
		temp = NULL;
		MEM_REF = temp_int = XBPE = OP = TA = EXT = OBJ = 0;
		line_ctr += 5;
		////////////////////////////////////////////////////////////////
		if (buf[0] == '.') { fprintf(fp_lst, "%3d\t%s", line_ctr, buf); continue; }		// continue if comment
		
		strcpy(full, buf);
		if(strchr(full, '\n')){
			*strchr(full,'\0')=-52;
			*strchr(full,'\n')= '\0';
		}
		else if(strchr(full,'\r')) *strchr(full,'\r') = '\0';							// exchange '\n' to '\0'	

		if (!rearrangeAsm(buf, &label, &mnemonic, &operand, &para)) { 					// rearrange instruction to proper format
			printf("file format error!\n"); return 0;
		}
		if (!(label || mnemonic || operand || para)) continue;
		if (mnemonic[0] == '+') { mnemonic++;  EXT = EXTENDED; XBPE = 1; } 				// '+' prefix for format4
		if (operand) {
			if (operand[0] == '@') { operand++; NI = 2; }
			else if (operand[0] == '#') { operand++; NI = 1; }
			else NI = 3;
		}
		/* directives */////////////////////////////////////////////////
		for (i = 0; i < 8; i++) { 
				if (!strcmp(mnemonic, DIRECTIVES[i]))break;
		}
		if(0<=i&&i<=7) {
			switch (i) {
			case 0:
			case 1:
			case 2:	if ((TA = strToDec(operand)) < 0||TA>MAX_MEM) { printf("unavailable value on line %d\n",line_ctr); return 0; }
					if (i == 0) { PC += WORD; EXT = WORD; }
					else if (i == 1) { PC += TA*WORD; EXT = DIRECTIVE; }
					else if (i == 2) { PC += TA*BYTE; EXT = DIRECTIVE; }
					break;
			case 3:	temp_int = strlen(para);
					if (!strcmp(operand, "X")) {
						for (i = 0; i < temp_int; i++) {
							if (para[i] != '0') break;
						}
						if (i == temp_int) para += (i - 1);
						else para += (i);
						if (60<(temp_int = strlen(para))) { printf("unavailable operand on line %d\n", line_ctr); return 0; }
						for (i = 0; i < temp_int; i++) {
							if (!(('A' <= para[i] && para[i] <= 'F') ||
								('a' <= para[i] && para[i] <= 'f') ||
								('0' <= para[i] && para[i] <= '9'))) {
								printf("unavailable operand on line %d\n", line_ctr); return 0;
							}
						}
						PC += temp_int / 2 + temp_int % 2;
					}
					else if (!strcmp(operand, "C")) {
						if (temp_int>30) { printf("unavailable operand on line %d\n", line_ctr); return 0; }
						for (i = 0; i< temp_int; i++) {
							if (!(0 <= para[i] && para[i] <= 255)) { printf("unavailable operand on line %d\n", line_ctr); return 0; }
						}
						PC += strlen(para);
					}
					else { printf("unavailable indicater on line %d\n", line_ctr); return 0; }
					EXT = CONST;
					break;
			case 4:	if((symb_temp=isValidSymb(operand))) BASE =symb_temp->adr;
					else {printf("no operand for base\n"); return 0;}
					EXT = -2;
					break;
			case 5: BASE = -1; //  base register can no longer be relied upon for addressing
					break;
			case 6:	EXT = -1;
					break;
			case 7:	if(strToDec(operand)==-1){printf("unavailable operand on line %d\n",line_ctr); return 0;}
					EXT = 0;
					break;
			}
		}
		/* mnemonic found in opcode Table *///////////////////////////////
		else if ((temp = opcodeMnemonic(mnemonic, 'a'))) { 
			if (!strcmp(temp->inst_type, "1")) { PC += BYTE; EXT = BYTE; NI = 0; }
			else if (!strcmp(temp->inst_type, "2")) { PC += DOUBLE; EXT = DOUBLE; NI = 0; }
			else if (!strcmp(temp->inst_type, "3/4")) {
				PC += WORD;
				if (EXT == EXTENDED) PC++;
				else EXT = WORD;
				OP = NI;
				if (para) {
					if (!strcmp(para, "X")) { 
						if (NI == 1 || NI == 2) { printf("wrong combinations of addressing mode on line %d\n",line_ctr); return 0; } // indexing mode after @# prefix
						XBPE += 8;
					}
					else{
						printf("unavailable parameter on line %d\n",line_ctr); return 0;
					}
				}
			}
			if (operationRules(EXT, mnemonic, operand, para, NI, &TA, line_ctr, &MEM_REF) == -1) { printf("error occured while assembling on line %d...\n",line_ctr); return 0; }
			OP += hexToDec(temp->opcode, 'a');
		}
		else{
			printf("unavilable instruction on line %d\n",line_ctr); return 0;
		}
		if (!(MEM_REF == CONST || MEM_REF==-1)) {
			if ((WORD == EXT) && (-2048 <= TA - PC && TA - PC <= 2047)) { TA -= PC; XBPE += 2; }				 	// PC relative
			else if ((WORD == EXT) && (0 <= TA - BASE && TA - BASE <= 4095)&&BASE!=-1) { TA -= BASE; XBPE += 4; } 	// BASE relative
		}
		if (TA < 0) TA += 4096;

		/* generate obj code *//////////////////////////////////////////////
		if (EXT == EXTENDED) { OBJ = OP * 16777216 + XBPE * 1048576 + TA; }
		else if (EXT == WORD) { OBJ = OP * 65536 + XBPE * 4096 + TA; }
		else if (EXT == DOUBLE) { OBJ = OP * 256 + TA; }
		else if (EXT == BYTE) { OBJ = OP; }
		else if (EXT == CONST) { OBJ = OP; }
		/*create Modification list*/////////////////////////////////////////
		if((EXT==EXTENDED && NI!=1) || MEM_REF==CONST){ 
			createModList(createModNode(loc_ctr-start_adr+1,OBJ,EXT));
		}
		/* write lst code */////////////////////////////////////////////////
		fprintf(fp_lst, "%3d\t", line_ctr);
		if(EXT>0)fprintf(fp_lst, "%04X",loc_ctr);
		fprintf(fp_lst,"\t%-40s",  full);
		printObj(EXT, OBJ, operand, para, temp_int,fp_lst);
		fprintf(fp_lst, "\n");
		/*write obj code *//////////////////////////////////////////////////
		if (loc_ctr == start_adr&&EXT) {
			fprintf(fp_obj, "T%06X%2X", loc_ctr, adr_temp->adr);
			adr_temp = adr_temp->next;
		}
		if (rec_len + (PC - loc_ctr) > MAX_TRECORD) {
			if (EXT) {
				fprintf(fp_obj, "\nT%06X%02X", loc_ctr, adr_temp->adr);
				adr_temp = adr_temp->next;
			}
			printObj(EXT, OBJ, operand, para, temp_int,fp_obj);
			rec_len = PC - loc_ctr;
		}
		else if (rec_len + (PC - loc_ctr) == MAX_TRECORD) {
			printObj(EXT, OBJ, operand, para, temp_int, fp_obj);
			if (EXT) {
				fprintf(fp_obj, "\nT%06X%02X", loc_ctr, adr_temp->adr);
				adr_temp = adr_temp->next;
			}
			rec_len = 0;
		}
		else {
			printObj(EXT, OBJ, operand, para, temp_int,fp_obj);
			if (EXT > DIRECTIVE) {
				rec_len += PC - loc_ctr;
			}
		}

		loc_ctr = PC;
	}
	/* write modification record *//////////////////////////////////////////
	m_temp = m_head;
	while(m_temp){
		if(m_temp->format==EXTENDED)
			fprintf(fp_obj,"\nM%06X%08X05",m_temp->adr,m_temp->obj);
		else if (m_temp->format==WORD)
			fprintf(fp_obj,"\nM%06X%06X03",m_temp->adr,m_temp->obj);
		m_temp=m_temp->next;
	}
	fprintf(fp_obj,"\nE%06X",start_adr);
	/* free allocated memory *//////////////////////////////////////////////
	while(adr_head){ 
		adr_temp=adr_head;
		adr_head=adr_head->next;
		free(adr_temp);
	}
	while(m_head){ 
		m_temp=m_head;
		m_head=m_head->next;
		free(m_temp);
	}
	return 1;
}

void freeSymbTab(){
	int i;
	SYMB_NODE* curr;
	for(i=0;i<26;i++){
		while(symb_head[i]){
			curr=symb_head[i];
			symb_head[i]=symb_head[i]->next;
			free(curr);
		}
	}
}
