#include "execute.h"
#include "externs.h"

#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <linux/limits.h>
#include <fcntl.h>


int execute_disk_command(void)
{
	if (cmd_count == 0)
		return 0;
	
	if (infile[0] != '\0')  //表示有输入重定向文件
		cmd[0].infd = open(infile,O_RDONLY);
	
	if (outfile[0] != '\0')  //表示有输出重定向文件
	{
		if (append)
			cmd[cmd_count - 1].outfd = open(outfile,O_WRONLY | O_CREAT | O_APPEND,0666);
		else
			cmd[cmd_count - 1].outfd = open(outfile,O_WRONLY | O_CREAT | O_TRUNC,0666);
	}
	
	//表示后台作业,因为后台作业不会调用wait等待子进程退出
	//为避免僵尸进程，可以忽略SIGCHLD信号（SIG_IGN)
	if (backgnd == 1)   
		signal(SIGCHLD,SIG_IGN);
	else
		signal(SIGCHLD,SIG_DFL);
	
	//管道解析
	int i; 
	int fd;
	int fds[2];
	for (i = 0; i < cmd_count; ++i)
	{
		// 如果不是最后一条命令，则需要创建管道
		if (i < cmd_count - 1)
		{
			pipe(fds);  //创建管道
			cmd[i].outfd = fds[1];
			cmd[i+1].infd = fds[0];
		}
		//执行命令
		forkexec(i);
		
		if ((fd = cmd[i].infd) != 0)
			close(fd);
		
		if ((fd = cmd[i].outfd) != 1)
			close(fd);
	}
	
	// &后台作业的解析
	if (backgnd == 0)  //backgnd == 0表示前台作业
	{
		//前台作业，需要等待管道中最后一个子进程的退出
		while(wait(NULL) != lastpid)
			;  
	}
	return 0;
}


void forkexec(int i)
{
	pid_t pid;
	pid = fork();
	if (pid == -1)
		ERR_EXIT("fork");
	if (pid > 0)
	{
		//父进程 
		if (backgnd == 1)
			printf("%d\n",pid);   //打印后台进程号
		lastpid = pid;
	}
	else if (pid == 0)
	{
		//backgnd = 1时，表后台作业，将第一条简单命令的infd重定向至/dev/null
		//当第一条命令试图从标准输入获取数据时立刻返回EOF
		if (cmd[i].infd == 0 && backgnd == 1)
			cmd[i].infd = open("/dev/null",O_RDONLY);
		
		//将第一个简单进程作为进程组组长
		if (i == 0)
			setpgid(0,0);
		
		//子进程
		if (cmd[i].infd != 0)  //表示从管道读端输入，而不是标准输入
		{
			close(0);
			// dup() uses the lowest-numbered unused descriptor for the new descriptor
			dup(cmd[i].infd);  //进行文件描述符的复制,0指向管道的读端
		}
		if (cmd[i].outfd != 1)  //表示从管道写端输出，而不是标准输出
		{
			close(1);
			dup(cmd[i].outfd);  //进行文件描述符的复制,1指向管道的写端
		}
		
		//将3以上的文件描述符关闭
		int j;
		for(j = 3; j < 1025; ++j)
		{
			close(j);
		}
		
		//前台作业，能够接收SIGINT,SIGQUIT信号
		//这两个信号恢复成默认操作
		if (backgnd == 0)
		{
			signal(SIGINT,SIG_DFL);  
			signal(SIGQUIT,SIG_DFL);
		}
		execvp(cmd[i].args[0],cmd[i].args);
		//替换失败
		exit(EXIT_FAILURE);
	}
	
}
