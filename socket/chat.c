/*
chat server  -- by larkguo@gmail.com

1.Compile:
	gcc 	chat.c 	-o 	chat

2.Run:
	./chat	8000
	./chat	127.0.0.1		8000
	./chat	::1		8000
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAX_BUF_LEN  (1024)

void *get_in_addr(struct sockaddr *sa);
unsigned int get_in_port(struct sockaddr *sa);
int tcp_accept(int listenfd);
int tcp_listen(const char *host, const char *serv, socklen_t *addrlenp);


void *
get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

unsigned int 
get_in_port(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return ((struct sockaddr_in*)sa)->sin_port;
	}
	return ((struct sockaddr_in6*)sa)->sin6_port;
}

int 
tcp_accept(int listenfd)
{
	int newfd = -1;    
	socklen_t addrlen;
	struct sockaddr_storage clientaddr; 
	char clientip[INET6_ADDRSTRLEN]={0};
	
	addrlen = sizeof(clientaddr);
	newfd = accept(listenfd,(struct sockaddr *)&clientaddr,&addrlen);
	if (newfd == -1) {
		printf("accept error\n");
	} else {
		printf("socket[%d] <-- connect from %s:%d \n",
			newfd,
			inet_ntop(clientaddr.ss_family,get_in_addr((struct sockaddr*)&clientaddr),clientip,sizeof(clientip)),
			ntohs(get_in_port((struct sockaddr*)&clientaddr)));
	}
	return newfd;
}

int 
tcp_listen(const char *host, const char *serv, socklen_t *addrlenp)
{
	int				listenfd = -1, n;
	const int			on = 1;
	struct addrinfo	hints, *res, *ressave;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ( (n = getaddrinfo(host, serv, &hints, &res)) != 0)
		printf("tcp_listen error for %s, %s: %s\n",host, serv, gai_strerror(n));
	ressave = res;

	do {
		listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (listenfd < 0)
			continue;	/* error, try next one */

		setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
		if (bind(listenfd, res->ai_addr, res->ai_addrlen) == 0)
			break;		/* success */

		close(listenfd);	/* bind error, close and try next one */
	} while ( (res = res->ai_next) != NULL);

	if (res == NULL)	/* errno from final socket() or bind() */
		printf("tcp_listen error for %s, %s\n", host, serv);

	listen(listenfd, 10);

	if (addrlenp)
		*addrlenp = res->ai_addrlen; /* return size of protocol address */
	freeaddrinfo(ressave);

	return(listenfd);
}

int 
main(int argc,char *argv[])
{
	fd_set all_fds;  
	fd_set event_fds; 
	int maxfd;    
	int listenfd;   
	int clientfd;       
	socklen_t addrlen;
	int i, j;
	int nevents;

	if(argc == 2 ){
		listenfd = tcp_listen(NULL, argv[1], &addrlen);
	}
	else if(argc == 3 ){
		listenfd = tcp_listen(argv[1], argv[2], &addrlen);
	}
	else{
		printf("usage: chat  [<listen IPaddress,default:NULL>]  <listen port#,default:8000>\n");
		listenfd = tcp_listen(NULL, 8000, &addrlen);
		//return -1;
	}
	if( listenfd <= 0 ){
		printf("listen error!\n");
		return -1;
	}

	FD_ZERO(&all_fds); 
	FD_ZERO(&event_fds);
	FD_SET(listenfd, &all_fds);
	maxfd = listenfd; 
	
	while(1) {
		event_fds = all_fds; 
		nevents = select(maxfd+1, &event_fds, NULL, NULL, NULL);
		if ( nevents <= 0) {
			continue;
		}
	
		for(i = 0; i <= maxfd; i++) {
			if (FD_ISSET(i, &event_fds)) {
				if (i == listenfd) { // new client
					clientfd = tcp_accept(listenfd);
					if (clientfd > -1) {
						FD_SET(clientfd, &all_fds); 
						if (clientfd > maxfd) maxfd = clientfd;
					}
				} else { // recv data and send to others
					char buf[MAX_BUF_LEN]={0}; 
					int rlen =  recv(i, buf, sizeof(buf), 0);
					if (rlen <= 0) {
						printf("socket[%d] %s\n",i,(rlen==0)?"closed":"recv error");
						close(i); 
						FD_CLR(i, &all_fds); 
					} else { // send to others
						printf("socket[%d] <-- %s\n", i, buf);
						for(j = 0; j <= maxfd; j++) {
							if (FD_ISSET(j, &all_fds)) {
								if (j != listenfd && j != i) {
									int slen = send(j, buf, rlen, 0) ;
									if(slen > 0) printf("socket[%d] --> %s\n",j,buf);
								}
							}
						}
					}
				}
			} 
		}
	}
	
	return 0;
}

