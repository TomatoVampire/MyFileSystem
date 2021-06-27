#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
//文件系统的客户端，运行在创建共享内存之后

enum CommandType{
	MKDIR,
	RMDIR,
	EDITNAME,
	OPEN,
	EDITFILE,
	RM,
	LS,
	EXIT,
	HELP
};



void decode_command(char* str){
	if(strcmp(str,"mkdir")==0){
	}
	else if(strcmp(str,"rmdir")==0){
	}
	else if(strcmp(str,"editanme")==0){
	}
	else if(strcmp(str,"exit")==0){
		exit(1);
	}

}

void getinput(){
	while(1){
		printf("%d > ",getpid());
		char str[1024];
		scanf("%s",str);
		printf(str);
		decode_command(str);
		printf("\n");
	}
}


int main(){
	//当前登陆的用户
	int pid = getpid();
	printf("Welcome to Tomao File! Your pid is %d.\n",pid);
	getinput();
	
	return 0;
}
