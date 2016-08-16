#ifndef _DEF_H_
#define _DEF_H_
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define ERR_EXIT(m) \
	do \
	{\
		perror(m); \
		exit(EXIT_FAILURE); \
	} while(0)
#define MAXLINE 1024  //输入命令行最大长度
#define MAXARG 20  //每个简单命令的参数最大个数
#define PIPELINE 5  //一个管道行中简单命令的最多个数
#define MAXNAME 100  //IO重定向文件名的最大长度

typedef struct command
{
	char *args[MAXARG+1];  //解析出的命令参数列表，最后一个位置保存空指针
	int infd;  //输入文件描述符
	int outfd;  //输出文件描述符
} COMMAND;

#endif
