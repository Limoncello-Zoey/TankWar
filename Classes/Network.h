#pragma once
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <cstring>
#include <vector>
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
#include "cocos2d.h"

const uint16_t BROADCAST_PORT = 21567;
const int BUFFER_SIZE = 64;

// 消息类型枚举 
enum class MessageType : uint8_t 
{
    ServerBroadcast,	// 服务器广播响应 
    ClientJoin,			// 客户端加入请求 
    Position,			// 玩家移动 
    Attack,				// 玩家攻击 
	Die,				// 玩家死亡
};

#pragma pack(push, 1)
// 数据包结构
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
//服务器广播消息
struct ServerInfo
{
	uint16_t gamePort;
	uint16_t mapid;
};

struct TankPosition
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

	void Reset();

	NetworkManager();

	~NetworkManager();

	// 房主端初始化 HostMain调用
	bool HostInitialize();

	// 客户端初始化 
	bool ClientInitialize();

	//// 消息发送模板 
	//template<typename T>
	//void SendGameMessage(MessageType type, const T& data, sockaddr_in* target);

	template<typename T>
	void SendGameMessage(MessageType type, const T& data, sockaddr_in* target = nullptr)
	{
		GameMessage msg;
		msg.serialNumber = ++send_serialNumber;
		msg.type = type;
		memcpy(msg.payload, &data, sizeof(T));
		msg.UpdateChecksum();

		sockaddr_in dest = m_peerAddr;
		if (target) dest = *target;

		sendto(m_socket, (char*)&msg, sizeof(msg), 0, (sockaddr*)&dest, sizeof(dest));
	}

	// 消息接收循环 
	void ReceiveLoop();

	//单例
	static NetworkManager* getInstance();

	//“创建房间”调用
	void HostMain();

	//“加入房间”调用
	void ClientMain();

private:
	int CreateUDPSocket(uint16_t port);

	void BroadcastResponder();

	int GetPort(int sock);

	void SendBroadcastProbe(int sock);

	bool ReceiveServerInfo(int sock);

	void HandleMessage(const GameMessage& msg, const sockaddr_in& from);

	int m_socket;
	int m_bcSocket;
	unsigned int rec_serialNumber;
	unsigned int send_serialNumber;

	bool m_run;
	bool m_broadcastRespondRun = true;

	bool forceExit = false; // 退出标志

	std::thread m_runThread;
	std::thread m_broadcastRespondThread;

	//对方的端口和ip
	sockaddr_in m_peerAddr;
};

void RunOnMainThread(std::function<void()> func);