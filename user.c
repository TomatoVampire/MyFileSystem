#include<stdio.h>
#include<unistd.h>
#include"Disk.h"
#include<string.h>
#include<stdlib.h>
#define LINEBUFFER 1024
#define MAXPARA 10

int empty_check(char* s)
{
	//printf("empty check\n");
	if(s[0] == '\0') return 1;
	//bool flag = false;
	for(int i=0;s[i] != '\0';i++)
	{
		if(s[i] != ' ') 
		{
		//printf("not empty\n");
		return 0;
		}
	}
	return 1;
}

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

void help(){
	printf("ls :list directory\n");
	printf("mkdir [dirname] : make directory\n");
	printf("touch [name] [size] : create file\n");
	printf("rm [name] : remove file/directory\n");
	printf("rename [oldname] [newname] : rename file\n");
	printf("read [name] : read file\n");
	printf("write [name] : write file\n");
	printf("help : show help info\n");
	printf("exit : exit user\n");
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
		else	changeName(cmdset[1], cmdset[2]);
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
	
	//退出
	else if(strcmp(cmdset[0], "exit")==0){
		exit(1);
	}
	//帮助
	else if(strcmp(cmdset[0], "help")==0){
		help();
	}
	else if(cmdset[0]==NULL || empty_check(cmdset[0])){
		//输入空，省略
	}
	else{
		printf("command not found!\n");
	}
}

int main(){
	//当前登陆的用户
	int pid = getpid();
	printf("Welcome to Tomato File! Your pid is %d.\n",pid);
	char buffer[LINEBUFFER];
	memset(buffer, '\0', LINEBUFFER);
	
	//连接共享内存区
	struct FileSystem* shared;
	int shmid = shmget((key_t)1234, sizeof(struct FileSystem), IPC_CREAT|0666);
	if(shmid < 0){
		printf("shmget failed!\n");
		exit(1);
	}
	shared = (struct FileSystem*) shmat(shmid,0,0);
	//printf("File system attached at %p\n",shared);
	if(shmid != -1 ) printf("successfully got shm, shmid= %d\n",shmid);
	else{
		printf("shm get failed!\n");
		exit(1);
	}
	//初始化文件系统?
	global_file = shared;
	//initDisk();
	//初始化根目录?
	rootDirTable = &(global_file->root);
	//initRootDirTable();
	
	printf("type help to get info.\n");
	//读取输入
	while(1)
	{
		//输出提示符
		printf("> ");
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
