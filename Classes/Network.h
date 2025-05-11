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

// 消息类型枚举 
enum class MessageType : uint8_t 
{
    ServerBroadcast,  // 服务器广播响应 
    ClientJoin,       // 客户端加入请求 
    PlayerMove,       // 玩家移动 
    PlayerAttack,     // 玩家攻击 
    GameSync          // 游戏状态同步 
};

#pragma pack(push, 1)
// 基础消息结构 
struct GameMessage 
{
	uint16_t checksum;          // 校验和
	unsigned int serialNumber;  // 序列号
	MessageType type;           // 消息类型
	char payload[BUFFER_SIZE - sizeof(MessageType) - sizeof(uint16_t) - sizeof(unsigned int)];

	bool Validate() const;
	void UpdateChecksum();

private:
	uint16_t CalculateChecksum() const;
};

// 具体消息结构 
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

	// 房主端初始化 
	bool HostInitialize();

	// 客户端初始化 
	bool ClientInitialize(std::string& serverIP);

	// 消息发送模板 
	template<typename T>
	void SendGameMessage(MessageType type, const T& data, sockaddr_in* target);

	// 消息接收循环 
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