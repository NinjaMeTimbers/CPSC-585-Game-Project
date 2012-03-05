#pragma once

#include <WinSock.h>
#include <inaddr.h>
//#include <WinSock2.h>

#define MAXCLIENTS 5

//Client headers
const char BUTTON = 'B';
const char READY = 'R';
const char UNREADY = 'U';
const char COLOR = 'C';

//Server headers
const char TRACK = 'T';
const char START = 'S';
const char END = 'E';
const char CLIENTINFO = 'L';
const char WORLDSTATE = 'W';
const char ID = 'I';

struct ClientInfo
{
	int id;
	int color;
	SOCKET sock;
	bool ready;
	SOCKADDR_IN addr;
};