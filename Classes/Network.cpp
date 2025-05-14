// 网络配置 
#include "Network.h"


//////////////////////////////////////////////////////////////////////////////////
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

//////////////////////////////////////////////////////////////////////////////////

NetworkManager::NetworkManager() : m_socket(0), m_run(false)
{
#ifdef _WIN32 
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif 
}

NetworkManager::~NetworkManager()
{
#ifdef _WIN32 
	WSACleanup();
#endif 
	if (m_runThread.joinable()) {
		m_run = false;
		m_runThread.join();
	}
	if (m_broadcastThread.joinable()) {
		m_broadcastRun = false;
		m_broadcastThread.join();
	}
}

// 房主端初始化 
bool NetworkManager::HostInitialize()
{
	// 关闭之前的线程
	m_run = false;
	if (m_broadcastThread.joinable())
	{
		m_broadcastRun = false;
		m_broadcastThread.join();
	}


	// 初始化，创建游戏通信socket ,用于接收定向的数据
	if (m_socket != 0)
	{
		closesocket(m_socket);
	}
	m_socket = CreateUDPSocket(0);
	if (m_socket == -1) return false;
	m_port = GetPort(m_socket);

	// 启动广播响应线程 
	m_run = true;
	m_broadcastRun = true;
	m_broadcastThread = std::thread(&NetworkManager::BroadcastResponder, this);
	return true;


}

NetworkManager* NetworkManager::getInstance()
{
	static NetworkManager instance;
	return &instance;
}

// 客户端初始化 
bool NetworkManager::ClientInitialize()
{
	// 创建游戏通信socket 
	closesocket(m_socket);
	m_socket = CreateUDPSocket(0);
	if (m_socket == -1) return false;
	m_port = GetPort(m_socket);

	// 发送广播探测 
	do 
	{
		SendBroadcastProbe(m_socket);

	} while (!ReceiveServerInfo(m_socket));

	return true;
}

// 消息发送模板 //???
template<typename T>
void NetworkManager::SendGameMessage(MessageType type, const T& data, sockaddr_in* target)
{
	GameMessage msg;
	msg.serialNumber = 0;
	msg.type = type;
	memcpy(msg.payload, &data, sizeof(T));
	msg.UpdateChecksum();

	sockaddr_in dest = m_peerAddr;
	if (target) dest = *target;

	sendto(m_socket, (char*)&msg, sizeof(msg), 0, (sockaddr*)&dest, sizeof(dest));

	//sendto(sock, (char*)&message, 1, 0, (sockaddr*)&bcAddr, sizeof(bcAddr))
}

// 消息接收循环 
void NetworkManager::ReceiveLoop()
{
	while (m_run)
	{
		sockaddr_in from;
		int fromLen = sizeof(from);

		GameMessage msg;
		int bytes = recvfrom(m_socket, (char*)&msg, sizeof(msg), 0,
			(sockaddr*)&from, &fromLen);

		if (bytes > 0 && msg.Validate())
		{
			HandleMessage(msg, from);
		}
	}
}

int NetworkManager::CreateUDPSocket(uint16_t port)
{
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (::bind(sock, (sockaddr*)&addr, sizeof(addr)) == -1)
	{
		std::cerr << "Bind failed" << std::endl;
		return -1;
	}
	return sock;
}

void NetworkManager::BroadcastResponder()
{
	//接受广播的socket
	int bcSocket = CreateUDPSocket(BROADCAST_PORT);
	if (bcSocket == -1) return;

	// 设置广播权限 
	int broadcastEnable = 1;
	setsockopt(bcSocket, SOL_SOCKET, SO_BROADCAST,
		(char*)&broadcastEnable, sizeof(broadcastEnable));

	while (m_broadcastRun)
	{
		sockaddr_in clientAddr;
		int addrLen = sizeof(clientAddr);

		char buffer[BUFFER_SIZE];
		if (recvfrom(bcSocket, buffer, BUFFER_SIZE, 0,
			(sockaddr*)&clientAddr, &addrLen) > 0)//阻塞
		{
			// 响应服务器信息 
			ServerInfo info{ m_port };
			SendGameMessage(MessageType::ServerBroadcast, info, &clientAddr);
		}
	}

}

int NetworkManager::GetPort(int sock)
{
	struct sockaddr_in assigned_addr;
	int addr_len = sizeof(assigned_addr);
	if (getsockname(sock, (struct sockaddr*)&assigned_addr, &addr_len) < 0)
	{
		std::cerr << "Getsockname failed" << std::endl;
		return -1;
	}
	return ntohs(assigned_addr.sin_port);
}

void NetworkManager::SendBroadcastProbe(int sock)
{
	sockaddr_in bcAddr;
	memset(&bcAddr, 0, sizeof(bcAddr));
	bcAddr.sin_family = AF_INET;
	bcAddr.sin_port = htons(BROADCAST_PORT);
	bcAddr.sin_addr.s_addr = INADDR_BROADCAST;
	int broadcast_flag = 1;
	setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&broadcast_flag, sizeof(broadcast_flag));

	int message = 0;
	int res = sendto(sock, (char*)&message, 1, 0, (sockaddr*)&bcAddr, sizeof(bcAddr));
}

bool NetworkManager::ReceiveServerInfo(int sock)
{
	sockaddr_in from;
	int fromLen = sizeof(from);
	GameMessage msg;

	timeval tv{ 3, 0 }; // 3秒超时 
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));

	if (recvfrom(sock, (char*)&msg, sizeof(msg), 0,
		(sockaddr*)&from, &fromLen) > 0)
	{
		if (!msg.Validate())
		{
			return false;
		}
		if (msg.type == MessageType::ServerBroadcast)
		{
			ServerInfo* info = (ServerInfo*)msg.payload;
			m_peerAddr = from;
			m_peerAddr.sin_port = htons(info->gamePort);
			//获取服务器IP的方法
			/*serverIP = inet_ntoa(m_peerAddr.sin_addr);*/
			m_run = true;
			return true;
		}
	}
	return false;
}

void NetworkManager::HandleMessage(const GameMessage& msg, const sockaddr_in& from)
{

	switch (msg.type)
	{
	case MessageType::ClientJoin:
	{
		ServerInfo* info = (ServerInfo*)msg.payload;

		std::cout << "玩家加入：" << inet_ntoa(from.sin_addr) << ":" << info->gamePort << std::endl;
		// 记录客户端
		
			m_broadcastRun = false;
			//m_broadcastThread.join();
		
		m_peerAddr = from;
		m_peerAddr.sin_port = htons(info->gamePort);

		
		// 将任务添加到主循环中执行
		RunOnMainThread([]() {
			// 这里的代码会在UI线程中执行
			auto gameScene = Gamemode::create();
			Director::getInstance()->replaceScene(gameScene);
			});

		
		break;
	}
	case MessageType::PlayerMove:
	{
		PlayerInput* input = (PlayerInput*)msg.payload;
		std::cout << "Received move: x=" << input->x
			<< " y=" << input->y << std::endl;
		break;
	}
	case MessageType::PlayerAttack:
	{
		AttackInfo* attack = (AttackInfo*)msg.payload;
		std::cout << "Attack direction: "
			<< 1 << std::endl;
		break;
	}
	// 其他消息处理...
	}
}

void NetworkManager::HostMain()
{
	if (!HostInitialize()) return;

	if (m_runThread.joinable()) {
		m_run = false;
		m_runThread.join();
	}
	m_runThread = std::thread(&NetworkManager::ReceiveLoop, this);
}

void NetworkManager::ClientMain()
{
	if (m_runThread.joinable()) 
	{
		m_run = false;
		m_runThread.join();
	}

	ClientInitialize();

	m_runThread = std::thread(&NetworkManager::ReceiveLoop, this);

	// 发送加入请求 
	ServerInfo info{ m_port };
	SendGameMessage(MessageType::ClientJoin, info);

	auto gameScene = Gamemode::create();
	Director::getInstance()->replaceScene(gameScene);
}

//////////////////////////////////////////////////////////////////////////////////

void RunOnMainThread(std::function<void()> func)
{
	auto scheduler = cocos2d::Director::getInstance()->getScheduler();
	scheduler->performFunctionInCocosThread(func);
}