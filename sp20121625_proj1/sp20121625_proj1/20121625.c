#include "20121625.h"

int hexToDec(char *hex){ // convert hexadecimal to decimal
	int len = strlen(hex),dec=0,low=0;
	int i,pow=1;
	if(len > 5) low=len-5;
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

INST_NODE* createInstNode(char* opcode, char* mnemonic, char* inst_type){
	INST_NODE *newNode=(INST_NODE*)malloc(sizeof(INST_NODE));
	strcpy(newNode->opcode,opcode);
	strcpy(newNode->mnemonic,mnemonic);
	strcpy(newNode->inst_type,inst_type);
	newNode->next=NULL;
	return newNode;
}

void createOpTable(INST_NODE* newNode,int idx){
	if(!op_head[idx]){op_head[idx]=newNode; op_tail[idx]=newNode;} // if hashtable's row is empty
	else{
		op_tail[idx]->next=newNode;
		op_tail[idx]=newNode;
	}
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

int opcodeMnemonic(char* inst_tok){
	int sum=0,i;
	INST_NODE *curr;
	for(i=0;i<strlen(inst_tok);i++) sum += inst_tok[i]+(inst_tok[i]-'A')*(i);
	for(curr=op_head[sum%20];curr;curr=curr->next){
		if(!strcmp(curr->mnemonic,inst_tok)){
			printf("opcode is %s\n",curr->opcode);
			return 1;
		}
	}
	return 0; // mnemonic not found
}

void opcodelist(){
	int i;
	INST_NODE* curr;
	for(i=0;i<20;i++){
		printf("%d : ",i);
		for(curr=op_head[i];curr;curr=curr->next){
			printf("[%s,%s]",curr->mnemonic,curr->opcode);
			if(curr!=op_tail[i]) printf("->");
		}
		printf("\n");
	}
}

int isValidInst(int tok_cnt, char **inst_tok, int comma_cnt, char** comma_pos,int inst_no){
	int res = 1,a[3];
	int i;

	if((0<=inst_no && inst_no<=9)){ // shell commands only
		if(tok_cnt) res = 0;
		if(comma_cnt) res = 0;
		return res;
	}
	
	else if(inst_no==10){ // 1 token
		if(comma_cnt) res=0;
		if(tok_cnt!=1) res =0;
		return res;
	}

	for(i=0;i<tok_cnt;i++){
		a[i]=hexToDec(inst_tok[i]);
		if(a[i]==-1) return 0;
	}

	if(inst_no==11 || inst_no==12){ // 0 to 2 tokens
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
	
	else if(inst_no==13 || inst_no==14){ // 2 tokens
		if(tok_cnt!=2) res=0;
		else if(comma_cnt!=1) res=0;
		else if(255<a[1]) res =0;
		else if(!(inst_tok[0]<=comma_pos[0]&&comma_pos[0]<=inst_tok[1])) res=0;
	}
	
	else if(inst_no==15 || inst_no==16){ // 3 tokens
		if(tok_cnt!=3) res=0;
		else if(comma_cnt!=2) res=0;
		else if(a[0]>a[1]) res=0;
		else if(255<a[2]) res =0;
		else if(!(inst_tok[0]<=comma_pos[0]&&comma_pos[0]<=inst_tok[1])) res=0;
		else if(!(inst_tok[1]<=comma_pos[1]&&comma_pos[1]<=inst_tok[2])) res=0;
	
	}
	return res;
}

void dir(){
	DIR *dirp=opendir(".");
	struct dirent *direntp;
	while((direntp= readdir(dirp))!=NULL){
		if(direntp->d_name[0]=='.') continue;
		else if(strchr(direntp->d_name,'.'))
			printf("%s*\n",direntp->d_name); // executable files
		else
			printf("%s/\n",direntp->d_name); // directory
	}
	closedir(dirp);
}	

HIS_NODE* createHisNode(char*inst){
	HIS_NODE *newNode=(HIS_NODE*)malloc(sizeof(HIS_NODE));
	strcpy(newNode->inst,inst);
	newNode->next=NULL;
	return newNode;
}

void createHistory(HIS_NODE* newNode){
	if(!his_head){his_head=newNode;his_tail=newNode;}
	else {
		his_tail->next = newNode;
		his_tail = newNode;
	}
}

void dump(int tok_cnt, char** inst_tok){
	static int adr_start=-1, adr_end=-1;
	int i,j=0;
	if(tok_cnt==2){
		adr_start = hexToDec(inst_tok[0]);
		adr_end = hexToDec(inst_tok[1]);
	}
	else if (!tok_cnt||tok_cnt==1){ 
		if(!tok_cnt)adr_start = adr_end+1;
		else adr_start=hexToDec(inst_tok[0]);
		adr_end = adr_start+159;
	}
	for(i=adr_start-adr_start%16;i<adr_end+(16-adr_end%16)&&i<MAX_MEM;i++){
		if(i==adr_start - adr_start%16 ||!(i%16)){
			j=i;
			printf("%05X ",i);
		}
		if(i<adr_start|| adr_end<i) printf("   ");
		else printf("%02X ",v_mem[i]);
		if(i!=adr_start-adr_start%16 &&!((i+1)%16)){
			printf(";");
			for(;j<=i;j++){
				if(32<=v_mem[j] && v_mem[j]<=126) printf("%c",v_mem[j]);
				else printf(".");
			}
			printf("\n");
		}
	}
	if(adr_end>=MAX_MEM) adr_start=adr_end = -1; // exception control for memory access
}

void fill(char **inst_tok){
	int adr_start=hexToDec(inst_tok[0]);
	int adr_end=hexToDec(inst_tok[1]);
	int val=hexToDec(inst_tok[2]);
	int i;
	for(i=adr_start;i<=adr_end;i++){
		v_mem[i] = val;
	}
}

void quit(){ // free allocated memory space
	HIS_NODE* his_curr;
	INST_NODE* op_curr,*tmp;
	int i;
	while(!his_head){
		his_curr=his_head;
		his_head=his_head->next;
		free(his_curr);
	}
	for(i=0;i<20;i++){
		op_curr=op_head[i];
		while(!op_tail[i]){
			tmp=op_curr;
			op_curr=op_curr->next;
			free(tmp);
		}
		free(op_head[i]);
	}
}

int main(void){
	char *sh_inst[17] = {"q","quit","h","help","d","dir","hi","history","reset","opcodelist","opcode","du","dump","e","edit","f","fill"};
	char *input,buf[200],full_inst[100];
	char *inst_tok[3],*comma_pos[3];
	int i,j,tok_cnt,comma_cnt=0,inst_flag=0,len;
	HIS_NODE *curr;
	
	if(!loadInstSet()) return 0;
	while(1){
		tok_cnt=comma_cnt=0;
		inst_flag=1;
		input = buf;
		printf("sicsim>");

		__fpurge(stdin);
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
		//
		if(!(input=strtok(input," \t\n")))
			continue;

		while ((inst_tok[tok_cnt] = strtok(NULL, " \t\n")) != NULL && tok_cnt < 4) {
			tok_cnt++;
		}
		
		for(i=0;i<17;i++) if(!strcmp(input,sh_inst[i])) break; // check if is valid shell command
		
		if(!isValidInst(tok_cnt,inst_tok,comma_cnt,comma_pos,i)||16<i||(MAX_INST_LEN<len)){ // check if is valid input format
			printf("invalid instruction\n");
			continue;
		}

		switch(i){
		case  0:
		case  1:return 0;
		case  2:
		case  3:printf("h[elp]\nd[ir]\nq[uit]\nhi[story]\ndu[mp][start, end]\ne[dit] address, value\n");
				printf("f[ill] start, end, value\nreset\nopcode mnemonic\nopcodelist\n");
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
		case 10:inst_flag=opcodeMnemonic(inst_tok[0]);
				break;
		case 11:
		case 12:dump(tok_cnt,inst_tok);
				break;
		case 13:
		case 14:v_mem[hexToDec(inst_tok[0])]=hexToDec(inst_tok[1]);
				break;
		case 15:
		case 16:fill(inst_tok);
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

