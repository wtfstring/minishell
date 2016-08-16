#include "parse.h"
#include "externs.h"
#include "init.h"
#include "execute.h"
#include "builtin.h"

#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <linux/limits.h>
#include <fcntl.h>



void getname(char *name);
void print_command(void);

/*
*shell 主循环
*/
void shell_loop(void)
{
	while(1)
	{
		//每次读取命令之前初始化环境
		init();  
		//获取命令
		if (read_command() == -1)
			break;
		//解析命令
		parse_command();
		//print_command();
		//执行命令
		execute_command();
	}
	
	printf("\nexit\n");
	
}
/*读取命令，成功返回0，失败或者读取到文件结束符（EOF)返回-1 */
int read_command(void)
{
	//按行读取命令，cmdline中包含\n字符
	if (fgets(cmdline,MAXLINE,stdin) == NULL)
		return -1;
	return 0;
}
/*解析命令，成功解析到的命令个数，失败返回-1 */
int parse_command(void)
{
	/*    表示单条的命令的解析
	// 将ls -l 解析成ls\0-l\0,保存在valine数组中，并且不更改cmdline 
	char *cp = cmdline;
	char *avp = avline;
	
	int i = 0; 
	while(*cp != '\0')  //表示还未读到命令行尾
	{
		while (*cp == ' ' || *cp == '\t')  //去除左端的空格或者制表符
			cp++;
		if (*cp == '\0' || *cp == '\n')  //到达行尾表示命令结束
			break;
		cmd.args[i] = avp;
		
		//解析参数
		while(*cp != '\0' && *cp != ' ' && *cp != '\n' && *cp != '\t')
		{
			*avp++ = *cp++;
		}
		*avp++ = '\0';
		printf("[%s]\n",cmd.args[i]);
		i++;
	}
	*/
	
	//解析更一般的命令，如
	//cat < test.txt | grep -n public > test2.txt &
	if (check("\n"))
		return 0;
	
	//判断是否内部命令并执行它
	if (builtin())
		return 0;
	
	//1.解析第一条简单命令
	get_command(0);
	//2.判定是否有输入重定向符
	if (check("<"))
		getname(infile);  //解析输入重定向符后面的文件名称
	//3.判定是否有管道，用管道多少来表示基本命令的个数
	int i;
	for (i = 1; i < PIPELINE; ++i)
	{
		if (check("|"))
			get_command(i);
		else
			break;
	}
	//4.判定是否有输出重定向符
	if (check(">"))
	{
		if (check(">"))
			append = 1;
		getname(outfile);
	}
	//5.判定是否有后台作业
	if (check("&"))
		backgnd = 1;
	//6.判定命令结束，'\n'
	if (check("\n"))
	{
		cmd_count = i;
		return cmd_count;
	}
	else
	{
		fprintf(stderr,"Command line syntax error\n");
		return -1;
	}
}

/*执行命令，成功返回0，失败返回-1 */
int execute_command(void)
{
	/* 表示单条的命令的解析
	pid_t pid;
	pid = fork();
	if (pid == -1)  //表示创建进程失败
		ERR_EXIT("fork");
		
	//int ret;
	if (pid == 0)   //让子进程去执行命令
		execvp(cmd.args[0],cmd.args);   //execvp第一个参数表示命令名，第二个表示参数列表
	
	wait(NULL);  //父进程等待子进程的退出，不关心其状态
	*/
	
	execute_disk_command();
	
	return 0;
}


void print_command(void)
{
	int i;
	int j;
	printf("cmd_count = %d\n",cmd_count);
	if (infile[0] != '\0')
		printf("infile = [%s]\n",infile);
	if (outfile[0] != '\0')
		printf("outfile = [%s]\n",outfile);
	for (i = 0; i < cmd_count; ++i)
	{
		j = 0; 
		while(cmd[i].args[j] != NULL)
		{
			printf("[%s] ",cmd[i].args[j]);
			j++;
		}
		printf("\n");
	}
}

//解析简单命令到结构体数组cmd[i]中，提取下标为i的命令参数到数组avline中
//并且将COMMAND结构体中的args[]中的每个指针指向avline中的字符串
void get_command(int i)
{
	//cat < test.txt | grep -n public > test2.txt &
	
	int j = 0;
	int inword = 0;  //用于判定是单词内部还是单词外部
	while(*lineptr != '\0')
	{
		while(*lineptr == ' ' || *lineptr == '\t')  //去除空格
			lineptr++;
		
		//将第i条命令第j个参数指向avptr
		cmd[i].args[j] = avptr;
		//提取参数
		while(*lineptr != '\0' && *lineptr != ' ' && *lineptr != '\t' && *lineptr != '<' 
			&& *lineptr != '>' && *lineptr != '&' && *lineptr != '|' && *lineptr != '\n')
		{
			*avptr++ = *lineptr++;
			inword = 1;	
		}
			
		*avptr++ = '\0';
		
		switch (*lineptr)
		{
			case ' ':
			case '\t':
				inword = 0;
				j++;
				break;  //表示提取下一个参数
			case '<':
			case '>':
			case '&':
			case '|':
			case '\n':
				if (inword == 0)
					cmd[i].args[j] = NULL;
				return;  //表示本次命令（以‘|’分割的）提取结束
			default:
				return;
		}
	}
}

//将lineptr中的字符串与str进行匹配，成功返回1,lineptr移过所匹配的字符串，
//失败返回0，lineptr保持不变
int check(const char *str)
{
	char *p;
	while (*lineptr == ' ' || *lineptr == '\t')
		lineptr++;
	
	p = lineptr;
	while(*str != '\0' && *p == *str)
	{
		str++;
		p++;
	}
	
	if (*str == '\0')
	{
		lineptr = p;  //lineptr移过所匹配的字符串
		return 1;
	}
	
	//lineptr保持不变
	return 0;
}

void getname(char *name)
{
	while (*lineptr == ' ' || *lineptr == '\t')
		lineptr++;
	
	while(*lineptr != '\0' && *lineptr != ' ' && *lineptr != '\t' && *lineptr != '<' 
			&& *lineptr != '>' && *lineptr != '&' && *lineptr != '|' && *lineptr != '\n')
		{
			*name++ = *lineptr++;
		}
	*name = '\0';
}


