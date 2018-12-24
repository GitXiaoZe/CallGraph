#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stack>
#include <algorithm>
#include <string>
#include <vector>

bool BeginWith(char *string, char *pattern){
	int i = 0, j = 0;
	int slen = strlen(string);
	int plen = strlen(pattern);
	while( i < slen &&  j < plen && string[i] == pattern[j]){ i++; j++;}
	if(j < plen) return false;
	return true;
}
int readFile(char* path,char* result){
	FILE *fp =  fopen(path,"r");
	if(fp == NULL){
		printf("Open %s ERROR\n",path);
		return -1;
	}
	FILE *out = fopen(result,"w");
	if(out == NULL){
		printf("Open %s ERROR\n",path);
		return -1;	
	}
	char line[1024];
	char temp[4];
	int len = 0, hierachy = -1, left = 0;
	while(!feof(fp)){
		char* buf = fgets(line,1023,fp);
		if(buf==NULL) break;
		if(BeginWith(line,"commit")){
			int len = strlen(line)-7;
			fwrite(line+7,sizeof(char),len,out);
		}			
	}
}

int main(){
	readFile("./log.txt","./log.commit");
	return 0;
}
