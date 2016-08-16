#include "init.h"
#include "externs.h"
#include <string.h>
#include <signal.h>
#include <stdio.h>

void sigint_handler(int sig);  //信号处理函数

void setup(void)
{
	signal(SIGINT,sigint_handler);  //安装信号
	signal(SIGQUIT,SIG_IGN);  //忽略退出信号
}

void sigint_handler(int sig)
{
	printf("\n[minishell]$ ");
	fflush(stdout);
}

void init(void)
{
	memset(cmd,0,sizeof(cmd));
	int i;
	for (i = 0; i < PIPELINE; ++i)
	{
		cmd[i].infd = 0;
		cmd[i].outfd = 1;
	}
	memset(cmdline,0,sizeof(cmdline));
	memset(avline,0,sizeof(avline));
	lineptr = cmdline;
	avptr = avline;
	
	memset(infile,0,sizeof(infile));
	memset(outfile,0,sizeof(outfile));
	cmd_count = 0;
	backgnd = 0;
	append = 0;
	lastpid = 0;
	
	printf("[minishell]$ ");
	fflush(stdout);  //清空输出流使printf输出,与\n具有相同的功能
}
