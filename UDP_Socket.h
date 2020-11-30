#pragma once

#include <thread>
#include <iterator>
#include <list>
#include <stdio.h>  // input/output operations
#include <stdlib.h> // RNG, memory management, sorting, converting
#include <sys/socket.h> // for non-windows system
#include <arpa/inet.h>  //Internet Protocol family
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>  // c_str()
#include <string>
#include <iostream>

/* Size of the buffer used to send the file
 * in several blocks
 */
#define BUF_SIZE 512

#ifdef _WIN32
    /* See http://stackoverflow.com/questions/12765743/getaddrinfo-on-win32 */
    #ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0501  /* Windows XP. */
    #endif
    #include <winsock2.h>
    #include <Ws2tcpip.h>

    #pragma comment (lib, "Ws2_32.lib")
#else
    /* Assume that any non-Windows platform uses POSIX-style sockets instead. */
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <netdb.h>  /* Needed for getaddrinfo() and freeaddrinfo() */
    #include <unistd.h> /* Needed for close() */

    typedef int SOCKET;
#endif



struct udpPacket
{
    unsigned short packetLength;   // htons()
    unsigned long lSeqNum;    // htonl()
    char buf[BUF_SIZE];
};

class UDP_Socket
{
public:

	/**
	 * Class destructor - closes socket and terminates thread.
	**/
	~UDP_Socket();

	/**
	 * Class constructor
	 *
	 * Initializes socket and binds it to specified port.
	 * Starts a std::thread to receive packets.
	 *
	 * @param usPort port that the socket is binded to
	**/
	UDP_Socket(unsigned short usPort, char* ipaddr);

	/**
	 * Sends a udpPacket to the specified port.
	 *
	 * @param strTo
	 * @param usPortNum
	 * @param pkt
	*/
	void sendPacket(const std::string& strTo, unsigned short usPortNum, const udpPacket& pkt);

	// Other stuff

	/**
	 * Determines what to do with incoming packets.
	 *
	 * @param inPkt reference to incoming packet.
	**/
	void processPacket(const udpPacket& inPkt);

	// Cross-platform socket initialize
	int sockInit(void);

	// Cross-platform socket quit
	int sockQuit(void);

	// Cross-platform socket close
	int sockClose();

	// Outputs error message and quits
	void error(const char* msg);

	// Socket file descriptor
	int m_sockfd;

    // List of messages that make up composite message
	std::list<udpPacket> m_packetlst;






private:

	// Port number socket is binded to
	unsigned short m_usPort;

	// Thread for receiving messages
	std::thread m_recvThread;

	// List of source addresses
	//std::list<sockaddr_in> m_lstSources;

	// Socket mutex
	std::mutex m_mutex;
};