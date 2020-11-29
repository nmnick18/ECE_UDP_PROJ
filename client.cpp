/*
	Client: Nikhil Malani & Will Stull 
	Server: Raj Chaudhari & Segev Apter
	CLI interface: Jeffrey Murray

	Professor: Jeffrey Hurley
	ECE-4122 Advanced Programming Techniques
	Date: November 13, 2020
*/


#include <sys/time.h> //timer for the system

#include <sys/time.h>
#include <time.h>
#include <string>
#include <stdio.h> // input/output operations
#include <stdlib.h> // RNG, memory management, sorting, converting
#include <sys/time.h> //timer for the system
#include <sys/socket.h> //Internet Protocol family
#include <sys/socket.h> //Internet Protocol family 
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include <iostream>
#include <mutex>
#include <thread>


std::mutex m_mutex;
std::thread p_thread;



/* Size of the buffer used to send the file
 * in several blocks
 */

#define BUFFER 512

struct sockaddr_in sock_serv;

//This function will send the file name without the whole path
using std::string;

int time_duration(struct timeval *start,struct timeval *stop,struct timeval *delta)
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


string getFileName(const string& s) 
{
	
	

   char sep = '/';

	#ifdef _WIN32
	sep = '\\';
	#endif

   size_t i = s.rfind(sep, s.length());
   if (i != string::npos) {
      return(s.substr(i+1, s.length() - i));
   }
   return("");

   

}//end function


/* Function allowing the creation of a socket
 * Returns a file descriptor
 */
int UDPSocket (int port, char* ipaddr)
{
	

    int length;
	int sockt;
	sockt = socket(AF_INET,SOCK_DGRAM,0);
	if (sockt < 0)
	{
        std::cout<<"socket fail"<<std::endl;
        return EXIT_FAILURE;
	}

	memset(&sock_serv,0,sizeof(struct sockaddr_in));

	sock_serv.sin_family=AF_INET;

	sock_serv.sin_port=htons(port);
	
    if (inet_pton(AF_INET,ipaddr,&sock_serv.sin_addr)==0)
	{
		std::cout << "Invalid IP adress" << std::endl;
			return EXIT_FAILURE;
	}
    return sockt;

	

}//end function
 


int main (int argc, char**argv)
{
	

	struct timeval start, stop, delta;
    int sockt,fd;
    char buf[BUFFER];
    long count=0, m,sz;
	long int n;
    int l=sizeof(struct sockaddr_in);
	struct stat buffer;
    
	if (argc != 4){
		std::cout <<"Error usage : " << argv[0] << " <ip_server> <port_server> <file directory>" << std::endl;
		return EXIT_FAILURE;
	}
    
    sockt = UDPSocket(atoi(argv[2]), argv[1]);
    
	//std::string filepath = argv[3];
   std::string path = argv[3];
	//std::cout << "file path: " << filepath << std::endl;

	if ((fd = open(argv[3],O_RDONLY))==-1){
		std::cout<<"open fail"<<std::endl;
		return EXIT_FAILURE;
	}
    
	if (stat(argv[3],&buffer)==-1){
		std::cout<< "stat fail"<<std::endl;
		return EXIT_FAILURE;
	}
	else
		sz=buffer.st_size;
    

	memset(&buf,0,BUFFER);
    
	gettimeofday(&start,NULL);


//-------------------sending file name ------------------------
//-------------------------------------------------------------
	
	char filename[BUFFER];
	memset(&filename,0,BUFFER);
	std::string s = getFileName(path); //hi.txt
	strcpy(filename, s.c_str());
	int r = sendto(sockt,filename,s.length(),0,(struct sockaddr*)&sock_serv,l);
	std::cout<<"value of r: "<<r<<std::endl;

//-----------------------------------------------------------

    n=read(fd,buf,BUFFER);

	while(n)
	{


		if(n < 0){
			std::cout<<"read fails"<<std::endl; 
			return EXIT_FAILURE;
		}
		m=sendto(sockt,buf,n,0,(struct sockaddr*)&sock_serv,l);
		if(m < 0)
		{
			std::cout<<"send error"<<std::endl;
			return EXIT_FAILURE;
		}
		count+=m;

	
		memset(buf,0,BUFFER);

        n=read(fd,buf,BUFFER);
	}

	
   
 




	std::cout << "The file name is: " << getFileName(path) << std::endl; //hi.txt



	m = sendto(sockt,buf,0,0,(struct sockaddr*)&sock_serv,l);

	//sendto(sockfd, net_buf, NET_BUF_SIZE,sendrecvflag, (struct sockaddr*)&addr_con,addrlen);


	gettimeofday(&stop,NULL);

	time_duration(&start,&stop,&delta);
    
	
	std::cout << "Bytes transferred: " << count << std::endl;
	std::cout << "Total duration: " << delta.tv_sec << " " << delta.tv_usec << std::endl;

	close(sockt);
	close(fd);

	

	return EXIT_SUCCESS;

	
}
