// 网络配置 
#include "Network.h"
#include "Gamemode.h"
#include "Gameover.h"
#include "Tank.h"
#include "AudioEngine.h"

using namespace cocos2d;

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
	forceExit = false;
#ifdef _WIN32 
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif 
	m_runThread = std::thread(&NetworkManager::ReceiveLoop, this);
	m_broadcastRespondThread = std::thread(&NetworkManager::BroadcastResponder, this);
}

NetworkManager::~NetworkManager()
{
	//// 获取底层句柄
	//HANDLE handle = static_cast<HANDLE>(m_runThread.native_handle());
	//TerminateThread(handle, 0);
	//handle = static_cast<HANDLE>(m_broadcastRespondThread.native_handle());
	//TerminateThread(handle, 0);
	forceExit = true;
	Reset();	
	if (m_runThread.joinable()) m_runThread.join();
	if (m_broadcastRespondThread.joinable()) m_broadcastRespondThread.join();
	
#ifdef _WIN32 
	WSACleanup();
#endif 
}

// 房主端初始化 
bool NetworkManager::HostInitialize()
{
	Reset();
	m_socket = CreateUDPSocket(0);
	if (m_socket == -1) return false;
	m_port = GetPort(m_socket);
	//接受广播的socket
	m_bcSocket = CreateUDPSocket(BROADCAST_PORT);
	if (m_bcSocket == -1) return false;

	// 设置广播权限 
	int broadcastEnable = 1;
	setsockopt(m_bcSocket, SOL_SOCKET, SO_BROADCAST,
		(char*)&broadcastEnable, sizeof(broadcastEnable));

	// 启动广播响应线程 
	m_run = true;
	m_broadcastRespondRun = true;

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
	Reset();
	// 创建游戏通信socket 
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

// 消息接收循环 
// run线程
void NetworkManager::ReceiveLoop()
{
	while (true)
	{
		if (forceExit) return;
		if (!m_run) {
			std::this_thread::sleep_for(std::chrono::milliseconds(16));
			continue;
		}
		sockaddr_in from;
		int fromLen = sizeof(from);

		GameMessage msg;

		//阻塞
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

	timeval tv{ 3, 0 }; // 3秒超时 
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));

	int opt = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

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

	while (true)
	{
		if (forceExit) return;
		if (!m_broadcastRespondRun || m_bcSocket == 0 || m_bcSocket==-1) {
			std::this_thread::sleep_for(std::chrono::milliseconds(16));
			continue;
		}
		sockaddr_in clientAddr;
		int addrLen = sizeof(clientAddr);

		char buffer[BUFFER_SIZE];
		if (recvfrom(m_bcSocket, buffer, BUFFER_SIZE, 0,
			(sockaddr*)&clientAddr, &addrLen) > 0)//阻塞
		{
			// 响应服务器信息 
			ServerInfo info{ m_port , 1024 };
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


//run线程中的,阻塞之后调用
void NetworkManager::HandleMessage(const GameMessage& msg, const sockaddr_in& from)
{
	if (msg.serialNumber < rec_serialNumber) return;
	rec_serialNumber = msg.serialNumber;

	switch (msg.type)
	{
		//host收到了客户端的广播，并回复了自己的端口号（不指定但固定（也不能算））
		//现在host又收到了client的join消息
		case MessageType::ClientJoin:
		{
			//常见的妙手！
			ServerInfo* info = (ServerInfo*)msg.payload;

			//终止广播的接收
			m_broadcastRespondRun = false;

			//记录对方的ip与端口
			m_peerAddr = from;
			m_peerAddr.sin_port = htons(info->gamePort);

			// 将任务添加到主循环中执行
			RunOnMainThread([=]() 
			{
				// 这里的代码会在UI线程中执行
				Gamemode::_self = 1;
				auto gameScene = Gamemode::create();
				gameScene->_mapid = info->mapid;
				gameScene->MapSetUp(gameScene->_mapid);
				gameScene->setCameraMask((unsigned short)CameraFlag::USER1, true);
				cocos2d::Director::getInstance()->replaceScene(gameScene);
			});
			
			break;
		}
		case MessageType::Position:
		{
			/*if(!Gamemode::Other()->isRunning()) break;*/
			RunOnMainThread([=]() 
			{
				TankPosition* input = (TankPosition*)msg.payload;
				if (!dynamic_cast<Gamemode*>(cocos2d::Director::getInstance()->getRunningScene()) || Gamemode::Other() == nullptr) 
					return;
				auto pos = cocos2d::Vec2(input->x,input->y);
				Vec2 tilePos;
				tilePos = Vec2(
					floor((pos.x) / 45.0f),
					floor(pos.y / 45.0f));
				if (tilePos.x < 0 || tilePos.x >= 20 || tilePos.y < 0 || tilePos.y >= 16) {
					return;
				}
				/*if (pos.x == input->x && pos.y == input->y && Gamemode::Other()->getRotation() == input->angle) {
					return;
				}*/
				Gamemode::Other()->setPosition(input->x, input->y);
				Gamemode::Other()->setRotation(input->angle);
			});
			break;
		}
		case MessageType::Attack:
		{
			RunOnMainThread([=]() {
				Gamemode::Other()->fire();
				});
			break;
		}
		//收到消息意味着自己赢了
		case MessageType::Die: {
			RunOnMainThread([=]() {
				auto gameover = dynamic_cast<Gameover*>(Gameover::createScene());
				gameover->ShowResultImage(true); // 显示胜利图片
				Director::getInstance()->replaceScene(gameover);
				cocos2d::experimental::AudioEngine::stopAll();
				});
			break;
		}
	}
}

void NetworkManager::HostMain()
{
	if (!HostInitialize()) return;
	Gamemode::_self = 1;
}

void NetworkManager::ClientMain()
{
	ClientInitialize();

	Gamemode::_self = 2;
	auto gameScene = Gamemode::create();
	gameScene->_mapid = gameScene->MapSetUp();
	 
	// 发送加入请求 
	ServerInfo info{ m_port , gameScene->_mapid };
	SendGameMessage(MessageType::ClientJoin, info);

	gameScene->setCameraMask((unsigned short)CameraFlag::USER1, true);
	
	cocos2d::Director::getInstance()->replaceScene(gameScene);
}

void NetworkManager::Reset()
{
	//先关线程
	m_run = false;
	m_broadcastRespondRun = false;

	//再关socket
	closesocket(m_socket);
	closesocket(m_bcSocket);
	//再置零
	m_socket = 0;
	m_bcSocket = 0;
	rec_serialNumber = 0;
	send_serialNumber = 0;
	m_port = 0;
	Gamemode::_self = 0;
	Gamemode::Tank1 = nullptr;
	Gamemode::Tank2 = nullptr;
}

//////////////////////////////////////////////////////////////////////////////////

void RunOnMainThread(std::function<void()> func)
{
	auto scheduler = cocos2d::Director::getInstance()->getScheduler();
	scheduler->performFunctionInCocosThread(func);
}