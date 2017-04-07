
/*
Tun/Tap Server -- by larkguo@gmail.com

1.Architecture:
	Applation(ping)
		|
	Virtual NIC(TCP/IP)
		|
	Real NIC(TCP/IP)
  
2.Compile:
	gcc server.c -o server

3.Run:
	 ./server -i tun132 &
	ifconfig tun132 192.168.8.132
	ip addr add 192.168.8.0/24 dev tun132	

4.Result:
	Init virtual interface tun132 ok!

	<--Real NIC 84
	-->Virtual NIC 84

	<--Virtual NIC 84
	-->Real NIC 84
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h> 
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <stdarg.h>

#define BUFSIZE (2000)  
#define PORT (55555)

int virtual_sock(char *dev, int flags) {
	struct ifreq ifr;
	int fd, ret;
	char *clonedev = "/dev/net/tun";

	if( (fd = open(clonedev , O_RDWR)) < 0 ) {
		printf("opening /dev/net/tun error %d\n",fd);
		return fd;
	}

	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = flags;
	if (*dev) {
		strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	}
	ret = ioctl(fd, TUNSETIFF, (void *)&ifr);
	if(  ret < 0 ) {
		printf("ioctl(TUNSETIFF) error %d\n",ret);
		close(fd);
		return ret;
	}
	strcpy(dev, ifr.ifr_name);

	return fd;
}

void usage(void) {
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, " -i <virtual-interface> [-u|-a] \n");
	fprintf(stderr, " -h\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "-i <ifacename>: name of virtual-interface\n");
	fprintf(stderr, "-u|-a: use TUN (-u, default) or TAP (-a)\n");
	fprintf(stderr, "-h:help\n");
}

int main(int argc, char *argv[]) {

	int virtual_fd = -1, server_fd = -1, real_fd = -1;
	int flags = IFF_TUN;
	char if_name[IFNAMSIZ] = {0};
	int maxfd;
	struct sockaddr_in local, remote;
	unsigned short int port = PORT;
	int  option,optval = 1;
	socklen_t remotelen;
	int ret = 0;
	
	/* Check command line options */
	while((option = getopt(argc, argv, "i:uah")) > 0) {
		switch(option) {
		  case 'h':
			  usage();
			  break;
		  case 'i':
			  strncpy(if_name,optarg, sizeof(if_name)-1);
			  break;
		  case 'u':
			  flags = IFF_TUN;
			  break;
		  case 'a':
			  flags = IFF_TAP;
			  break;
		  default:
			  printf("Unknown option %c\n", option);
			  usage();
			  goto _exit;
		}
	}

	if(*if_name == '\0') {
		printf("Must specify interface name!\n");
		usage();
	} 

	/* initialize tun/tap interface */
	if ( (virtual_fd = virtual_sock(if_name, flags | IFF_NO_PI)) < 0 ) {
		printf("Init tun/tap interface %s failed!\n", if_name);
		goto _exit;
	}

	printf("Init virtual interface %s ok!\n", if_name);

	if ( (server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		goto _exit;
	}

	ret = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval));
	if( ret < 0) {
		goto _exit;
	}

	memset(&local, 0, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	local.sin_port = htons(port);
	ret = bind(server_fd, (struct sockaddr*) &local, sizeof(local));
	if (ret < 0) {
		goto _exit;
	}

	ret = listen(server_fd, 5);
	if ( ret < 0) {
		goto _exit;
	}

	/* wait for connection */
	remotelen = sizeof(remote);
	memset(&remote, 0, remotelen);
	if ((real_fd = accept(server_fd, (struct sockaddr*)&remote, &remotelen)) < 0) {
		goto _exit;
	}

	maxfd = (virtual_fd > real_fd) ? virtual_fd:real_fd;

	while(1) {
		fd_set rd_set;
		uint16_t nread = 0, nwrite = 0;
		char buffer[BUFSIZE] = {0};
		
		FD_ZERO(&rd_set);
		FD_SET(virtual_fd, &rd_set); 
		FD_SET(real_fd, &rd_set);

		ret = select(maxfd + 1, &rd_set, NULL, NULL, NULL);
		if (ret < 0) {
			if ( errno == EINTR){
				continue;
			}else{
				goto _exit;
			}
		}
		if(FD_ISSET(real_fd, &rd_set)) {/* Real NIC */
			nread = read(real_fd, buffer, sizeof(buffer)-1);
			if(nread <= 0) {
				goto _exit;
			}
			printf("\n<--Real NIC %d\n",nread);

			nwrite = write(virtual_fd, buffer, nread);
			printf("-->Virtual NIC %d\n",nwrite);
		}
		if(FD_ISSET(virtual_fd, &rd_set)) { /* Virtual NIC */
			nread = read(virtual_fd, buffer, sizeof(buffer)-1);
			printf("\n<--Virtual NIC %d\n",nread);

			nwrite = write(real_fd, buffer, nread);
			printf("-->Real NIC %d\n",nwrite);
		}
	}

_exit:
	if(server_fd > 0) close(server_fd);
	if(real_fd > 0) close(real_fd);
	if(virtual_fd > 0) close(virtual_fd);
	return(0);
}
