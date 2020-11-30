#include "UDP_Socket.h"

/**
 * Class destructor - closes socket and terminates thread.
**/
UDP_Socket::~UDP_Socket()
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
 * Class constructor
 *
 * Initializes socket and binds it to specified port.
 * Starts a std::thread to receive packets.
 *
 * @param usPort port that the socket is binded to
**/
UDP_Socket::UDP_Socket(unsigned short usPort, char* ipaddr) :m_usPort(usPort)
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

    if (inet_pton(AF_INET, ipaddr, &serv_addr.sin_addr) == 0)
        error("Invalid IP address");

    // Convert port number from host to network
    serv_addr.sin_port = htons(usPort);
    // Bind the socket to the port number
    if (bind(m_sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");


    // Start thread that waits for messages 
    m_recvThread = std::thread(receiveSocketMsgs, this);
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
