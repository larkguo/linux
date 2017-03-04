

/* 
 socket pipe --by larkguo@gmail.com

1.Architecture:
	fds[0] <==socket pipe==> fds[1]

2.Compile:
	gcc pipe.c -o pipe

3.Run:
	# ./pipe 
	connect(39404)
	pipe fds[0]=5 fds[1]=4
	fds[0] --> hello fds[1]!
	fds[1] <-- hello fds[1]!
	fds[1] --> hello fds[0]!
	fds[0] <-- hello fds[0]!
    
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX_BUF_LEN (1024)

int pipe_create(int *fds) 
{ 
	struct sockaddr_in parent_addr,current_addr; 
	int ret = 0, parent_fd = -1, addr_len = 0; 

	/* parent_fd  */
	parent_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) ;
	if(parent_fd < 0) return (-1);
	
	memset((void *) &parent_addr, 0, sizeof(parent_addr)); 
	parent_addr.sin_family = AF_INET; 
	parent_addr.sin_port = htons(0); 
	parent_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); 
	ret = bind(parent_fd,(struct sockaddr *)&parent_addr,sizeof(parent_addr)) ;
	if(ret < 0){
		close(parent_fd) ;
		return (-2);
	}
	ret = listen(parent_fd, 1) ;
	if(ret < 0){
		close(parent_fd) ;
		return (-3);
	}

	/* client fds[1] */
	fds[1] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(fds[1] < 0){
		close(parent_fd) ;
		return (-4);
	}
	memset((void *)&current_addr,0,sizeof(current_addr)); 
	addr_len = sizeof(current_addr);
	ret = getsockname(parent_fd,(struct sockaddr *)&current_addr,&addr_len);
	if(ret < 0){
		close(parent_fd) ;
		return (-5);
	}
	ret = connect(fds[1],(struct sockaddr *)&current_addr,sizeof(current_addr)) ;
	if(ret < 0){
		close(parent_fd) ;
		return (-6);
	}
	printf("connect(%d)\n",ntohs(((struct sockaddr_in*)&current_addr)->sin_port));

	/* fds[0] inherit from parent_fd */
	addr_len = sizeof(parent_addr);
	fds[0] = accept(parent_fd, (struct sockaddr *)&parent_addr,&addr_len);
	if(fds[0] < 0){
		close(parent_fd) ;
		close(fds[1]);
		fds[1] = -1;
		return (-7);
	}

	close(parent_fd); 
	return (0); 
} 

int  main(int argc, char** argv)
{
	int fds[2] = {-1,-1};
	int ret = -1; 
	char from0[MAX_BUF_LEN] = "hello fds[1]!";
	char from1[MAX_BUF_LEN] = "hello fds[0]!";
	char recv0[MAX_BUF_LEN]={0};
	char recv1[MAX_BUF_LEN]={0};

	ret = pipe_create(fds);
	if( ret < 0 ) {
		printf("pipe_create failed %d\n",ret);
		return ret;
	}

	printf("pipe fds[0]=%d fds[1]=%d\n",fds[0], fds[1]);

	/* fds[0] -> fds[1] */
	ret =send(fds[0], from0, sizeof(from0)-1,0) ;
	printf("fds[0] --> %s\n",from0);
	ret = recv(fds[1], (char *)&recv0, sizeof(recv0)-1,0) ;
	printf("fds[1] <-- %s\n",recv0);

	/* fds[1] -> fds[0] */
	ret =send(fds[1], from1, sizeof(from1)-1,0) ;
	printf("fds[1] --> %s\n",from1);
	ret = recv(fds[0], (char *)&recv1, sizeof(recv1)-1,0) ;
	printf("fds[0] <-- %s\n",recv1);

	close(fds[0]);
	close(fds[1]);
	return 0;
}

