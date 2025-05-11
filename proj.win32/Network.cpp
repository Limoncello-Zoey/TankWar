// 网络配置 
#include "Network.h"
const uint16_t BROADCAST_PORT = 21567;
const int BUFFER_SIZE = 64;

bool GameMessage::Validate() const 
{
	return checksum == CalculateChecksum();
}

void GameMessage::UpdateChecksum()
{
	checksum = CalculateChecksum();
}

uint16_t GameMessage::CalculateChecksum() const
{
	// 简易校验和实现 
	const uint8_t* bytes = reinterpret_cast<const uint8_t*>(this);
	uint16_t sum = 0;
	for (size_t i = 2; i < sizeof(GameMessage); ++i)
	{
		sum += bytes[i];
	}
	return ~sum;
}

