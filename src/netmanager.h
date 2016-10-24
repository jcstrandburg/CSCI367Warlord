/*=============================
Justin Strandburg
CSCI 367
Fall Quarter 2013

netmanager.h

Specification for NetManager module, which handles creation of management of
sockets and Connection objects
=============================*/

#ifndef NETMANAGER_H
#define NETMANAGER_H

#include <string>
#include "connection.h"

struct NetManagerImpl;
class Connection;
struct SocketWrapper;

class NetManager
{
	public:
		NetManager();
		~NetManager();
	void StartLogging( const std::string &base);
	void Listen( int port, int queueLength);//begin listening on a port and monitoring for connection requests
	void Update();//check all open sockets for input or closure and update the corresponding Connection Object
	bool HasConnectRequest();//returns true if there is input waiting 
	Connection AcceptConnectRequest();//create a Connection object for an incoming connection request and return a handle to it
	Connection RequestConnection( const char *host, int port);//attempt to connect to a given listening host, note that this is blocking
	unsigned int ConnectionCount();//returns the number of open connections
	void PollSockets();
	
	private:
SocketWrapper AllocateSocketWrapper( int sd);
	
		NetManager( NetManager&);//make the class non-copyable
		NetManager& operator = ( NetManager&);//disable assignment operator
NetManagerImpl *my;
};

#endif
