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

#define BUFFERT 512


/* Command to generate a test file
 * dd if = / dev / urandom of = file count = 8
 */

int duration (struct timeval *start,struct timeval *stop, struct timeval *delta);
int create_server_socket (int port);

struct sockaddr_in sock_serv,clt;

int main (int argc, char**argv){
    //Descripteur
	int fd, sfd;
    
	char buf[BUFFERT];
	off_t count=0, n; // long type
	char filename[256];
    unsigned int l=sizeof(struct sockaddr_in);
	
    // Variable pour la date
	time_t intps;
	struct tm* tmi;
    
	if (argc != 2){
		printf("Error usage : %s <port_serv>\n",argv[0]);
		return EXIT_FAILURE;
	}
    
    sfd = create_server_socket(atoi(argv[1]));
    
	intps = time(NULL);
	tmi = localtime(&intps);
	bzero(filename,256);
	sprintf(filename,"clt.%d.%d.%d.%d.%d.%d",tmi->tm_mday,tmi->tm_mon+1,1900+tmi->tm_year,tmi->tm_hour,tmi->tm_min,tmi->tm_sec);
	printf("Creating the output file : %s\n",filename);
    
	//file opening
	if((fd=open(filename,O_CREAT|O_WRONLY|O_TRUNC,0600))==-1){
		perror("open fail");
		return EXIT_FAILURE;
	}
    
	//preparation of the shipment
	bzero(&buf,BUFFERT);
    n=recvfrom(sfd,&buf,BUFFERT,0,(struct sockaddr *)&clt,&l);
	while(n){
		printf("%lld of data received \n",n);
		if(n==-1){
			perror("read fails");
			return EXIT_FAILURE;
		}
		count+=n;
		write(fd,buf,n);
		bzero(buf,BUFFERT);
        n=recvfrom(sfd,&buf,BUFFERT,0,(struct sockaddr *)&clt,&l);
	}
    
	printf("Nombre d'octets transférés : %lld \n",count);
    
    close(sfd);
    close(fd);
	return EXIT_SUCCESS;
}

/* Fonction permettant le calcul de la durée de l'envoie */
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
        return EXIT_FAILURE;
	}
    
    //preparation de l'adresse de la socket destination
	l=sizeof(struct sockaddr_in);
	bzero(&sock_serv,l);
	
	sock_serv.sin_family=AF_INET;
	sock_serv.sin_port=htons(port);
	sock_serv.sin_addr.s_addr=htonl(INADDR_ANY);
    
	//Affecter une identité au socket
	//copied from hurley code
	if(bind(sfd,(struct sockaddr*)&sock_serv,l)==-1){
		perror("bind fail");
		return EXIT_FAILURE;
	}
    
    
    return sfd;
}