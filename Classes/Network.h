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
// ������Ϣ�ṹ 
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

	// �����˳�ʼ�� 
	bool HostInitialize();

	// �ͻ��˳�ʼ�� 
	bool ClientInitialize(std::string& serverIP);

	// ��Ϣ����ģ�� 
	template<typename T>
	void SendGameMessage(MessageType type, const T& data, sockaddr_in* target);

	// ��Ϣ����ѭ�� 
	void ReceiveLoop();

private:
	int CreateUDPSocket(uint16_t port);

	void BroadcastResponder();

	int GetPort(int sock);

	void SendBroadcastProbe(int sock);

	bool ReceiveServerInfo(int sock, std::string& serverIP);

	void HandleMessage(const GameMessage& msg, const sockaddr_in& from);

	int m_socket;

	std::atomic<bool> m_run;
	bool m_broadcastRun = true;
	std::thread m_broadcastThread;
	sockaddr_in m_peerAddr;
};