#include <stdio.h>

#include "SocketUnit.h"
//#include "global.h"
#include "VehicleController.h"
#include "dgram_util.h"

const char* des_key = "T&<9c#A`";
const bool Is_Encrypted = false;

/* ----------------------------------------
 SocketRecver
-------------------------------------------*/
SocketRecver::SocketRecver()
{
	m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_socket == SOCKET_ERROR)
		printf("Create socket reciever failed.\n");
}

SocketRecver::~SocketRecver()
{
	closesocket(m_socket);
}

void SocketRecver::SetSocket(const long _dgram_size, const int _port, const std::string& _ip)
{
	m_dgram_size = _dgram_size;
	m_port = _port;
	m_remote_ip = _ip;

	memset(&m_addr, 0, sizeof(m_addr));
	m_addr.sin_family = AF_INET;
	m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	m_addr.sin_port = htons(_port);

	m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_socket == SOCKET_ERROR)
		printf("Create socket reciever failed.\n");

	if (bind(m_socket, (struct sockaddr *)&m_addr, sizeof(m_addr)) == SOCKET_ERROR)
		printf("bind socket remote control error !");

	int iMode = 1; //0£º×èÈû
	ioctlsocket(m_socket, FIONBIO, (u_long FAR*) &iMode);

	m_dgram = new char[m_dgram_size];
}

int SocketRecver::MainLoop()
{
	memset(m_dgram, 0, m_dgram_size);

	int nAddrLen = sizeof(struct sockaddr);
	bool decrypt_result = false;
	int cnt = 0;
	UplinkDGram dgram;

	if (Is_Encrypted)
	{
		char* encrypted_buff = new char[m_dgram_size]; 		// Use in recvfrom
		memset(encrypted_buff, 0, m_dgram_size);

		cnt = recvfrom(m_socket, encrypted_buff, m_dgram_size, 0, (struct sockaddr *)&m_addr, &nAddrLen);

		if (cnt == m_dgram_size)
		{
			m_dgram = DGramUtil::DecryptBytes(encrypted_buff, des_key, m_dgram_size);
		}
		else
		{
			printf("Socket recieved wrong size (%d bytes) of encrypted data in <socket_receive>.\n", cnt);
		}

		delete[] encrypted_buff;
	}
	else
	{
		cnt = recvfrom(m_socket, m_dgram, m_dgram_size, 0, (struct sockaddr *)&m_addr, &nAddrLen);
	}

	//int cnt = recvfrom(m_socket, m_dgram, m_dgram_size, 0, (struct sockaddr *)&m_addr, &nAddrLen);

	if (cnt > 0 && m_dgram)
	{
		UplinkDGram dgram;
		memcpy(&dgram, m_dgram, m_dgram_size);

		//printf("%d bytes received.\n", cnt);
		//printf("Sizeof dgram is %ld\n", sizeof(m_dgram));
		//dgram.Print();

		DataTransform();
	}
	else
	{
		//printf("Recv Error of %d\n", cnt);
		int ret_err = WSAGetLastError();
		//printf("Receiver Error code is: %d\n", ret_err);
		//printf("Sizeof m_dgram is %ld\n", sizeof(m_dgram));
		//printf("Dgram Size is %d\n", m_dgram_size);

		Sleep(100);
	}

	return cnt;
}

void SocketRecver::Close()
{
	closesocket(m_socket);
}

/* ----------------------------------------
SocketSender
-------------------------------------------*/
SocketSender::SocketSender()
{

}

SocketSender::~SocketSender()
{
	closesocket(m_socket);
}

void SocketSender::SetSocket(const int _port, const std::string& _ip)
{
	m_port = _port;
	m_remote_ip = _ip;

	memset(&m_addr, 0, sizeof(m_addr));
	m_addr.sin_family = AF_INET;
	m_addr.sin_port = htons(m_port);
	m_addr.sin_addr.S_un.S_addr = inet_addr(m_remote_ip.c_str());

	m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_socket == SOCKET_ERROR)
		printf("create socket sender failed\n");
}

int SocketSender::MainLoop(const char* _data_to_send, const long _dgram_size, bool _consern_encrypt)
{
	int cnt = 0;

	if (Is_Encrypted && _consern_encrypt)
	{
		char* bytes_encrypt = DGramUtil::EncryptBytes((char*)_data_to_send, des_key, _dgram_size);

		if (bytes_encrypt)
		{
			cnt = sendto(m_socket, bytes_encrypt, _dgram_size, 0, (struct sockaddr *)&m_addr, sizeof(SOCKADDR));
			delete[] bytes_encrypt;
		}
	}
	else
	{
		cnt = sendto(m_socket, _data_to_send, _dgram_size, 0, (SOCKADDR *)&m_addr, sizeof(SOCKADDR));
	}

	return cnt;
}

int SocketSender::MainLoop(const int _port, const std::string& _ip, const char* _data_to_send, const long _dgram_size)
{
	m_port = _port;
	m_remote_ip = _ip;

	memset(&m_addr, 0, sizeof(m_addr));
	m_addr.sin_family = AF_INET;
	m_addr.sin_port = htons(m_port);
	m_addr.sin_addr.S_un.S_addr = inet_addr(m_remote_ip.c_str());

	m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_socket == SOCKET_ERROR)
		printf("create socket sender failed\n");

	int cnt = 0;

	if (Is_Encrypted)
	{
		char* bytes_encrypt = DGramUtil::EncryptBytes((char*)_data_to_send, des_key, _dgram_size);

		if (bytes_encrypt)
		{
			cnt = sendto(m_socket, bytes_encrypt, _dgram_size, 0, (struct sockaddr *)&m_addr, sizeof(SOCKADDR));
			delete[] bytes_encrypt;
		}
	}
	else
	{
		cnt = sendto(m_socket, _data_to_send, _dgram_size, 0, (SOCKADDR *)&m_addr, sizeof(SOCKADDR));
	}

	//printf("%d bytes sent.\n", cnt);
	//int ret_err = WSAGetLastError();
	//printf("Error code is: %d\n", ret_err);
	
	// test
	//DownlinkDGram dgram;
	//memcpy(&dgram, _data_to_send, _dgram_size);
	//dgram.Print();
	

	//if (cnt > 0)
	//	printf("%d bytes sent.\n", cnt);
	closesocket(m_socket);

	return cnt;
}

void SocketSender::Close()
{
	closesocket(m_socket);
}

/* ----------------------------------------
SocketSender
-------------------------------------------*/

void SocketRecv_Vehicle::DataTransform()
{
	memcpy(&m_up_dgram, m_dgram, m_dgram_size);
	//m_up_dgram.Print();

	VehicleController::TargetAngle = m_up_dgram.m_steering / 100.0f;
}