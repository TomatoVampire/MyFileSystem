#include<sys/shm.h>
#include<stdio.h>
#include<unistd.h>
#include"Disk.h"
//#include"File.h"


//测试
void test(){
	printf("%d\n",createFile("n",1));
	printf("%d\n",createDir("dir"));
	//printf("%d\n",rootDirTable->dirs[0].inode.blockCount);
	//printf("%d\n",rootDirTable->dirCount);
	printf("%d\n",findEntryInRoot("n"));
	listRootDir();
	printf("%d\n",changeName("dir","newd"));
	listRootDir();
	readFile("n");
	writeFile("n");
	//printf("%d\n",allocBlock(1));
}

int main(){
	struct FileSystem* shared;
	int shmid = shmget((key_t)1234, sizeof(struct FileSystem), 0666 | IPC_CREAT);
	shared = (struct FileSystem*) shmat(shmid,0,0);
	printf("File system attached at %p\n",shared);
	//初始化文件系统
	global_file = shared;
	initDisk();
	//初始化根目录
	rootDirTable = &(global_file->root);
	initRootDirTable();
	
	//test
	test();
	
	printf("press any key to detach file system.\n");
	getchar();
	//解绑空间
	shmdt(shared);
	shmctl(shmid, IPC_RMID, 0);
	printf("file system close.\n");
	return 0;
}
