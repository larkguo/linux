/*
IPv4 and IPv6 TCP client  -- by larkguo@gmail.com

1.Compile:
        gcc     client.c        -o      client

2.Run:
        ./client        8000
        ./client        127.0.0.1	8000
        ./client        ::1             8000
*/

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_BUF_LEN (1024 )
#define TIMEOUT_SECONDS (3)

char * sock_ntop_host(const struct sockaddr *sa, socklen_t salen);
unsigned int get_in_port(struct sockaddr *sa);
int tcp_connect(const char *host, const char *serv);


char *
sock_ntop_host(const struct sockaddr *sa, socklen_t salen)
{
    static char str[128] = {0};         /* Unix domain is largest */

        switch (sa->sa_family) {
        case AF_INET: {
                struct sockaddr_in      *sin = (struct sockaddr_in *) sa;

                if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL)
                        return(NULL);
                return(str);
        }
        case AF_INET6: {
                struct sockaddr_in6     *sin6 = (struct sockaddr_in6 *) sa;

                if (inet_ntop(AF_INET6, &sin6->sin6_addr, str, sizeof(str)) == NULL)
                        return(NULL);
                return(str);
        }

        default:
                snprintf(str, sizeof(str),"unknown AF_xxx:%d,len %d",sa->sa_family,salen);
                return(str);
        }
    return (NULL);
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
tcp_connect(const char *host, const char *serv)
{
        int                             sockfd, n;
        struct addrinfo hints, *res, *ressave;
        socklen_t       len;
        struct sockaddr_storage ss;

        bzero(&hints, sizeof(struct addrinfo));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        if ( (n = getaddrinfo(host, serv, &hints, &res)) != 0){
                printf("tcp_connect error for %s, %s: %s",host, serv, gai_strerror(n));
                return -1;
        }
        ressave = res;

        do {
                sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
                if (sockfd < 0)
                        continue;       /* ignore this one */

                if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)
                        break;          /* success */

                close(sockfd);  /* ignore this one */
        } while ( (res = res->ai_next) != NULL);

        if (res == NULL)        /* errno set from final connect() */
                printf("tcp_connect error for %s, %s\n", host, serv);

        freeaddrinfo(ressave);

        len=sizeof(ss);  
        getsockname(sockfd,(struct sockaddr *)&ss,&len);  
        printf("local %s %d\n",sock_ntop_host((struct sockaddr *)&ss,len),
                ntohs(get_in_port((struct sockaddr*)&ss)));

        len = sizeof(ss);
        getpeername(sockfd, (struct sockaddr *)&ss, &len);
        printf("remote %s %d\n",sock_ntop_host((struct sockaddr *)&ss,len),
                ntohs(get_in_port((struct sockaddr*)&ss)));

        return(sockfd);
}

int 
sock_read(int sockfd,char *buf,int buflen,int timeout)
{
        int nevents;
        fd_set readSet;
        struct timeval tv;
        int rlen;

        if (timeout < 0) return -1;

        FD_ZERO(&readSet);
        FD_SET(sockfd, &readSet);
        tv.tv_sec = timeout;
        tv.tv_usec = 0;
        while (1) {
                nevents = select(sockfd + 1, &readSet, NULL,NULL, &tv);
                if (nevents == 0){
                        printf("read timeout %d(s)\n", timeout);
                        return -1;
                }
                else if (nevents == -1) {
                        if (errno == EINTR) continue;
                        return -1;
                } else break;
        }
        rlen = recv(sockfd, buf, buflen, 0);
        return (int)rlen;
}

int 
sock_write(int sockfd,char *buf, int  buflen,int timeout)
{
        int nevents;
        fd_set writeSet;
        struct timeval tv;
        int bytes_sent = 0;
        size_t byte_left = (size_t)0;
        ssize_t slen;

        if (timeout < 0) return -1;

        FD_ZERO(&writeSet);
        FD_SET(sockfd, &writeSet);
        tv.tv_sec = timeout;
        tv.tv_usec = 0;

        while (1) {
                nevents = select(sockfd + 1, NULL, &writeSet,NULL, &tv);
                if (nevents == 0){
                        printf("write timeout %d(s)\n",timeout);
                        return -1;
                }
                else if (nevents == -1) {
                        if (errno == EINTR) continue;
                        return -1;
                } else
                        break;
        }

        byte_left = buflen;
        while (byte_left != (size_t)0) {
                slen = send(sockfd, buf + bytes_sent, byte_left,0);
                if (slen == -1) {
                        return (int)slen;
                }
                byte_left -= (size_t)slen;
                bytes_sent += slen;
        }

        return  bytes_sent;
}

int 
main(int argc, char *argv[])  
{  
        int sockfd = -1;  

        if(argc == 2 ){
                sockfd = tcp_connect(NULL, argv[1]);
        }
        else if(argc == 3 ){
                sockfd = tcp_connect(argv[1], argv[2]);
        }
        else{
                printf("usage: client [<hostname or IPaddress>] <service or port#>\n");
                return -1;
        }

        while (1)  { 
                char buf[MAX_BUF_LEN]={0};  
                int rlen = 0,slen = 0;
                rlen = read(STDIN_FILENO, buf, sizeof(buf)-1);  
                if (rlen > 0)  
                {  
                        slen = sock_write(sockfd, buf, rlen, TIMEOUT_SECONDS);  
                        if (slen <= 0)  {  
                                goto out;
                        } 
                        rlen = sock_read(sockfd, buf, rlen, TIMEOUT_SECONDS);  
                        if( rlen > 0 ){
                                printf("recv: %s\n",buf);
                        }
                }  
        }  

out:
        close(sockfd);  
        return 0;  
} 