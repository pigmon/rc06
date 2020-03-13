#pragma once

#include <winsock2.h>
#include <pthread.h>
#include <string>

#include "DGram.h"

#pragma comment(lib, "ws2_32.lib") //╪сть ws2_32.dll

class SocketRecver
{
protected:
	SOCKET m_socket;
	sockaddr_in m_addr;

	char* m_dgram;
	long m_dgram_size;
	std::string m_remote_ip;
	int m_port;

public:
	SocketRecver();
	~SocketRecver();
	
	virtual void SetSocket(const long _dgram_size, const int _port, const std::string& _ip);
	virtual int MainLoop();
	void Close();

	virtual void DataTransform() = 0;
};

class SocketSender
{
private:
	SOCKET m_socket;
	sockaddr_in m_addr;

	std::string m_remote_ip;
	int m_port;

public:
	SocketSender();
	~SocketSender();

	virtual void SetSocket(const int _port, const std::string& _ip);
	virtual int MainLoop(const int _port, const std::string& _ip, const char* _data_to_send, const long _dgram_size);
	virtual int MainLoop(const char* _data_to_send, const long _dgram_size, bool _consern_encrypt = true);
	void Close();
};

class SocketRecv_Vehicle : public SocketRecver
{
public:	
	UplinkDGram m_up_dgram;
	virtual void DataTransform();
};