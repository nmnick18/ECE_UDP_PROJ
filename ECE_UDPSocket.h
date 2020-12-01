/*
Author: Will Stull
Class: ECE4122
Last Date Modified: 11/17/2020
Description:
Header file for ECE_UDPSocket class.
*/
#pragma once

#include <cstdio>
#include <thread>
#include <iterator>
#include <iostream>
#include <fstream>
#include <list>
#include <cstring>
#include <string>
#include <mutex>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

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
    //#include <unistd.h> /* Needed for close() */

    typedef int SOCKET;
#endif

#define BUF_SIZE 512

struct udpMessage
{
    unsigned short nType;     // htons()
    unsigned long lSeqNum;    // htonl()
    char chMsg[BUF_SIZE];
};

using std::string;

string getFileName(const string& s);

/**
 * Implementation of ECE_UDPSocket
 *
 * This is a generic socket class that can be used in both
 * server & client applications.
**/
class ECE_UDPSocket
{
public:

	/**
	 * Class destructor - closes socket and terminates thread.
	**/
    ~ECE_UDPSocket();
	
	/**
	 * Class constructor
	 *
	 * Initializes socket and binds it to specified port.
	 * Starts a std::thread to receive messages.
	 *
	 * @param usPort port that the socket is binded to
	**/
    ECE_UDPSocket(unsigned short usPort);

	/**
	 * Gets next message in m_lstMsgs.
	 *
	 * @param msg reference variable for next message
	*/
    bool getNextMessage(udpMessage& msg);

	/**
	 * Sends a udpMessage to the specified port.
	 *
	 * @param strTo
	 * @param usPortNum
	 * @param msg
	*/
    void sendMessage(const std::string& strTo, unsigned short usPortNum, const udpMessage& msg);
	
    /**
	 * Sends a udpMessages containing file data to the specified port.
	 *
	 * @param strTo
	 * @param usPortNum
	 * @param path
	*/
    void sendFile(const std::string& strTo, unsigned short usPortNum, const std::string& path);
    
	
    // Other stuff
	
	/**
	 * Determines what to do with incoming messages based on nType.
	 * 
	 * @param inMsg reference to incoming message.
	**/
    void processMessage(const udpMessage &inMsg);
	
	/**
	 * Determines what to do with incoming messages based on nType.
	 * 
	 * @param inMsg reference to incoming message.
	**/
    void addSource(const sockaddr_in &from); // Add source to the list

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
private:

	// Port number socket is binded to
    unsigned short m_usPort;
	
	// Thread for receiving messages
    std::thread m_recvThread;
	
	// List of messages that make up composite message
    std::list<udpMessage> m_lstMsgs;
	
	// List of source addresses
    std::list<sockaddr_in> m_lstSources;
	
	// Socket mutex
    std::mutex m_mutex;
};

