#include<stdio.h>
#include<unistd.h>
#include"Disk.h"
#include<string.h>
#define LINEBUFFER 1024
#define MAXPARA 10

//将输入的命令以空格分隔，保存至set。参数后加入一个NULL!!!
void div_command(char* str, char* cmdset[])
{
	int pos = 0;
	char temp[BUFFERSIZE];

	cmdset[0] = strtok(str, " ");
	if(empty_check(str))
	{
		cmdset[0] = "";
		cmdset[1] = NULL;
		return;
	}
	while(cmdset[pos]!=NULL)
	{
		pos++;
		cmdset[pos] = strtok(NULL, " ");
	}
	//空输入检查
	cmdset[pos] = NULL;
	//for(int i=0;i<pos;i++)
	//{printf("%s\n", cmdset[i]);}
}

//解析命令
void decode_command(char* cmdset[]){
	//ls
	if(strcmp(cmdset[0], "ls")==0){
		listRootDir();
	}
	//mkdir dirname
	else if(strcmp(cmdset[0], "mkdir")==0){
		if(cmdset[1]==NULL) printf("usage: mkdir [dirname]\n");
		else	createDir(cmdset[1]);
	}
	//touch name size
	else if(strcmp(cmdset[0], "touch")==0){
		if(cmdset[1]==NULL || cmdset[2]==NULL) printf("usage: touch [name] [size]\n");
		else	createFile(cmdset[1],cmdset[2][0]-'0');
	}
	//rm filename
	else if(strcmp(cmdset[0], "rm")==0){
		if(cmdset[1]==NULL) printf("usage: rm [name]\n");
		else	removeFile(cmdset[1]);
	}
	//rename oldname newname
	else if(strcmp(cmdset[0], "rename")==0){
		if(cmdset[1]==NULL || cmdset[2]==NULL) printf("usage: rename [oldname] [newname]\n");
		else	removeFile(cmdset[1]);
	}
	//read name
	else if(strcmp(cmdset[0], "read")==0){
		if(cmdset[1]==NULL) printf("usage: read [name]\n");
		else	readFile(cmdset[1]);
	}
	//write name
	else if(strcmp(cmdset[0], "write")==0){
		if(cmdset[1]==NULL) printf("usage: write [name]\n");
		else	writeFile(cmdset[1]);
	}
}

int main(){
	//当前登陆的用户
	int pid = getpid();
	printf("Welcome to Tomao File! Your pid is %d.\n",pid);
	char buffer[LINEBUFFER];
	memset(buffer, '\0', LINEBUFFER);
	while(1)
	{
		//输出提示符
		printf("$ ");
		char set[MAXPARA+1][LINEBUFFER];
		gets(buffer);
		//分割好的命令
		//printf("%s\n", buffer);
		div_command(buffer, set);
		//处理命令
		decode_command(set);
	}	
	return 0;
}
