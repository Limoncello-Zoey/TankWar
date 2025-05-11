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