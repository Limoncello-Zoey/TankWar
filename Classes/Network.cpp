// �������� 
#include "Network.h"
#include "Gamemode.h"
#include "Gameover.h"
#include "Tank.h"

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
	// ����У���ʵ�� 
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
	Reset();
}

// �����˳�ʼ�� 
bool NetworkManager::HostInitialize()
{
	Reset();
	m_socket = CreateUDPSocket(0);
	if (m_socket == -1) return false;
	m_port = GetPort(m_socket);

	// �����㲥��Ӧ�߳� 
	m_run = true;
	m_broadcastRespondRun = true;
	m_broadcastRespondThread = std::thread(&NetworkManager::BroadcastResponder, this);
	return true;
}

NetworkManager* NetworkManager::getInstance()
{
	static NetworkManager instance;
	return &instance;
}

// �ͻ��˳�ʼ�� 
bool NetworkManager::ClientInitialize()
{
	Reset();
	// ������Ϸͨ��socket 
	m_socket = CreateUDPSocket(0);
	if (m_socket == -1) return false;
	m_port = GetPort(m_socket);

	// ���͹㲥̽�� 
	do 
	{
		SendBroadcastProbe(m_socket);

	} while (!ReceiveServerInfo(m_socket));

	return true;
}

// ��Ϣ����ѭ�� 
// run�߳�
void NetworkManager::ReceiveLoop()
{
	while (m_run)
	{
		sockaddr_in from;
		int fromLen = sizeof(from);

		GameMessage msg;

		//����
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
	//���ܹ㲥��socket
	int bcSocket = CreateUDPSocket(BROADCAST_PORT);
	if (bcSocket == -1) return;

	// ���ù㲥Ȩ�� 
	int broadcastEnable = 1;
	setsockopt(bcSocket, SOL_SOCKET, SO_BROADCAST,
		(char*)&broadcastEnable, sizeof(broadcastEnable));

	while (m_broadcastRespondRun)
	{
		sockaddr_in clientAddr;
		int addrLen = sizeof(clientAddr);

		char buffer[BUFFER_SIZE];
		if (recvfrom(bcSocket, buffer, BUFFER_SIZE, 0,
			(sockaddr*)&clientAddr, &addrLen) > 0)//����
		{
			// ��Ӧ��������Ϣ 
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

	timeval tv{ 3, 0 }; // 3�볬ʱ 
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
			//��ȡ������IP�ķ���
			/*serverIP = inet_ntoa(m_peerAddr.sin_addr);*/
			m_run = true;
			return true;
		}
	}
	return false;
}


//run�߳��е�,����֮�����
void NetworkManager::HandleMessage(const GameMessage& msg, const sockaddr_in& from)
{
	if (msg.serialNumber < rec_serialNumber) return;
	rec_serialNumber = msg.serialNumber;

	switch (msg.type)
	{
		//host�յ��˿ͻ��˵Ĺ㲥�����ظ����Լ��Ķ˿ںţ���ָ�����̶���Ҳ�����㣩��
		//����host���յ���client��join��Ϣ
		case MessageType::ClientJoin:
		{
			//���������֣�
			ServerInfo* info = (ServerInfo*)msg.payload;

			//��ֹ�㲥�Ľ���
			m_broadcastRespondRun = false;

			//��¼�Է���ip��˿�
			m_peerAddr = from;
			m_peerAddr.sin_port = htons(info->gamePort);

			// ��������ӵ���ѭ����ִ��
			RunOnMainThread([=]() 
			{
				// ����Ĵ������UI�߳���ִ��
				Gamemode::_self = 1;
				auto gameScene = Gamemode::create();
				cocos2d::Director::getInstance()->replaceScene(gameScene);
			});
			
			break;
		}
		case MessageType::Position:
		{
			/*if(!Gamemode::Other()->isRunning()) break;*/
			TankPosition* input = (TankPosition*)msg.payload;
			RunOnMainThread([=]() 
			{
				if (!dynamic_cast<Gamemode*>(cocos2d::Director::getInstance()->getRunningScene()) || Gamemode::Other() == nullptr) return;
				Gamemode::Other()->setPosition(input->x, input->y);
				Gamemode::Other()->setRotation(input->angle);
			});
			break;
		}
		case MessageType::Attack:
		{
			RunOnMainThread([=]() 
			{
				Gamemode::Other()->fire();
			});
			break;
		}
		//�յ���Ϣ��ζ���Լ�Ӯ��
		case MessageType::Die:
		{
			RunOnMainThread([=]()
			{
				auto gameover = dynamic_cast<Gameover*>(Gameover::createScene());
				gameover->_win = true;
				gameover->ChangeText("You Win!");
				cocos2d::Director::getInstance()->replaceScene(gameover);
				//this->removeFromParent();
			});
			break;
		}
	}
}

void NetworkManager::HostMain()
{
	if (!HostInitialize()) return;
	m_runThread = std::thread(&NetworkManager::ReceiveLoop, this);
}

void NetworkManager::ClientMain()
{
	ClientInitialize();
	m_runThread = std::thread(&NetworkManager::ReceiveLoop, this);

	// ���ͼ������� 
	ServerInfo info{ m_port };
	SendGameMessage(MessageType::ClientJoin, info);
	Gamemode::_self = 2;
	
	auto gameScene = Gamemode::create();
	cocos2d::Director::getInstance()->replaceScene(gameScene);
}

void NetworkManager::Reset()
{
	//�ȹ��߳�
	m_run = false;
	if (m_runThread.joinable())
	{
		//m_runThread.join();
		m_runThread.detach();
	}

	m_broadcastRespondRun = false;
	if (m_broadcastRespondThread.joinable())
	{
		//m_broadcastRespondThread.join();
		m_broadcastRespondThread.detach();
	}
	//�ٹ�socket
	closesocket(m_socket);
	//������
	m_socket = 0;
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