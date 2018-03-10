#include "20121625.h"

int hexToDec(char *hex, char mode){ // convert hexadecimal to decimal
	int len = strlen(hex),dec=0,low=0;
	int i,pow=1;
	if(len > 5&&mode=='s') low=len-5;
	for(i=len-1;i>=low;i--){
		if('A'<=hex[i] && hex[i] <='F')
			dec +=(10+hex[i]-'A')*pow;
		else if('a'<=hex[i] && hex[i]<='f')
			dec +=(10+hex[i]-'a')*pow;
		else if('0' <= hex[i] && hex[i] <= '9')
			dec +=(hex[i]-'0')*pow;
		else return -1; // unconvertable charactors
		pow*=16;
	}
	for(;i>=0;i--){ // exception control for unauthorized address value
		if(hex[i] != '0') return -1;
	}
	return dec;
}

int loadInstSet(){
	FILE* fp_r = fopen("opcode.txt","r");
	
	char opcode[3],mnemonic[10],inst_type[4];
	int i,sum;
	if(!fp_r){ // exception control
		printf("file open error\n");
		return 0;
	}
	op_head=(INST_NODE**)calloc(20,sizeof(INST_NODE*));
	op_tail=(INST_NODE**)calloc(20,sizeof(INST_NODE*));

	while(fscanf(fp_r,"%s %s %s",opcode,mnemonic,inst_type)!=EOF){
		sum=0;
	for(i=0;i<strlen(mnemonic);i++) sum += mnemonic[i]+(mnemonic[i]-'A')*(i);
		createOpTable(createInstNode(opcode,mnemonic,inst_type),sum%20);
	}
	return 1;
}

int isValidInst(int tok_cnt, char **inst_tok, int comma_cnt, char** comma_pos,int inst_no){
	int res = 1,a[3];
	int i;

	if((0<=inst_no && inst_no<=10)){ 
		if(tok_cnt) res = 0;
		if(comma_cnt) res = 0;
		return res;
	}
	
	else if(11<=inst_no && inst_no <= 13){ // 1 token
		if(comma_cnt) res=0;
		if(tok_cnt!=1) res =0;
		return res;
	}

	for(i=0;i<tok_cnt;i++){
		a[i]=hexToDec(inst_tok[i],'s');
		if(a[i]==-1) return 0;
	}

	if(inst_no==14 || inst_no==15){ // 0 to 2 tokens
		if(!tok_cnt){
			if(comma_cnt) res=0;
		}
		else if(tok_cnt==1){
			if(comma_cnt) res=0;
		}
		else if(tok_cnt==2){
			if(comma_cnt!=1) res=0;
			if(!(inst_tok[0]<=comma_pos[0] && comma_pos[0]<=inst_tok[1])) res=0; // position of comma
			if(a[0]>a[1]) res=0; // if start address is bigger than end address
		}
		else return 0;
	}
	
	else if(inst_no==16 || inst_no==17){ // 2 tokens
		if(tok_cnt!=2) res=0;
		else if(comma_cnt!=1) res=0;
		else if(255<a[1]) res =0;
		else if(!(inst_tok[0]<=comma_pos[0]&&comma_pos[0]<=inst_tok[1])) res=0;
	}
	
	else if(inst_no==18 || inst_no==19){ // 3 tokens
		if(tok_cnt!=3) res=0;
		else if(comma_cnt!=2) res=0;
		else if(a[0]>a[1]) res=0;
		else if(255<a[2]) res =0;
		else if(!(inst_tok[0]<=comma_pos[0]&&comma_pos[0]<=inst_tok[1])) res=0;
		else if(!(inst_tok[1]<=comma_pos[1]&&comma_pos[1]<=inst_tok[2])) res=0;
	
	}
	return res;
}

int main(void){
	char *sh_inst[20] = {"q","quit","h","help","d","dir","hi","history","reset","opcodelist","symbol","opcode","assemble","type","du","dump","e","edit","f","fill"};
	char *input,buf[200],full_inst[100];
	char *inst_tok[3],*comma_pos[3],obj[20],lst[20];
	int i,j,tok_cnt,comma_cnt=0,len;
	HIS_NODE *curr;
	INST_NODE *inst_flag;
	FILE *fp_asm,*fp_lst, *fp_obj;
	
	symb_head = (SYMB_NODE**)calloc(26,sizeof(SYMB_NODE*));
	symb_tail = (SYMB_NODE**)calloc(26,sizeof(SYMB_NODE*));

	if(!loadInstSet()) return 0;
	while(1){
		tok_cnt=comma_cnt=0;
		inst_flag=op_head[0];
		input = buf;
		printf("sicsim>");

		//__fpurge(stdin);
		fgets(buf, 200,stdin);
		buf[strlen(buf)-1] = '\0';
		strcpy(full_inst,buf);
		len = strlen(full_inst);
		for(i=0;i<len;i++){
			if(input[i]==','){
				comma_pos[comma_cnt++]=&input[i];
				input[i]=' ';
			}
		}
		
		if(!(input=strtok(input," \t\n")))
			continue;

		while ((inst_tok[tok_cnt] = strtok(NULL, " \t\n")) != NULL && tok_cnt < 4) {
			tok_cnt++;
		}
		
		for(i=0;i<20;i++) if(!strcmp(input,sh_inst[i])) break; // check if is valid shell command
		
		if(!isValidInst(tok_cnt,inst_tok,comma_cnt,comma_pos,i)||19<i||(MAX_INST_LEN<len)){ // check if is valid input format
			printf("invalid instruction\n");
			continue;
		}

		switch(i){
		case  0:
		case  1:return 0;
		case  2:
		case  3:printf("h[elp]\nd[ir]\nq[uit]\nhi[story]\ndu[mp][start, end]\ne[dit] address, value\n");
				printf("f[ill] start, end, value\nreset\nopcode mnemonic\nopcodelist\n");
				printf("assemble filename\ntype filename\nsymbol\n");
				break;
		case  4:
		case  5:dir();
				break;
		case  6:
		case  7:j=0;
				createHistory(createHisNode(full_inst));
				for(curr=his_head;curr;curr=curr->next){
					printf("%d %s\n",++j,curr->inst);
				}
				break;
		case  8:for (j = 0; j < MAX_MEM; j++) v_mem[j] = 0;
				break;
		case  9:opcodelist();
				break;
		case 10:symbol();
				break;
		case 11:inst_flag=opcodeMnemonic(inst_tok[0],'s');
				break;
		case 12:if((inst_tok[1]=strchr(inst_tok[0],'.'))) 
					if(strcmp(inst_tok[1],".asm")){ printf("not an asm file\n"); break;}
				if (!(fp_asm = fopen(inst_tok[0], "r"))) { printf("file open error\n"); break; }
				strtok(inst_tok[0],".");
				strcpy(lst,inst_tok[0]); strcpy(obj,inst_tok[0]);
				fp_lst=fopen(strcat(lst,".lst"),"w");
				fp_obj=fopen(strcat(obj,".obj"),"w");
				freeSymbTab();
				i=assemble(fp_asm,fp_lst,fp_obj);
				fclose(fp_asm); fclose(fp_lst); fclose(fp_obj); //file close
				if(!i) {
					remove(lst);
					remove(obj);
					freeSymbTab();
				}
				else{
					printf("\toutput file : [%s], [%s]\n",lst,obj);
				}
				break;
		case 13:type(inst_tok[0]);
				break;

		case 14:
		case 15:dump(tok_cnt,inst_tok[0],inst_tok[1]);
				break;
		case 16:
		case 17:v_mem[hexToDec(inst_tok[0],'s')]=hexToDec(inst_tok[1],'s');
				break;
		case 18:
		case 19:fill(inst_tok[0], inst_tok[1], inst_tok[2]);
				break;
		}
		if(!inst_flag){
			printf("invalid instruction\n");
			continue;
		}
		if(!(i==6 || i==7))createHistory(createHisNode(full_inst));
	}
	return 0;
}

