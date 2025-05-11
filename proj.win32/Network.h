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