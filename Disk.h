#include<sys/shm.h>
#include<stdio.h>
#include<unistd.h>
#include<semaphore.h>
#include<fcntl.h>
#include<string.h>
//#include"Disk.h"

#define MAX_STORAGE_SIZE 100*1024*1024 //磁盘大小（B）
#define BLOCK_SIZE 1024		//盘块大小（B）
#define BLOCK_COUNT MAX_STORAGE_SIZE / BLOCK_SIZE //盘块总数

#define BUFFERSIZE 100			//缓冲区大小（char）
#define MAXDIRSIZE 15			//最大目录项数
#define MAXREADER 5			//最大读者数（最大写者为1）

/////////////////////////////////////////////////////////////////文件
//inode索引结点
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
		if(strcmp(name, rootDirTable->dirs[i].fileName) == 0)
			return i;
	}
	return -1;
}

//创建目录项，返回在根目录的序号
int createDirEntry(char* name, char type){
	int dircount = rootDirTable->dirCount;
	//名字
	strcpy(rootDirTable->dirs[dircount].fileName,name);
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
	ninode->link = 1;
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
	//printf("create 2 sems success.\n");
	return entry;
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
	createDirEntry(name,'1');//0目录 1文件
	//分配磁盘空间
	int nblock = allocBlock(size);
	//创建inode
	int success = createINode(curdir,nblock,size);
	//目录数+1
	rootDirTable->dirCount++;
	if(success >= 0) {
		return curdir;
	}
	else return -1;
}

//在根目录创建目录，成功则返回根目录的目录项序号
int createDir(char* name){
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
	createDirEntry(name,'0');//0目录 1文件
	//分配磁盘空间，目录占用1个块大小
	int nblock = allocBlock(1);
	//创建inode
	int success = createINode(curdir,nblock,1);
	//目录数+1
	rootDirTable->dirCount++;
	if(success >= 0) {
		return curdir;
	}
	else return -1;
}

//删除目录项！
int removeEntry(int entry){
	int count = rootDirTable->dirCount;
	for(int i=entry;i<count-1;i++){
		//逐个向前挪动
		rootDirTable->dirs[i] = rootDirTable->dirs[i+1];
	}
	rootDirTable->dirCount--;
	return 1;
}

//删除文件，参数为文件名
int removeFile(char* name){
	int entry = findEntryInRoot(name);
	//未找到
	if(entry==-1){
		printf("filename not found!\n");
		return 0;
	}
	//TODO 检查是否有进程在读？
	
	//连接数为1则可以删除
	if(rootDirTable->dirs[entry].inode.link == 1){
	//开始块与移除大小
	int stblock, rmsize;
	stblock = rootDirTable->dirs[entry].inode.startBlock;
	rmsize = rootDirTable->dirs[entry].inode.blockCount;
	//删除对应的盘块(bitmap操作将对应盘块)
	releaseBlock(stblock,rmsize);
	//删除信号量
	sem_close(rootDirTable->dirs[entry].inode.read_sem);
	sem_close(rootDirTable->dirs[entry].inode.write_sem);
	sem_unlink("read_sem");
	sem_unlink("write_sem");
	
	//删除目录项
	removeEntry(entry);
	return 1;
	}
	else{
		printf("file is still in use! cannot close.\n");
		return 0;
	}
}

//显示根目录的内容ls
void listRootDir(){
	printf("Root Directory:\n");
	printf("name\ttype\tstart\tsize\t\n");
	for(int i=0;i<rootDirTable->dirCount;i++){
		printf("%s\t%c\t%d\t%d\t\n",rootDirTable->dirs[i].fileName, rootDirTable->dirs[i].type,
		rootDirTable->dirs[i].inode.startBlock, rootDirTable->dirs[i].inode.blockCount);
	}
	if(rootDirTable->dirCount==0){
		printf("root directory is empty.\n");
	}
}

//修改目录或文件的名字
int changeName(char* oldname, char* newname){
	int entry = findEntryInRoot(oldname);
	//未找到
	if(entry==-1){
		printf("filename not found!\n");
		return 0;
	}
	if(strlen(oldname)  >= BUFFERSIZE || strlen(newname)  >= BUFFERSIZE){
		printf("filename too long!\n");
		return 0;
	}
	//修改
	strcpy(rootDirTable->dirs[entry].fileName, newname);
	return 1;
}

//TODO 读文件
int readFile(char* name){
	printf("start reading file...\n");
	int entry = findEntryInRoot(name);
	//未找到
	if(entry==-1){
		printf("filename not found!\n");
		return 0;
	}
	if(rootDirTable->dirs[entry].type=='0'){
		printf("cannot read a directory.\n");
		return 0;
	}
	int val;
	//wait读
	rootDirTable->dirs[entry].inode.read_sem = sem_open("read_sem",0);
	if(sem_wait(rootDirTable->dirs[entry].inode.read_sem) == -1){
		printf("read_sem error!\n");
		return 0;
	}
	sem_getvalue(rootDirTable->dirs[entry].inode.read_sem, &val);
	//printf("wait complete.\n");
	//如果为第一个读就wait写
	if(val == MAXREADER-1){
		rootDirTable->dirs[entry].inode.write_sem = sem_open("write_sem",0);
		if(sem_wait(rootDirTable->dirs[entry].inode.write_sem) == -1){
			printf("write_sem error!\n");
			return 0;
		}
		//printf("wait write_sem complete.\n");
	}
	printf("Read [%s] complete. press enter to exit read.",rootDirTable->dirs[entry].fileName);
	getchar();
	//关闭文件
	if(closeFile(name)!=1){
		printf("close file error!\n");	
		return 0;
	}
	
	return 1;

}

//TODO 关闭文件(读文件完毕)（连接数-1）
int closeFile(char* name){
	int entry = findEntryInRoot(name);
	//未找到
	if(entry==-1){
		printf("filename not found!\n");
		return 0;
	}
	if(rootDirTable->dirs[entry].type=='0'){
		printf("cannot read a directory.\n");
		return 0;
	}
	//struct iNode node = rootDirTable->dirs[entry].inode;
	int val;
	//如果为最后一个读则signal写
	rootDirTable->dirs[entry].inode.read_sem = sem_open("read_sem",0);
	sem_getvalue(rootDirTable->dirs[entry].inode.read_sem, &val);
	if(val == MAXREADER-1){
		rootDirTable->dirs[entry].inode.write_sem = sem_open("write_sem",0);
		sem_post(rootDirTable->dirs[entry].inode.write_sem);
	}
	//signal读
	sem_post(rootDirTable->dirs[entry].inode.read_sem);
	return 1;
}

//TODO 写文件
int writeFile(char* name){
	printf("start writing file...\n");
	int entry = findEntryInRoot(name);
	//未找到
	if(entry==-1){
		printf("filename not found!");
		return 0;
	}
	if(rootDirTable->dirs[entry].type=='0'){
		printf("cannot write a directory.\n");
		return 0;
	}
	
	//wait写
	rootDirTable->dirs[entry].inode.write_sem = sem_open("write_sem",0);
	if(sem_wait(rootDirTable->dirs[entry].inode.write_sem) == -1){
			printf("write_sem error!\n");
			return 0;
	}
	printf("Read [%s] complete. press enter to exit write.",rootDirTable->dirs[entry].fileName);
	getchar();
	//post写
	rootDirTable->dirs[entry].inode.write_sem = sem_open("write_sem",0);
	sem_post(rootDirTable->dirs[entry].inode.write_sem);
	return 1;
}

////////////////////////////////////////////////////////////////////硬盘///////////////////////////////////////////////////////
struct FileSystem{
	char* storageStartAddr;//磁盘起始地址?
	char storageDisk[MAX_STORAGE_SIZE];
	//目录项等
	struct DirTable root;
};

//当前程序中使用的文件的struct指针
struct FileSystem* global_file;

//初始化磁盘，添加bitmap
void initDisk(){
	//共享内存分配完毕后，进行系统初始化
	global_file->storageStartAddr = global_file->storageDisk;
	for(int i=0;i<BLOCK_COUNT;i++){
		//前为bitmap,'0'为未使用，1为使用
		global_file->storageDisk[i] = '0';
	}
	//bitmap本身占有的空间置1
	int bitmapSize = BLOCK_COUNT*sizeof(char) / BLOCK_SIZE;
	for(int i=0;i<bitmapSize;i++){
		//
		global_file->storageDisk[i] = '1';
	}
	printf("file initiation complete.\n");
}

//连续分配size个盘块的空间，成功则返回首个盘块的数字，否则返回-1
int allocBlock(int size){
	int start=0;
	int serial=0;
	for(int i=0;i<BLOCK_COUNT;i++)
	{
		if(global_file->storageDisk[i] == '0'){
			if(serial==0)
				start = i;
			serial++;
			if(serial == size)
			{
			//分配成功
				for(int j=start;j<start+serial;j++)
				{
					global_file->storageDisk[j] = '1';
				}
				return start;
			}
		}
		else{
			serial=0;
		}
	}
	printf("fail to alloc %d blocks of memory.\n",size);
	return -1;
}

//获取指定盘块的首地址
char* getPointerOfBlock(int block){
	//if(global_file->storageDisk[i])
	return global_file->storageDisk + (block*BLOCK_SIZE);
}

//获取指针所在的盘块号
int getBlockOfPointer(char* pointer){
	return (pointer - global_file->storageDisk)/BLOCK_SIZE;
}

//释放block和后续n个块的空间
int releaseBlock(int block, int n){
	for(int i=0;i<n;i++){
		//仅将对应盘块的bitmap置0
		global_file->storageDisk[block+i] = '0';
	}
	return 1;
}


