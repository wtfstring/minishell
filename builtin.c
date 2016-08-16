#include "builtin.h"
#include "parse.h"
#include "externs.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

typedef void (*CMD_HANDLER)();

typedef struct builtin_cmd
{
	char *name;
	CMD_HANDLER handler;
	
} BUILTIN_CMD;
char tcmd[MAXLINE];

void do_exit(char *tcmd,char *name);
void do_cd(char *tcmd,char *name);
void do_type(char *tcmd,char *name);
void do_echo(char *tcmd,char *name);
void parse_singalcmd(void);

//内置命令列表
char *builtinList[] = {"echo","pwd","jobs","kill","fg","bg","cd","exit","ulimit","umask","history",NULL};

BUILTIN_CMD builtins[] = 
{
	{"exit",do_exit},
	{"cd",do_cd},
	{"type",do_type},
	{"echo",do_echo},
	{NULL,NULL}
};

//内部命令解析函数
//返回1表示为内部命令，返回0表示不是内部命令
int builtin(void)
{
	/*
	if (check("exit"))
		do_exit();
	else if (check("cd"))
		do_cd();
	else
		return 0;
	*/
	memset(tcmd,0,sizeof(tcmd));
	int i = 0;
	int found = 0;
	while(builtins[i].name != NULL)
	{
		if (check(builtins[i].name))
		{
			parse_singalcmd();
			builtins[i].handler(tcmd,builtins[i].name);
			found = 1;
			break;
		}
		++i;	
	}
	return found;
}

void do_exit(char *tcmd,char *name)
{
	printf("%s\n",name);
	exit(EXIT_SUCCESS);
}

void do_cd(char *tcmd,char *name)
{
	char cwd[100];
	if (getcwd(cwd,99) == NULL)
		ERR_EXIT("getcwd");
	printf("当前路径为：%s\n",cwd);
	chdir(tcmd);
	if (getcwd(cwd,99) == NULL)
		ERR_EXIT("getcwd");
	printf("更改后路径为：%s\n",cwd);
    printf("%s\n",name);
}

void do_type(char *tcmd,char *name)
{
	int i;
	int isBuiltinCmd = 0;
	for(i = 0; builtinList[i] != NULL; ++i){
		if (!strcmp(tcmd,builtinList[i]))
		{
			isBuiltinCmd = 1;
			break;
		}		
	}
	if (isBuiltinCmd)
		printf("%s is a builtin cmd\n",tcmd);
	else
		printf("%s is a external cmd\n",tcmd);
}

void do_echo(char *tcmd,char *name)
{
	
	printf("%s\n",tcmd);

}

void parse_singalcmd(void)
{
	
	char *pcmd = cmdline + strlen(cmdline) - 1;
	char *ptcmd = tcmd;
	
	while(*pcmd == '\0' || *pcmd == '\t' || *pcmd == '\n')
		--pcmd;
	while(*pcmd != ' ')
		pcmd--;
		
	pcmd++;
	while(*pcmd != '\n')
		*ptcmd++ = *pcmd++;
	//*ptcmd = '\0';
	
}
