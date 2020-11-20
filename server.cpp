#include <stdio.h>
#include <stdlib.h>
// Time function, sockets, htons... file stat
#include <sys/time.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>
// File function and bzero
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>


/* Size of the buffer used to send the file
 * in several blocks
 */

#define BUFFER 512

struct sockaddr_in sock_serv,clt;

/* Command to generate a test file
 */
int duration (struct timeval *start,struct timeval *stop,struct timeval *delta)
{
    suseconds_t microstart, microstop, microdelta;
    
    microstart = (suseconds_t) (100000*(start->tv_sec))+ start->tv_usec;
    microstop = (suseconds_t) (100000*(stop->tv_sec))+ stop->tv_usec;
    microdelta = microstop - microstart;
    
    delta->tv_usec = microdelta%100000;
    delta->tv_sec = (time_t)(microdelta/100000);
    
    if((*delta).tv_sec < 0 || (*delta).tv_usec < 0)
        return -1;
    else
        return 0;
}

/* Function allowing the creation of a socket and its attachment to the system
 * Returns a file descriptor in the process descriptor table
 * bind allows its definition with the system
 * Inspired from Professor Huley code
 */
int create_server_socket (int port){
    int l;
	int sfd;
    
	sfd = socket(AF_INET,SOCK_DGRAM,0);
	if (sfd == -1){
        perror("socket fail");
        return 1;
	}
    
	l=sizeof(struct sockaddr_in);
	bzero(&sock_serv,l);
	
	sock_serv.sin_family=AF_INET;
	sock_serv.sin_port=htons(port);
	sock_serv.sin_addr.s_addr=htonl(INADDR_ANY);

	//copied from hurley code
	if(bind(sfd,(struct sockaddr*)&sock_serv,l)==-1){
		perror("bind fail");
		return 1;
	}
    
    return sfd;
}



int main (int argc, char**argv){
	int fd, sfd;
    
	char buf[BUFFER];
	off_t count=0, n; // long type
	char filename[256];
    unsigned int l=sizeof(struct sockaddr_in);
	
    // date variable
	time_t intps;
	struct tm* tmi;
    
	if (argc != 2){
		printf("Error usage : %s <port_serv>\n",argv[0]);
		return 1;
	}
    
    sfd = create_server_socket(atoi(argv[1]));
    
	intps = time(NULL);
	tmi = localtime(&intps);
	bzero(filename,256);
	sprintf(filename,"Transferred.%d.%d.%d",tmi->tm_hour,tmi->tm_min,tmi->tm_sec);
	printf("Creating the output file : %s\n",filename);
    
	//file opening
	if((fd=open(filename,O_CREAT|O_WRONLY|O_TRUNC,0600))==-1){
		perror("open fail");
		return 1;
	}
    
	//preparation of the shipment
	bzero(&buf,BUFFER);
    n=recvfrom(sfd,&buf,BUFFER,0,(struct sockaddr *)&clt,&l);
	while(n){
		printf("%lld of data received \n",n);
		if(n==-1){
			perror("read fails");
			return 1;
		}
		count+=n;
		write(fd,buf,n);
		bzero(buf,BUFFER);
        n=recvfrom(sfd,&buf,BUFFER,0,(struct sockaddr *)&clt,&l);
	}
    
	printf("Number of bytes transferred: %lld \n",count);
    
    close(sfd);
    close(fd);
	return 0;
}

