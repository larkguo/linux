/*
C call linux shell --by larkguo@gmail.com

1.Architecture:
	parent fp --cmd -->child stdin
	parent fp <--result--child stdout

2.Compile:
	gcc  shell.c -o shell

2.Run:
	#./shell
*/

#include <stdio.h>
#include <string.h>

#define MAX_BUF_SIZE  (1024)

static int pipe_shell(const char *cmd, char *retbuf, int retbuflen)
{
	FILE *fp = NULL;
	char *ptr = retbuf;
	int recvlen = 0;

	if( (fp = popen(cmd, "r")) == NULL ){
		error( " popen %s error\n",cmd );
		return -1;
	}

	while( !feof(fp) ){
		*ptr = 0;
		if( fgets( ptr, retbuflen-recvlen, fp ) == NULL ){
			break;
		}
		if(strlen(ptr) == 0 ){
			break;
		}
		ptr += strlen(ptr);
		recvlen = strlen(retbuf);
		if ( recvlen >= retbuflen ) {
			printf("(recvlen:%d >=%d)'%s'\r\n", recvlen,retbuflen,retbuf);
			break;
		}
	}
	pclose(fp);
	return 0;
}

#define CMD1   "arp -n"
#define CMD2   "arp -n|grep 00:50:56:c0:00:08 | awk -F ' '  '{ print $1}' |awk NR==1"

int main()
{	
	char result[MAX_BUF_SIZE]={0};

	printf(" Command1:\n%s\n", CMD1);
	pipe_shell(CMD1, result, sizeof(result)-1);
	printf(" Result1:\n%s\n",result);

	memset(result,0,sizeof(result));
	printf(" Command2:\n%s\n", CMD2);
	pipe_shell(CMD2, result, sizeof(result)-1);
	printf(" Result2:\n%s\n",result);
	return 0;
}

