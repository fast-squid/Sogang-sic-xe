#include "20121625.h"

INST_NODE* opcodeMnemonic(char* mnemonic, char caller) {
	int sum = 0, i;
	INST_NODE *curr;
	for (i = 0; i<strlen(mnemonic); i++) sum += mnemonic[i] + (mnemonic[i] - 'A')*(i);
	for (curr = op_head[sum % 20]; curr; curr = curr->next) {
		if (!strcmp(curr->mnemonic, mnemonic)) {
			if(caller=='s') printf("opcode is %s\n", curr->opcode);
			return curr;
		}
	}
	return NULL; // mnemonic not found
}

void opcodelist() {
	int i;
	INST_NODE* curr;
	for (i = 0; i<20; i++) {
		printf("%d : ", i);
		for (curr = op_head[i]; curr; curr = curr->next) {
			printf("[%s,%s]", curr->mnemonic, curr->opcode);
			if (curr != op_tail[i]) printf("->");
		}
		printf("\n");
	}
}

int type(char* filename) { 
	FILE* fp_r=fopen(filename,"r");
	char buf[100];
	char * temp;
	if (!strchr(filename, '.')){ printf("not an excutable file\n"); return 0;}
	if (!fp_r) { printf("file open error\n");return 0; }
	while (fgets(buf,100,fp_r)){
		if((temp=strchr(buf,'\r'))) *temp= ' ';
		printf("%s", buf);
	}
	printf("\n");
	fclose(fp_r);
	return 1;
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


void dump(int tok_cnt, char* str_adr, char* end_adr) {
	static int adr_start = -1, adr_end = -1;
	int i, j = 0;
	if (tok_cnt == 2) {
		adr_start = hexToDec(str_adr,'s');
		adr_end = hexToDec(end_adr,'s');
	}
	else if (!tok_cnt || tok_cnt == 1) {
		if (!tok_cnt)adr_start = adr_end + 1;
		else adr_start = hexToDec(str_adr,'s');
		adr_end = adr_start + 159;
	}
	for (i = adr_start - adr_start % 16; i<adr_end + (16 - adr_end % 16) && i<MAX_MEM; i++) {
		if (i == adr_start - adr_start % 16 || !(i % 16)) {
			j = i;
			printf("%05X ", i);
		}
		if (i<adr_start || adr_end<i) printf("   ");
		else printf("%02X ", v_mem[i]);
		if (i != adr_start - adr_start % 16 && !((i + 1) % 16)) {
			printf(";");
			for (; j <= i; j++) {
				if (32 <= v_mem[j] && v_mem[j] <= 126) printf("%c", v_mem[j]);
				else printf(".");
			}
			printf("\n");
		}
	}
	if (adr_end >= MAX_MEM) adr_start = adr_end = -1; // exception control for memory access
}

void fill(char *str_adr, char* end_adr, char* mem_val) {
	int adr_start = hexToDec(str_adr,'s');
	int adr_end = hexToDec(end_adr,'s');
	int val = hexToDec(mem_val,'s');
	int i;
	for (i = adr_start; i <= adr_end; i++) {
		v_mem[i] = val;
	}
}

void quit() { // free allocated memory space
	HIS_NODE* his_curr;
	INST_NODE *tmp;
	int i;
	while (his_head) {
		his_curr = his_head;
		his_head = his_head->next;
		free(his_curr);
	}
	for (i = 0; i<20; i++) {
		while (op_head[i]) {
			tmp = op_head[i];
			op_head[i] = op_head[i]->next;
			free(tmp);
		}
		free(op_head);
	}
}
