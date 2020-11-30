/*
	Client: Nikhil Malani & Will Stull 
	Server: Raj Chaudhari & Segev Apter
	CLI interface: Jeffrey Murray


	Professor: Jeffrey Hurley
	ECE-4122 Advanced Programming Techniques
	Date: November 13, 2020
*/

#include <stdio.h>  // input/output operations
#include <stdlib.h> // RNG, memory management, sorting, converting
#include <sys/socket.h> // Internet Protocol family
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>  // c_str()
#include <iostream>
#include "UDP_Socket.h"




struct sockaddr_in sock_serv;

//This function will send the file name without the whole path
using std::string;


string getFileName(const string& s) 
{
    char sep = '/';

    #ifdef _WIN32
        sep = '\\';
    #endif

    size_t i = s.rfind(sep, s.length());
    
    if (i != string::npos)
        return(s.substr(i + 1, s.length() - i));

    return("");
}//end function







//---------------------------UDPSOCKET--------------------------------------------------
/* Function allowing the creation of a socket
 * Returns a file descriptor
 */
int UDPSocket(int port, char* ipaddr)
{
    int l;
	int sockt;
    
	sockt = socket(AF_INET,SOCK_DGRAM, 0);

    // Zero out serv_addr variable
	
	if (sockt < 0)
    {
        std::cout << "socket fail" << std::endl;
        return EXIT_FAILURE;
    }

    //setting up destination socket address
    l = sizeof(struct sockaddr_in);
    bzero(&sock_serv, l);
    sock_serv.sin_family = AF_INET;
    sock_serv.sin_port = htons(port);


    if (inet_pton(AF_INET, ipaddr, &sock_serv.sin_addr) == 0)
    {
        std::cout << "Invalid IP adress" << std::endl;
        return EXIT_FAILURE;
    }
	
    return sockt;
}//end function

//----------------------------------------------------------------------------------





int main(int argc, char**argv)
{
    udpPacket packet; 
    int sockt,fd;
    int seqCount = 0;
    char buf[BUF_SIZE];
    long count = 0, m, sz;
    long int n;
    int l = sizeof(struct sockaddr_in);
    struct stat buffer;
    
    if (argc != 4)
    {
        std::cout << "Error usage : " << argv[0] << " <ip_server> <port_server> <file directory>" << std::endl;
        return EXIT_FAILURE;
    }
    
    //sockt=UDPSocket(atoi(argv[2]), argv[1]);
    UDP_Socket socket(atoi(argv[2]), argv[1]);
    std::string path = argv[3];

    if ((fd = open(argv[3],O_RDONLY)) == -1)
    {
        std::cout<<"open fail"<<std::endl;
        return EXIT_FAILURE;
    }
    
    if (stat(argv[3],&buffer) == -1)
    {
        std::cout << "stat fail" << std::endl;
        return EXIT_FAILURE;
    }
    else
        sz = buffer.st_size;

    // send file name to server
    m = sendto(socket->m_sockfd, getFileName(path).c_str(), 256, 0, (struct sockaddr*)&sock_serv, l);
	
    bzero(&packet->buf,BUF_SIZE);   // clear buffer

    n=read(fd,packet->buf,BUF_SIZE);    // initial read of file
	
    
    int seqCount = 0;

    //---------------------------FILE TRANSFER--------------------------------------------
    // read and send bytes of file
    while (1)
    {
        if (n < 0)
        {
            std::cout<<"read fails"<<std::endl; 
            return EXIT_FAILURE;
        }
		
        packet->lSeqNum = seqCount++;
        //m = sendto(socket->m_sockfd, (char*)&packet, n, 0, (struct sockaddr*)&sock_serv, l);
        m = sendto(socket->m_sockfd, (char*)&packet, sizeof(udpPacket), 0, (struct sockaddr*)&sock_serv, l);
        if (m < 0)
        {
            std::cout << "send error" << std::endl;
            return EXIT_FAILURE;
        }
		
        count += m;
        bzero(&packet->buf, BUF_SIZE);
        n = read(fd, packet->buf, BUF_SIZE);
    }

    //-------------------------------------------------------------------------------------

    //m = sendto(socket->m_sockfd, (char*)&packet, 0, 0, (struct sockaddr*)&sock_serv, l);  // send remaining bytes
    m = sendto(socket->m_sockfd, (char*)&packet, sizeof(udpPacket), 0, (struct sockaddr*)&sock_serv, l);  // send remaining bytes
    std::cout << "The file name is: " << getFileName(path) << std::endl;
    std::cout << "Bytes transferred: " << count << std::endl;

    socket.sockQuit();
    socket.sockClose();
    return EXIT_SUCCESS;
}