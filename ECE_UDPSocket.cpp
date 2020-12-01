/*
Author: Will Stull
Class: ECE4122
Last Date Modified: 11/17/2020
Description:
Source file for ECE_UDPSocket class.
*/
#include "ECE_UDPSocket.h"

static int file;
static char filename[256];

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
}

/**
 * Function called by thread for receiving messages.
 *
 * @param pUDpSocket active socket
**/
void receiveSocketMsgs(ECE_UDPSocket* pUDpSocket)
{
    // Loop that waits on incoming messages
    udpMessage inMsg;

    sockaddr_in from;
    socklen_t fromlen{ sizeof(struct sockaddr_in) };
    int n;

    do
    {
        n = recvfrom(pUDpSocket->m_sockfd, (char*)&inMsg, sizeof(udpMessage), 0, (struct sockaddr*)&from, &fromlen);
        if (n < 0)
        {
			pUDpSocket->error("ERROR reading from socket");
            break;
        }
        else
        {
            pUDpSocket->processMessage(inMsg);
            pUDpSocket->addSource(from);
        }
    } while (true);
}

/**
 * Class constructor
 *
 * Initializes socket and binds it to specified port.
 * Starts a std::thread to receive messages.
 *
 * @param usPort port that the socket is binded to
**/
ECE_UDPSocket::ECE_UDPSocket(unsigned short usPort) :m_usPort(usPort) 
{
    sockaddr_in serv_addr;
    sockInit();
    // Create the socket: domain IPv4, type-UDP, protocl-IP
    m_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    // Make sure the socket was created
    if (m_sockfd < 0)
        error("ERROR opening socket");
    // Zero out the variable serv_addr
    memset((char*)&serv_addr, 0, sizeof(serv_addr));

    // Initialize the serv_addr
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    // Convert port number from host to network
    serv_addr.sin_port = htons(usPort);
    // Bind the socket to the port number
    if (bind(m_sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        error("ERROR on binding");
    }
    // Start thread that waits for messages 
    m_recvThread = std::thread(receiveSocketMsgs, this);
};

/**
 * Class destructor - closes socket and terminates thread.
**/
ECE_UDPSocket::~ECE_UDPSocket()
{
    if (m_sockfd > 0)
	{
        sockClose();
		sockQuit();
		m_recvThread.join();
	}
	
	return;
}

/**
 * Determines what to do with incoming messages based on nType.
 * 
 * @param inMsg reference to incoming message.
**/
void ECE_UDPSocket::processMessage(const udpMessage& inMsg)
{
    int i = 0;
    udpMessage tmpMsg;
    std::list<udpMessage>::iterator it;
    std::ofstream outFile;
    // Determine what to do with the message
	switch (inMsg.nType)
	{
        case 0: // Indicates filename
            std::cout << "Received filename : " << inMsg.chMsg << std::endl;
            strcpy(filename, inMsg.chMsg);
            break;
        case 1: // Indicates file data
            std::cout << "Receiving data" << std::endl;
            m_mutex.lock(); // lock socket mutex
                it = m_lstMsgs.begin();
                // Check if list is empty. If so, insert at front
                if (m_lstMsgs.empty())
                    m_lstMsgs.push_back(inMsg);
                else
                {
                    for (auto& msg : m_lstMsgs)
                    {
                        if (msg.lSeqNum > inMsg.lSeqNum)
                        {
                            std::advance(it, i);
                            m_lstMsgs.insert(it, inMsg);
                            break;
                        }
                        i++;
                        if (i >= m_lstMsgs.size())
                        {
                            m_lstMsgs.push_back(inMsg);
                            break;
                        }
                    }
                }
                m_mutex.unlock();
            break;
        case 2: // Indicates EOF data
            std::cout << "Receiving EOF data" << std::endl;
            outFile.open(filename); // open file
            m_mutex.lock();
                // Iterate through list and write data to file
                it = m_lstMsgs.begin();
                for (auto& msg : m_lstMsgs)
                {
                    outFile << msg.chMsg;
                }
                outFile << inMsg.chMsg; // write last message received
                std::cout << "Finished writing" << std::endl;
            m_mutex.unlock();
            outFile.close();
            break;
	}
}

/**
 * Determines what to do with incoming messages based on nType.
 * 
 * @param inMsg reference to incoming message.
**/
void ECE_UDPSocket::addSource(const sockaddr_in& from)
{
    bool bIsPresent = false;
	m_mutex.lock();	// lock socket mutex
    // Iterate through list check is already present
	for (auto& source : m_lstSources)
	{
		if ((source.sin_addr.s_addr == from.sin_addr.s_addr) && (source.sin_port == from.sin_port))
		{
			bIsPresent = true;
			break;
		}
	}
    if (!bIsPresent)
    {
		m_lstSources.push_back(from);	// add source to list
    }
	
	m_mutex.unlock();	// unlock socket
}

/**
 * Gets next message in m_lstMsgs.
 *
 * @param msg reference variable for next message
*/
bool ECE_UDPSocket::getNextMessage(udpMessage& msg)
{
    m_mutex.lock();	// lock socket mutex
	if (!m_lstMsgs.empty())
	{
		// Get next message from list
		std::list<udpMessage>::iterator it = m_lstMsgs.begin();
		msg = *it;
		m_lstMsgs.pop_front();
		m_mutex.unlock();	// unlock socket mutex
		return true;
	}
	
	m_mutex.unlock();	// unlock socket mutex
	return false;	// return false if there are no more messages
};

/**
 * Sends a udpMessage to the specified port.
 *
 * @param strTo
 * @param usPortNum
 * @param msg
*/
void ECE_UDPSocket::sendMessage(const std::string& strTo, unsigned short usPortNum, const udpMessage& msg) 
{
    struct hostent* server;
    struct sockaddr_in serv_addr;
    socklen_t fromlen;
    struct sockaddr_in from;

    server = gethostbyname(strTo.c_str());

    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    // Zero out serv_addr variable
    memset((char*)&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;

    memmove((char*)&serv_addr.sin_addr.s_addr, (char*)server->h_addr, server->h_length);

    serv_addr.sin_port = htons(usPortNum);

    fromlen = sizeof(from);

    int n = sendto(m_sockfd, (char*)&msg, sizeof(udpMessage), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    if (n < 0)
        error("ERROR writing to socket");

};

/**
 * Sends a udpMessages containing file data to the specified port.
 *
 * @param strTo
 * @param usPortNum
 * @param path
*/
void ECE_UDPSocket::sendFile(const std::string& strTo, unsigned short usPortNum, const std::string& path) 
{
    int l = sizeof(struct sockaddr_in), seqCount = 0, fd;
    long sz;
    long int n;
    struct stat buffer;
	udpMessage outMsg;

    // open file
	if ((fd = open(path.c_str(), O_RDONLY)) == -1)
        error("open fail");
    
    if (stat(path.c_str(), &buffer) == -1)
        error("stat fail");
    else
        sz = buffer.st_size;
	
    // send filename
    outMsg.nType = 0;   // filename is a udpMessage of nType 0
    strcpy(outMsg.chMsg, getFileName(path).c_str());
    sendMessage(strTo, usPortNum, outMsg);
    
    memset(outMsg.chMsg, 0, sizeof(outMsg.chMsg));  // clear msg buffer
    n = read(fd, outMsg.chMsg, BUF_SIZE);   // first BUF_SIZE bytes of file
    outMsg.nType = 1;   // file data is a udpMessage of nType 1
    
    // read and send remaining bytes of file
    while (n)
    {
        if (n < 0)
            error("ERROR writing to socket");
        
        outMsg.lSeqNum = seqCount++;
        sendMessage(strTo, usPortNum, outMsg);
        
        memset(outMsg.chMsg, 0, sizeof(outMsg.chMsg));  // clear msg buffer
        n = read(fd, outMsg.chMsg, BUF_SIZE);
    }
    
    outMsg.lSeqNum = seqCount++;    // last sequence
    outMsg.nType = 2;   // EOF data is a udpMessage of nType 2
    sendMessage(strTo, usPortNum, outMsg);   // send remaining bytes
    
    std::cout << getFileName(path) << " was transferred to " << strTo << std::endl;
    std::cout << close(fd) << std::endl;

};

/////////////////////////////////////////////////
// Cross-platform socket initialize
int ECE_UDPSocket::sockInit(void)
{
#ifdef _WIN32
    WSADATA wsa_data;
    return WSAStartup(MAKEWORD(1, 1), &wsa_data);
#else
    return 0;
#endif
}
/////////////////////////////////////////////////
// Cross-platform socket quit
int ECE_UDPSocket::sockQuit(void)
{
#ifdef _WIN32
    return WSACleanup();
#else
    return 0;
#endif
}
/////////////////////////////////////////////////
// Cross-platform socket close
int ECE_UDPSocket::sockClose()
{

    int status = 0;

#ifdef _WIN32
    status = shutdown(m_sockfd, SD_BOTH);
    if (status == 0)
    {
        status = closesocket(m_sockfd);
    }
#else
    status = shutdown(m_sockfd, SHUT_RDWR);
    if (status == 0)
    {
        status = close(m_sockfd);
    }
#endif

    // Set to show socket closed
    m_sockfd = -1;
    // Wait for thread to shut down.
    return status;

}
/////////////////////////////////////////////////
// Output error message and exit
void ECE_UDPSocket::error(const char* msg)
{
    perror(msg);
    // Make sure any open sockets are closed before calling exit
    exit(1);
}