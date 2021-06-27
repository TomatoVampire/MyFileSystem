#include<sys/shm.h>
#include<stdio.h>
#include<unistd.h>
#include<semaphore.h>
#include<fcntl.h>
#include<string.h>
//#include"Disk.h"

#define BUFFERSIZE 100
#define MAXDIRSIZE 15
#define MAXREADER 5

//inode
struct iNode{
	int startBlock;//开始块号
	int blockCount;//占用内存的块数（占有内存startBlock -> startBlock+count-1）
	int dataSize;//占有内存的大小，单位为B
	int link;//连接数
	int readptr;//读指针，所在内存的位置，单位为B?
	
	//信号量？
	sem_t *read_sem;
	sem_t *write_sem;
	
	
};

//目录项
struct DirEntry{
	char fileName[BUFFERSIZE];
	char type;//0目录 1文件
	struct iNode inode;
};

//目录表
struct DirTable{
	int dirCount;//目录项数
	struct DirEntry dirs[MAXDIRSIZE];//目录项列表
};

//TODO 在使用函数前先指定根目录指针！
//全局的根目录表！
struct DirTable* rootDirTable;

//TODO
//初始化根目录
void initRootDirTable(){
	//目录项数=0
	rootDirTable->dirCount=0;
}

//在根目录的目录项中寻找含有该名字的目录，找到返回该目录项在根目录的序号，否则返回-1
int findEntryInRoot(char* name){
	int dircount = rootDirTable->dirCount;
	for(int i=0;i<dircount;i++){
		if(strcmp(name,rootDirTable->dirs[i].fileName) == 0)
			return i;
	}
	return -1;
}

//创建目录项，返回在根目录的序号
int createDirEntry(char* name, char type){
	int dircount = rootDirTable->dirCount;
	//名字
	strcpy(name, rootDirTable->dirs[dircount].fileName);
	//类型
	rootDirTable->dirs[dircount].type = type;
	
}

//创建文件的inode
int createINode(int entry, int start, int size){
	//获取目录项
	struct DirEntry* direntry = &(rootDirTable->dirs[entry]);
	struct iNode* ninode = &(direntry->inode);
	//创建inode
	ninode->startBlock = start;
	ninode->blockCount = size;
	ninode->dataSize = 0;
	ninode->readptr = 0;
	//信号量
	ninode->read_sem = sem_open("read_sem", O_CREAT, 0644, MAXREADER);
	if(ninode->read_sem == SEM_FAILED){
		printf("create read_sem failed!\n");
		return -1;
	}
	ninode->write_sem = sem_open("write_sem", O_CREAT, 0644, 1);
	if(ninode->write_sem == SEM_FAILED){
		printf("create read_sem failed!\n");
		return -1;
	}
	return entry;
}

//在根目录创建目录，成功则返回根目录的目录项序号
int createDir(char* name){
	
}

//在根目录创建文件，成功则返回根目录的目录项序号
int createFile(char* name, int size){
	if(rootDirTable->dirCount >= MAXDIRSIZE){
		printf("system has reached max dir number!\n");
		return -1;
	}
	if(strlen(name) >= BUFFERSIZE){
		printf("file name should be less than %d chars!\n",BUFFERSIZE);
		return -1;
	}
	//创建目录项(检查是否同名！)
	if(findEntryInRoot(name)!=-1){
		printf("already has a file/dir of same name!\n");
		return -1;
	}
	int curdir=rootDirTable->dirCount;
	//分配磁盘空间
	int nblock = allocBlock(size);
	//创建inode
	

}


