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

using namespace std;

#define MAX_COMMIT 5000
#define MAX_FILE 200
// map<string,int> fn2i;
// int mindex;
// int mascii[MAX_FILE];
// map<string,char> fn2c[MAX_FUNC];
map<string,int> fn2i;
int fcnt;
double counter[MAX_FILE][MAX_COMMIT];


bool BeginWith(char *string, char *pattern){
	int i = 0, j = 0;
	int slen = strlen(string);
	int plen = strlen(pattern);
	while( i < slen &&  j < plen && string[i] == pattern[j]){ i++; j++;}
	if(j < plen) return false;
	return true;
}

int myread(char* path){
    FILE *fp =  fopen(path,"r");
	if(fp == NULL){
		printf("Open %s ERROR\n",path);
		return -1;
	}
    char line[1024];
    int cnt = -1;
	while(!feof(fp)){
        char* buf = fgets(line,1023,fp);
		if(buf==NULL) break;
        //cnt++;
        if(BeginWith(line,"-----")){
            cnt ++;
            continue;
        }
        int i;
        for(i = 0;line[i]!='=';i++);
        char *path = line + i + 1;
        for(;line[i]!=' ';i++);
        int end = i;
        i+=2;
        char* r = line + i;
        for(i = 0;line[i]!='\n';i++);
        line[i] = '\0';
        line[end] = '\0';
        string s(path);
        int index;
        if( (index = fn2i[path]) == 0){
            fn2i[path] = index = fcnt;
            fcnt++;
            for(int j = 0; j < cnt ;j++)
                counter[index][j] = 0.0;
        }
        double rate = atof(r);
        counter[index][cnt] = rate;
	}
    cout <<"123" << endl;;
    //printf("%d\n",cnt);
    map<string,int>::iterator ite;
    ite = fn2i.begin();
    while(ite != fn2i.end()){
        //printf("%s",ite->first);
        cout << ite->first;
        int findex = ite->second;
        for(int i=0;i < MAX_COMMIT;i++){
            if ( counter[findex][i] >= 0.0){
                printf(",%lf",counter[findex][i]);
            }else break;
        }
        printf("\n");
        ite++;
    }
    
    fclose(fp);
    
    return 0;

}

int main(int argc,char** argv){
    if(argc < 2){
        printf("Usage : ./main <file>\n");
        exit(0);
    }
    fcnt = 1;
    for(int i=1; i<MAX_FILE; i++){
        for(int j=0;j<MAX_COMMIT; j++){
            counter[i][j] = -1.0;
        }
    }
    myread(argv[1]);
    return 0;
}