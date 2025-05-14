#pragma once
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <iostream>
#include <cstring>
#include <vector>
#include <thread>
#include <atomic>
	#ifdef _WIN32 
	#define _WINSOCK_DEPRECATED_NO_WARNINGS 
	#include <winsock2.h>
	#else 
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <unistd.h>
	#endif 
#include <random>
#include "WaitingHall.h"
#include "AppDelegate.h"

const uint16_t BROADCAST_PORT = 21567;
const int BUFFER_SIZE = 64;

// ��Ϣ����ö�� 
enum class MessageType : uint8_t 
{
    ServerBroadcast,  // �������㲥��Ӧ 
    ClientJoin,       // �ͻ��˼������� 
    PlayerMove,       // ����ƶ� 
    PlayerAttack,     // ��ҹ��� 
    GameSync          // ��Ϸ״̬ͬ�� 
};

#pragma pack(push, 1)
// ���ݰ��ṹ
struct GameMessage
{
	uint16_t checksum;          // У���
	unsigned int serialNumber;  // ���к�
	MessageType type;           // ��Ϣ����
	char payload[BUFFER_SIZE - sizeof(MessageType) - sizeof(uint16_t) - sizeof(unsigned int)];

	bool Validate() const;
	void UpdateChecksum();

private:
	uint16_t CalculateChecksum() const;
};

// ������Ϣ�ṹ 
//�������㲥��Ϣ
struct ServerInfo
{
	uint16_t gamePort;
};

struct PlayerInput
{
	float x, y;
	float angle;
};

struct AttackInfo
{
	bool attacking;
};
#pragma pack(pop)

class NetworkManager {
public:
	int m_port;

	NetworkManager();

	~NetworkManager();

	// �����˳�ʼ�� HostMain����
	bool HostInitialize();

	// �ͻ��˳�ʼ�� 
	bool ClientInitialize();

	// ��Ϣ����ģ�� 
	template<typename T>
	void SendGameMessage(MessageType type, const T& data, sockaddr_in* target = nullptr);

	// ��Ϣ����ѭ�� 
	void ReceiveLoop();

	//����
	static NetworkManager* getInstance();

	//���������䡱����
	void HostMain();

	//�����뷿�䡱����
	void ClientMain();

private:
	int CreateUDPSocket(uint16_t port);

	void BroadcastResponder();

	int GetPort(int sock);

	void SendBroadcastProbe(int sock);

	bool ReceiveServerInfo(int sock);

	void HandleMessage(const GameMessage& msg, const sockaddr_in& from);

	int m_socket;
	unsigned int rec_serialNumber;
	unsigned int send_serialNumber;

	bool m_run;
	bool m_broadcastRespondRun = true;

	std::thread m_runThread;
	std::thread m_broadcastRespondThread;

	sockaddr_in m_peerAddr;
};

void RunOnMainThread(std::function<void()> func);