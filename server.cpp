/*
	Server: Raj Chaudhari & Segev Apter
	Client: Nikhil Malani & Will Stull 
    CLI interface: Jeffrey Murray


	Professor: Jeffrey Hurley
	ECE-4122 Advanced Programming Techniques
	Date: November 13, 2020
*/

#include "UDP_Socket.h"

/* Size of the buffer used to send the file
 * in several blocks
 */
#define BUF_SIZE 512

struct sockaddr_in sock_serv,clt;

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
    if(bind(sockt, (struct sockaddr *)&sock_serv, sizeof(struct sockaddr_in)) < 0)
    {
        std::cout << "ERROR on binding" << std::endl;
        return 1;
    }
    
    return sockt;
}//end Create_server_socket function



void UDP_Socket::processPacket(const udpPacket& inPkt)
{
    if(inPkt.size() == 0)
    {
        inPkt.push_front(packet);   
    }
    else
    {
        for (auto& pck : m_packetlst)
        {
            if (pck.lSeqNum > inPkt.lSeqNum)
            {
                std::advance(it, i);
                m_packetlst.insert(it, packet);
                break;
            }
            i++;
            if (i >= m_packetlst.size())
            {
                m_packetlst.push_back(packet);
                break;
            }
        }
    }

}



int main (int argc, char** argv)
{
    int file; 
    int sockt;
    udpPacket packet;

    char buf[BUF_SIZE];
    off_t count = 0, n; // long type
    char filename[256];
    unsigned int l = sizeof(struct sockaddr_in);
    
    if (argc != 2)
    {
        std::cout << "Error usage : " << argv[0] << " " << "<port_serv>" << std::endl;
        return 1;
    }
    
    sockt = UDPSocket(atoi(argv[1]));	// set up socket file descriptor

    bzero(filename, 256);	// clear filename
    n = recvfrom(sockt, &filename, 256, 0, (struct sockaddr *)&clt, &l);	// receive filename

    std::cout << "Creating the output file : " << filename << std::endl;

    //file opening
    if((file = open(filename,O_CREAT | O_WRONLY | O_TRUNC, 0600)) < 0)
    {
        std::cout << "fail to open the file" << std::endl;
        return 1;
    }
    
    // preparation of the shipment
    bzero(&buf, BUF_SIZE);
	
    //n = recvfrom(sockt, &packet->buf, BUF_SIZE, 0, (struct sockaddr *)&clt, &l);	// receive first # of bytes
    n = recvfrom(sockt, (char*)&packet, BUF_SIZE, 0, (struct sockaddr*)&clt, &l);	// receive first # of bytes
    processPacket(packet);
    
    // receive remaining bytes
    while(n)
    {
        std::cout << n << " of data received" << std::endl; 

        if (n < 0)
        {
            std::cout << "Unable to read file "<< std::endl;
            return 1;
        }

        count += n;
        write(file, buf, n);
        bzero(buf, BUF_SIZE);
        n = recvfrom(sockt, &buf, BUF_SIZE, 0, (struct sockaddr *)&clt, &l);
    }

}