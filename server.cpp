

/*
	Server: Raj Chaudhari & Segev Apter
	Client: Nikhil Malani & Will Stull 
    CLI interface: Jeffrey Murray


	Professor: Jeffrey Hurley
	ECE-4122 Advanced Programming Techniques
	Date: November 13, 2020


*/



#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include <sys/socket.h> //for non-windows system

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <iostream>

/* Size of the buffer used to send the file
 * in several blocks
 */

#define BUFFER 512

struct sockaddr_in sock_serv,clt;

/* Command to generate a test file
 */
int duration (struct timeval *start,struct timeval *stop,struct timeval *delta)
{
    suseconds_t time_start, time_end, time;
    
    time_start = (suseconds_t) (100000*(start->tv_sec))+ start->tv_usec;

    time_end = (suseconds_t) (100000*(stop->tv_sec))+ stop->tv_usec;

    time = time_end - time_start;
    
    delta->tv_usec = time%100000;
    delta->tv_sec = (time_t)(time/100000);
    
    if((*delta).tv_sec < 0 || (*delta).tv_usec < 0)
        return -1;
    else
        return 0;


}//end function



/* Function allowing the creation of a socket and its attachment to the system
 * Returns a file descriptor in the process descriptor table
 * bind allows its definition with the system
 * Inspired from Professor Huley code
 */
int UDPSocket(int usPort)
{
 

	int sockt;

	//creating a socket sockt
	sockt = socket(AF_INET,SOCK_DGRAM, 0);

	if (sockt < 0)
	{
        std::cout << "ERROR opening socket" << std::endl; 
        return 1;
	}


	//zero out the server address variable
    //bzero(&sock_serv,sizeof(struct sockaddr_in));
	
	//zero out the server address variable
    memset(&sock_serv, 0, sizeof(struct sockaddr_in));
	
	//initialize the server address
	sock_serv.sin_family = AF_INET;
	//convert portnumber from host to network
	sock_serv.sin_port = htons(usPort);

	sock_serv.sin_addr.s_addr = htonl(INADDR_ANY);


	//bind the socket to the portnumber
	if(bind(sockt,(struct sockaddr*)&sock_serv,sizeof(struct sockaddr_in)) < 0)
	{
		std::cout<<"ERROR on binding"<<std::endl;
		return 1;
	}
    
    return sockt;

}//end Create_server_socket function



int main (int argc, char** argv)
{
	int file; 
	
	int sockt;
    
	char buf[BUFFER];

	off_t count=0, n; // long type

	char filename[256];

    unsigned int l=sizeof(struct sockaddr_in);
	
    // date variable
	time_t intps;
	struct tm* tmi;
    
	if (argc != 2)
	{
		std::cout << "Error usage : " << argv[0] << " " << "<port_serv>" << std::endl;
		return 1;
	}
    
    sockt = UDPSocket(atoi(argv[1]));
    
	intps = time(NULL);
	tmi = localtime(&intps);

	bzero(filename, 256);	// clear filename
	n=recvfrom(sockt, &filename, 256, 0, (struct sockaddr *)&clt, &l);	// receive filename

	printf("Transferred.%d.%d.%d\n", tmi->tm_hour, tmi->tm_min, tmi->tm_sec);
	std::cout << "Creating the output file : " << filename << std::endl;

	//file opening
	if((file=open(filename,O_CREAT|O_WRONLY|O_TRUNC,0600)) < 0)
	{
		std::cout << "fail to open the file" << std::endl;
		return 1;
	}
    
	// preparation of the shipment
	bzero(&buf,BUFFER);
	
    n=recvfrom(sockt, &buf, BUFFER, 0, (struct sockaddr *)&clt, &l);	// receive first # of bytes

	// receive remaining bytes
	while(n)
	{
		std::cout << n << " of data received" << std::endl; 

		if (n < 0)
		{
			std::cout << "Unable to read file "<< std::endl;
		
			return 1;
		}

		count+=n;
		write(file,buf,n);
		bzero(buf,BUFFER);
        n=recvfrom(sockt,&buf,BUFFER,0,(struct sockaddr *)&clt,&l);
	}

}
    
