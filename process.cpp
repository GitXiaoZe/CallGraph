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
#include <errno.h>
#include <map>
#include <string>
#include <iostream>

#define MAX 40000
#define MAX_NAME 2048
#define LCS_MAX 20000
#define PATH_LEN 1024
#define MAX_FILE 1000
#define MAX_FUNC 1000
#define ASCII_START 1
#define ASCII_END 256
#define ASCII_LENGTH (ASCII_END-ASCII_START)

using namespace std;

void Recursive_TraversalFile(char* path,int len,char** file,char** str,int* index,int* strlen);
int myexec(char* inputpath,char *outputfile);
int mygit(char* path, char* commit_id);
void itoa(int num,char* temp);
int convertTree2String(char* path, char* result,int mapindex);


char relative_path[MAX_NAME];
char str1[MAX];
//char str2[MAX];
//short lcs[LCS_MAX<<1][LCS_MAX<<1];
short** lcs;

char* CMD = "cflow";
char* GIT = "git";
char* TDIR = "/dev/shm/result.cg";
char* TEMPDIR = NULL;

char** cfile[2];
char** fstr[2];
int* fstrlen[2];
int findex[2];

map<string,int> fn2i;
int mindex;
int mascii[MAX_FILE];
map<string,char> fn2c[MAX_FUNC];
map<string,int> fn2cnt[MAX_FUNC];


//copy src to dest, and return the copied bytes' lengtL
int mystrcpy(char *dest, char *src){
	if(dest==NULL || src == NULL){
		printf("dest is NULL or src is NULL\n");
		return -1;	
	}
	int len=0;
	while((dest[len]=src[len])!='\0'){ len++; }
	return len; 
}

bool EndWith(char *string, char *pattern){
	int i = 0;
	while(string[i] != '\0'){ i++;}
	int j = 0;
	while(pattern[j] != '\0'){ j++;}
	while( i >= 0 && j>=0 && string[i] == pattern[j]){
		i--;j--;
	}
	if(j >= 0) return false;
	return true;
}

bool Equal(char* str_1, char* str_2){
	int i = 0;
	while(str_1[i]!='\0' && str_2[i]!='\0' && str_1[i] == str_2[i]){ i++;}
	if(str_1[i]=='\0' && str_2[i]=='\0') return true;
	return false;
}

bool BeginWith(char *string, char *pattern){
	int i = 0, j = 0;
	int slen = strlen(string);
	int plen = strlen(pattern);
	while( i < slen &&  j < plen && string[i] == pattern[j]){ i++; j++;}
	if(j < plen) return false;
	return true;
}

void itoa(int num,char* temp){
	int i = 0;
	while(num > 0){
		int mod = num % 10;
		num /= 10;
		temp[i++] = '0'+mod;
	}
	temp[i] = '\0';
}

int myexec(char* inputpath,char *outputfile){
	pid_t pid;
	if((pid = fork()) == 0){
		if(execlp(CMD,CMD,inputpath,"-o",outputfile,NULL) < 0){
			printf("exec(%s %s -o %s) ERROR\n",CMD,inputpath,outputfile);
			exit(-1);
		}
		exit(0);
	}
	waitpid(-1,NULL,0);
	return 0;
}

int mygit(char* path, char* tag){
	pid_t pid;
	if((pid=fork())==0){
		if(execlp(GIT,GIT,"-C",path,"checkout",tag,NULL) < 0){
			printf("exec(git fail) %s\n",strerror(errno));
			exit(-1);
		}
		exit(0);
	}
	waitpid(-1,NULL,0);
	return 0;
}

void TraversalFile(char* path, char** file, char** str,int* index,int* strlen){
	int len = mystrcpy(relative_path,path);
	if(len <= 0){
		printf("path's length can not be zero!!!!!!!!\n");
		return;
	}
	Recursive_TraversalFile(path,len,file,str,index,strlen);
}

void Recursive_TraversalFile(char* path,int len,char** file,char** str,int* index,int* strlen){
	struct stat statbuf;
	if(lstat(path,&statbuf) < 0){
		printf("lstat : %s ERROR\n",path);
		return;	
	}
	if(S_ISREG(statbuf.st_mode)){
		if(EndWith(path,".c") || EndWith(path,".cpp")){ 
			string s(path);
			int mapindex;
			if( !(mapindex=fn2i[s]) ){
				fn2i[s] = mapindex = mindex;
				mascii[mindex] = ASCII_START;
				mindex++;
			}
			char *temp = (char*)malloc(len+1);
			int next = *index;
			mystrcpy(temp,path);
			file[next] = temp;
			myexec(path,TEMPDIR);
			int len = convertTree2String(TEMPDIR,str1,mapindex);
			str[next] = (char*)malloc(len+1);
			mystrcpy(str[next],str1);
			strlen[next] = len;
			*index = next + 1;
		}
	}else if(S_ISDIR(statbuf.st_mode)){
		DIR* dp = opendir(path);
		if(dp==NULL){
			printf("OpenDir %s ERROR\n",path);
			return;
		}
		struct dirent *entry = NULL;
		while((entry = readdir(dp))!=NULL){
			if(strcmp(entry->d_name,".") != 0 && strcmp(entry->d_name,".." ) != 0){
				relative_path[len] = '/';
				int temp = mystrcpy(relative_path+len+1,entry->d_name);
				Recursive_TraversalFile(relative_path,len+1+temp,file,str,index,strlen);
				relative_path[len] = '\0';
			}
		}
		closedir(dp);
	}else{
	}
}

int convertTree2String(char* path, char* result,int mapindex){
	FILE *fp =  fopen(path,"r");
	if(fp == NULL){
		printf("Open %s ERROR\n",path);
		return -1;
	}
	char line[1024];
	char temp[4];
	int len = 0, hierachy = -1, left = 0;
	while(!feof(fp)){
		char* buf = fgets(line,1023,fp);
		if(buf==NULL) break;
		int space_cnt = 0,i = 0;
		for(;line[i]==' ';i++);
		char *tpath = line + i;
		space_cnt = i >> 2;
		int temp_len = len;
		for(;line[i]!='(';i++);
		if(space_cnt > hierachy){
			hierachy++;	result[len++] = '{'; left++;
		}else if(space_cnt < hierachy){
			hierachy--;	result[len++] = '}'; left--;
		}else if(!space_cnt){
			while(left-- > 0) result[len++] = '}';
		}
		char nc = line[i] = '\0';
		string s(tpath);
		if ( (nc = fn2c[mapindex][s]) == '\0'){
			int na  = mascii[mapindex]++;
			fn2c[mapindex][s] = nc = (char)(na%ASCII_END)+1;
			fn2cnt[mapindex][s] = na / (ASCII_END+1);
		}
		int ccnt = fn2cnt[mapindex][s];
		for(int i=0; i<= ccnt;i++)
			result[len++] = nc;
	}
	while(left-->0) result[len++] = '}';
	result[len++] ='\0';
	fclose(fp);

	return len - 1;
}

int LCS(char* path,int n,int m,char* str1,char* str2){
	if(n > 39990 || m > 39990) printf("%s %d %d\n",path,n,m);
	for(int i=0;i < n; i++){
		for(int j=0; j < m; j++){
			if(str1[i] == str2[j]){
				lcs[i+1][j+1] = lcs[i][j] + 1;
			}else{
				lcs[i+1][j+1] = max(lcs[i][j+1],lcs[i+1][j]);	
			}
		}
	}
	return lcs[n][m];
}

int begin(char *path,char* output,char *commit_id_file){
	mindex = 1;

	FILE *fp = fopen(commit_id_file,"r");
	if(fp == NULL){
		printf("Open commit_id_file Error\n");
		exit(-1);
	}
	FILE *out = fopen(output,"w");
	if(out == NULL){
		fclose(fp);
		printf("Open commit_id_file Error\n");
		exit(-1);
	}
	for(int i=0;i<2;i++){
		cfile[i] = (char**)malloc(sizeof(char*)*MAX_FILE);
		fstr[i] = (char**)malloc(sizeof(char*)*MAX_FILE);
		fstrlen[i] = (int*)malloc(sizeof(int)*MAX_FILE);
		findex[i] = 0;
	}
	lcs = (short**)malloc(sizeof(short*)*(LCS_MAX<<1));
	for(int i=0;i<(LCS_MAX<<1);i++) lcs[i] = (short*)malloc(sizeof(short)*(LCS_MAX<<1));
	TraversalFile(path,cfile[0],fstr[0],findex,fstrlen[0]);

	int bias = strlen(path);
	char cid[60];
	bool flag[MAX_FILE];
	int cnt = 0;
	while(!feof(fp)){
		cnt++;
		printf("%d\n",cnt);
		fprintf(out,"----------%d------------\n",cnt);
		char *temp = fgets(cid,60,fp);
		if(temp==NULL)	break;
		int i;
		for(i=0;cid[i]!='\n';i++);
		cid[i] = '\0';
		mygit(path,cid);
		TraversalFile(path,cfile[1],fstr[1],findex+1,fstrlen[1]);
		for(int i=0; i < findex[1];i++) flag[i] = true;
		for(int i=0; i < findex[0];i++){
			for(int j=0; j < findex[1];j++){
				if( flag[j] && Equal(cfile[0][i]+bias,cfile[1][j]+bias) ){
					flag[j] = false;
					if(fstrlen[0][i] !=0 && fstrlen[0][i] == fstrlen[1][j] && Equal(fstr[0][i],fstr[1][j])){
						fprintf(out,"file=%s  1.000\n",cfile[0][i]);
						continue;
					}
					int lcslen = LCS(cfile[0][i],fstrlen[0][i],fstrlen[1][j],fstr[0][i],fstr[1][j]);
					if(lcslen!=0)	fprintf(out,"file=%s  %.3lf\n",cfile[0][i],lcslen/(double)fstrlen[0][i]);
				}
			}
		}
		for(int i=0; i < findex[0]; i++){
			free(cfile[0][i]);
			free(fstr[0][i]);
		}
		char** c = cfile[0];
		cfile[0] = cfile[1];
		cfile[1] = c;

		c = fstr[0];
		fstr[0] = fstr[1];
		fstr[1] = c;

		int *tmp = fstrlen[0];
		fstrlen[0] = fstrlen[1];
		fstrlen[1] = tmp;
		findex[0] = findex[1];
		findex[1] = 0;
	}
	fclose(fp);
	fclose(out);
	for(int i=0;i<findex[0];i++){
			free(cfile[0][i]);
			free(fstr[0][i]);
	}

	for(int j=0; j < 2 ;j++){
		free(fstrlen[j]);
		free(cfile[j]);
		free(fstr[j]);
	}
	for(int i=0;i<(LCS_MAX<<1);i++) free(lcs[i]);
	free(lcs);

	free(TEMPDIR);
}
int main(int argc, char* argv[]){
	
	if(argc < 5){
		printf("Usage ./main <src_directory> <output_path> <tag_file> <start>\n");
		return -1;
	}
	TEMPDIR = (char*)malloc(256);
	int len = mystrcpy(TEMPDIR,TDIR);
	mystrcpy(TEMPDIR + len,argv[4]);

	begin(argv[1],argv[2],argv[3]);
	return 0;
}
