/*
Author: Will Stull
Class: ECE4122
Last Date Modified: 11/17/2020
Description:
Main source file for running server and client.
*/
#include <iostream>
#include <vector>
#include <sstream>
#include "ECE_UDPSocket.h"

using namespace std;

// g++ -pthread main.cpp ECE_UDPSocket.cpp -o Lab5.out
// Server:
//   Lab5.out 65112
// Client:
//   Lab5.out localhost 65112

/**
 * Terminates the application and closes socket.
 *
 * @param activeSocket socket to be closed
**/
void terminateApplication(ECE_UDPSocket& activeSocket)
{
	cout << "Terminating application." << endl;
    activeSocket.sockClose();
}

/**
 * Server application
 *
 * @param activeSocket server socket
**/
void doServerInteractions(ECE_UDPSocket &activeSocket)
{
    bool bStop = false;
    int nCommand{ 0 };
    do
    {
        // Handle the interactions with the user
        cout << "Please enter a command: ";
        cin >> nCommand;

        switch (nCommand)
        {
/*         case 0: // the server immediately sends to all clients the current 
                // composite message and clears out the composite message
            activeSocket.sendCompositeMsg();
			activeSocket.clearCompositeMsg();
            break;
        case 1: // the server immediately clears out the composite message.
            activeSocket.clearCompositeMsg();
            break;
        case 2: // the server immediately displays to the console the 
                // composite message but takes no other actions.
            activeSocket.displayCompositeMsg();
            break; */
        case 3:	// the server application terminates.
            terminateApplication(activeSocket);
            bStop = true;
            break;
        }
    } while (!bStop);

    exit(0);
}

/**
 * Client application
 *
 * @param activeSocket client socket
 * @param ipAddress server IP address
 * @param portNum server port number
**/
void doClientInteractions(ECE_UDPSocket& activeSocket, string ipAddress, unsigned short portNum)
{
    bool bStop = false;
    string inStr, path;
    int nCommand{ 0 };
    udpMessage outMsg;
	cout << "Please enter a command: ";
    do
    {
        // Handle the interactions with the user
        std::getline(std::cin, inStr);
        // Parse the inStr
        vector<string> strings;
        istringstream f(inStr);
        string s;
        while (getline(f, s, ' ')) {
            strings.push_back(s);
        }

        if (strings.size() == 1 && strings[0].at(0) == 'q')
        {
            terminateApplication(activeSocket);
			bStop = true;
        }
		else if (strings.size() > 1 && strings[0] == "put")
		{
            //may need to fix
			activeSocket.sendFile(ipAddress, portNum, strings[1]);
		}
		else
		{
			cout << "Please enter a command: ";
		}
		cout << flush;
    } while (!bStop);

    exit(0);
}

/**
 * Main application - interprets command line arguments, sets socket up for receiving and listening.
**/
int main(int argc, char** argv)
{
    bool bIsServer = false;
    std::string strLine;
    unsigned short usPortNum { 0 };

    // Get the command line parameters to determine if this is server or a client
    if (argc == 2)
    {
        // This is a server
		cout << "Running server" << endl;
        bIsServer = true;
        usPortNum = atoi(argv[1]);
    }
    else if (argc == 3)
    {
        // This is a client
		cout << "Running client" << endl;
        bIsServer = false;
    }
    else
    {
        // This is a mistake
        cout << "You entered a incorrect number of command line arguments" << endl;
        cout << "For a server enter just the port number!" << endl;
        cout << "For a client enter the IP Address and the port number!" << endl;
    }

    // Setup socket
    ECE_UDPSocket udpSocket(usPortNum);

    if (bIsServer)
    {
        doServerInteractions(udpSocket);
    }
    else
    {
        doClientInteractions(udpSocket, argv[1], atoi(argv[2]));
    }

    return 0;
}