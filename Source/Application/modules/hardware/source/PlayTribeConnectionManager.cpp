
#include "PlayTribeConnectionManager.h"

#if defined ( __linux__ ) || defined( __linux ) || defined( linux )
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <linux/input.h>

#include <sys/sysinfo.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iomanip>
#include <sstream>
#include <cstdlib>

struct sockaddr_in server_addr;
#endif //FP_UART_LINUX

using namespace Ignite;

PlayTribeConnectionManager::PlayTribeConnectionManager(FrontPannelManager* _frontpannelmangr) :frontPannelManager(_frontpannelmangr),
playtribeConnStatus(nullptr),
PlayTribeEthernetWatchThread("PlayTribeConnectionEthernetWatch", IgniteConstants::threadStackSize, osPriorityHigh),
PlayTribeExternalReadThread("PlayTribeExternalReadThread", IgniteConstants::threadStackSize, osPriorityHigh)
{

#if defined ( __linux__ ) || defined( __linux ) || defined( linux )
	system("mkdir -p /home/ftp/Uploads");
	system("chown -R ftp:ftp /home/ftp/Uploads");
	system("chmod 755 /home/ftp/Uploads");
	system("mkdir -p /home/ftp/Downloads");
	system("chmod 755 /home/ftp/Downloads");

	try 
	{
		std::string username = "ftp";
		std::string password = generate_random_password(12); // Generate a random password

		std::string encryption_key = "MTBEIGNITE";  // Example key
		encrypted_password = encrypt_password(password, encryption_key);			// Encrypt the password

		std::cout << "Generated Password: " << password << std::endl;
		std::cout << "Encrypted Password: " << encrypted_password << std::endl;

		std::string decrypted_password = decrypt_password(encrypted_password, encryption_key);
		std::cout << "Decrypted password: " << decrypted_password << std::endl;

		if (set_ftp_user_password(username, password))		// Set the password for the FTP user
		{
			std::cout << "Password set successfully for user " << username << std::endl;
		}
		else 
		{
			std::cerr << "Failed to set password for user " << username << std::endl;
		}
    }
	catch (const std::exception& e) 
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
#endif
}

PlayTribeConnectionManager::~PlayTribeConnectionManager()
{
	frontPannelManager = nullptr;
	app_connected = false;

#if defined( __linux__ ) || defined( __linux ) || defined( linux )
	int socket = clientSocket.load();
	if (socket != -1)
		close(socket);
#endif
	clientSocket.store(-1);

	stopPlayTribeConnectionManagerThreads();
}

void PlayTribeConnectionManager::disconnectClient()
{
	shouldExitWatchThread = true;
	shouldExitReadThread = true;
	app_connected = false;

#if defined( __linux__ ) || defined( __linux ) || defined( linux )
	int socket = clientSocket.load();
	if (socket != -1)
		close(socket);
#endif
	clientSocket.store(-1);
	stopPlayTribeConnectionManagerThreads();

	if (playtribeConnStatus != nullptr)
		playtribeConnStatus->setVisible(false);
}

void PlayTribeConnectionManager::setFirmwareUpdateStatus(DEV_STATUS state)
{
	fw_update_status = state;
}

void PlayTribeConnectionManager::PlayTribeConnectionManagerStartThreads(void)
{
	disconnectClient();

	playtribeConnStatus = PlaytribeConnectionStatus::GetPlaytribeConnectionStatusInstance();
	playtribeConnStatus->waitingForConnectionPopup();

	shouldExitWatchThread = false;
	shouldExitReadThread = false;

	// Nullify the function callbacks safely
	PlayTribeEthernetWatchThreadFunctionCB = nullptr;
	PlayTribeExternalReadThreadFunctionCB = nullptr;

	PlayTribeEthernetWatchThreadFunctionCB = [](void* context) { static_cast<PlayTribeConnectionManager*> (context)->PlayTribeEthernetWatchThreadFunction(); };
	PlayTribeEthernetWatchThread.startThread(PlayTribeEthernetWatchThreadFunctionCB, this);
	PlayTribeExternalReadThreadFunctionCB = [](void* context) { static_cast<PlayTribeConnectionManager*> (context)->PlayTribeExternalReadThreadFunction(); };
	PlayTribeExternalReadThread.startThread(PlayTribeExternalReadThreadFunctionCB, this);
}

void PlayTribeConnectionManager::stopPlayTribeConnectionManagerThreads()
{
	if (auto dw = getCurrentlyModalComponent())
		dw->exitModalState(0);
	PlayTribeEthernetWatchThread.stopThread();
	PlayTribeExternalReadThread.stopThread();
}

void PlayTribeConnectionManager::handleCommandMessage(int commandId)
{
	std::string response = "ok";
	switch (commandId)
	{
	case 1:
		frontPannelManager->engine.SendResponse((moduleId_t)ModuleName::moduleFrontPannel, (moduleId_t)ModuleName::moduleUI, (paramId_t)FrontPannelParam::paramKey_DevStatus, (int)DEV_STATUS::PLAYTRIBE_OP_STARTED);
		sendResponseOverEthernet(response.c_str(), strlen(response.c_str()));
		break;
	case 2:
		frontPannelManager->engine.SendResponse((moduleId_t)ModuleName::moduleFrontPannel, (moduleId_t)ModuleName::moduleUI, (paramId_t)FrontPannelParam::paramKey_DevStatus, (int)DEV_STATUS::PLAYTRIBE_OP_COMPLETED);
		sendResponseOverEthernet(response.c_str(), strlen(response.c_str()));
		break;
	case 3:
		if (fw_update_status == DEV_OP_ONGOING)
		{
			auto status = (int)frontPannelManager->getDevOpStatus();
			DBG("dev_status ---: " << status);
			response = DEVICE_FW_UPDATE_STATUS_REQUEST + (std::string)":" + std::to_string(status);
			sendResponseOverEthernet(response.c_str(), strlen(response.c_str()));
		}
		break;
	case 4:
		if (fw_update_status == DEV_OP_ONGOING)
		{
			auto status = (int)frontPannelManager->getDevOpStatus();
			DBG("dev_status ---: " << status);
			response = DEVICE_FAC_RESET_STATUS_REQUEST + (std::string)":" + std::to_string(status);
			sendResponseOverEthernet(response.c_str(), strlen(response.c_str()));
		}
		break;
	case 5:
		playtribeConnStatus->setVisible(true);
		playtribeConnStatus->toFront(true);
		break;
	case 6:
		playtribeConnStatus->setVisible(false);
		break;
	case 7:
		disconnectClient();
		PlayTribeConnectionManagerStartThreads();
		break;
	}
}

void PlayTribeConnectionManager::PlayTribeEthernetWatchThreadFunction(void)
{
#if defined( __linux__ ) || defined( __linux ) || defined( linux )
	PlayTribeEthernetWatchThread.delayMs(PLAYTIBE_THREAD_INIT_WAIT); //Wait till Init Complete
	int serverSocket;
	bool connected = false;
	bool udpData = false;
	struct sockaddr_in server_addr;

	while (!shouldExitWatchThread)
	{
		if (!app_connected)
		{
			DBG("waiting for the broadcast message from application.....");
			int udp_sockfd;
			sockaddr_in listenAddr, clientAddr;
			char buffer[1024];

			// Create udp socket
			if ((udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
			{
				perror("Socket creation failed");
				continue;
			}

			struct timeval tv;
			tv.tv_sec = 3;  // Timeout after 3 seconds
			tv.tv_usec = 0;
			setsockopt(udp_sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

			// Enable address reuse
			int reuse = 1;
			if (setsockopt(udp_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
			{
				perror("Reuse address option failed");
				close(udp_sockfd);
				continue;
			}

			// Bind to the broadcast address and port
			memset(&listenAddr, 0, sizeof(listenAddr));
			listenAddr.sin_family = AF_INET;
			listenAddr.sin_port = htons(8282);
			listenAddr.sin_addr.s_addr = htonl(INADDR_ANY);

			if (bind(udp_sockfd, (struct sockaddr*)&listenAddr, sizeof(listenAddr)) < 0)
			{
				perror("Bind failed");
				close(udp_sockfd);
				continue;
			}

			// Listen for broadcast messages
			while (true)
			{
				udpData = false;
				socklen_t len = sizeof(clientAddr);
				int n = recvfrom(udp_sockfd, buffer, 1024, 0, (struct sockaddr*)&clientAddr, &len);
				if (n < 0)
				{
					if (errno == EAGAIN || errno == EWOULDBLOCK)
					{
						DBG("No data received, retrying...");
						break;
					}
				}
				buffer[n] = '\0';
				std::cout << "Received broadcast message: " << buffer << std::endl;

				const char* recBroadMessage = buffer;
				const char* message = "PlayTribe: Who is available?";

				if (strcmp(message, recBroadMessage) == 0)
				{
					std::string server_ip = getIPAddress();
					std::string udp_response = "Device:Ignite ServerIP:" + std::string(server_ip) + " Port:" + std::to_string(8080) + " Password:" + encrypted_password;

					if (sendto(udp_sockfd, udp_response.c_str(), udp_response.length(), 0, (sockaddr*)&clientAddr, sizeof(clientAddr)) == SO_ERROR)
					{
						std::cout << "Send failed" << std::endl;
					}
					else
					{
						udpData = true;
						std::cout << "udp response : " << udp_response << std::endl;
						break;
					}
				}
			}

			close(udp_sockfd);   // close udp socket 

			if (serverSocket != -1)
				close(serverSocket);
			serverSocket = -1;
			if (udpData)
			{
				serverSocket = socket(AF_INET, SOCK_STREAM, 0);
				if (serverSocket == -1)
				{
					std::cerr << "PlayTribeConnectionManager: serverSocket failed" << std::endl;
					std::this_thread::sleep_for(std::chrono::seconds(1));
					continue;
				}

				// Allow port reuse
				int opt = 1;
				setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

				// Set SO_LINGER to force immediate closure of the socket
				struct linger so_linger_val;
				so_linger_val.l_onoff = 1;  // Enable linger option
				so_linger_val.l_linger = 0; // Close immediately, discarding unsent data
				setsockopt(serverSocket, SOL_SOCKET, SO_LINGER, &so_linger_val, sizeof(so_linger_val));

				server_addr.sin_family = AF_INET;
				server_addr.sin_port = htons(8080); // Port number
				server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on all available IPs

				if (bind(serverSocket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
				{
					std::cerr << "PlayTribeConnectionManager: bind failed (wait or restart App) : " << strerror(errno) << std::endl;
					close(serverSocket);
					serverSocket = -1;
					this->postCommandMessage(7);
					std::this_thread::sleep_for(std::chrono::seconds(1));
					continue;
				}

				if (listen(serverSocket, 5) == -1)
				{
					std::cerr << "PlayTribeConnectionManager: listen failed (wait or restart App)" << std::endl;
					close(serverSocket);
					std::this_thread::sleep_for(std::chrono::seconds(1));
					continue;
				}
				app_connected = true;
			}

			sockaddr_in clientAddress;
			socklen_t clientAddrLen = sizeof(clientAddress);

			if (clientSocket.load() == -1)
			{
				int socket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddrLen);
				if (socket == -1)
				{
					std::this_thread::sleep_for(std::chrono::seconds(1));
					continue;
				}
				else
				{
					clientSocket.store(socket);
					int flags = fcntl(clientSocket, F_GETFL, 0);
					fcntl(clientSocket, F_SETFL, flags | O_NONBLOCK);
				}
				std::cout << "PlayTribeConnectionManager: Client connected @ : " << clientSocket << std::endl;
				playtribeConnStatus->setConnectionState(true);
				playtribeConnStatus->ConnectionEstablishedPopup();
				this->postCommandMessage(5);
			}
			else
			{
				std::this_thread::sleep_for(std::chrono::seconds(2/*10*/));
			}
		}
	}
#endif
}

void PlayTribeConnectionManager::PlayTribeExternalReadThreadFunction(void)
{
	PlayTribeExternalReadThread.delayMs(PLAYTIBE_THREAD_INIT_WAIT); //Wait till Init Complete
	const int bufferSize = 256;
	[[maybe_unused]] char szBuff[bufferSize] = { 0 };
	while (!shouldExitReadThread)
	{
#if defined( __linux__ ) || defined( __linux ) || defined( linux )
		int dwBytesRead = 0;
		bool isDataFromEth = false;

		if (dwBytesRead <= 0)
		{
			dwBytesRead = 0;
			int socket = clientSocket.load();
			if (socket != -1)
			{
				auto tmpRead = recv(socket, szBuff, bufferSize, 0);
				if (tmpRead > 0)
				{
					if (tmpRead > 0 && tmpRead < bufferSize)
					{
						dwBytesRead = tmpRead;
						isDataFromEth = true;
						std::cout << "PlayTribeConnectionManager: got message via tcp socket : " << dwBytesRead << std::endl;
						processCommandReceivedOverEthernet(szBuff);
						continue;
					}
				}
				else if (tmpRead == 0)
				{
					// Connection closed
					std::cout << "PlayTribeConnectionManager: Connection closed by the peer" << std::endl;
					clientSocket.store(-1);
				}
				else if (tmpRead == -1)
				{
					// Error occurred
					switch (errno)
					{
					case EAGAIN:
						break;
					case EBADF:
					case ECONNRESET:
					case ECONNABORTED:
					case ENETUNREACH:
					case EHOSTUNREACH:
						std::cout << "PlayTribeConnectionManager:ETH ERROR: recv failed Socket reset" << std::endl;
						clientSocket.store(-1);
						this->postCommandMessage(6);
						break;
					default:
						std::cout << "PlayTribeConnectionManager:ETH ERROR: recv failed with unknown error " << errno << std::endl;
						break;
					}
				}
			}
		}
#endif
	}
}

void PlayTribeConnectionManager::processCommandReceivedOverEthernet(const char* cmd)
{
	std::string response = "ok";
	if (strncmp(cmd, DEVICE_INFO_REQUEST, 11) == 0)
	{
		sendDeviceInfoRequestResponse();
	}
	else if (strncmp(cmd, PC_OPERATION_START_REQUEST, 18) == 0)
	{
		DBG("PC app started comm");
		this->postCommandMessage(1);
	}
	else if (strncmp(cmd, PC_OPERATION_END_REQUEST, 16) == 0)
	{
		DBG("PC app stoped comm");
		this->postCommandMessage(2);
		
	}
	else if (strncmp(cmd, DEVICE_SYSTEM_LOG_REQUEST, 11) == 0)
	{
		std::string tar_cmd = "tar -cvf /data/ignite/logs.tar /var/log/; openssl aes-256-cbc -a -salt -pbkdf2 -in /data/ignite/logs.tar -out /data/ignite/logs.img -k 'MTBTMIIGNITE'; rm -f /data/ignite/logs.tar";
		system(tar_cmd.c_str());

		system("rm -rf /home/ftp/Downloads/logs.img");

		std::string cp_cmd = "cp /data/ignite/logs.img  /home/ftp/Downloads/";
		system(cp_cmd.c_str());

		sendResponseOverEthernet(response.c_str(), strlen(response.c_str()));
	}
	else if (strncmp(cmd, DEVICE_LOAD_BACKUP_START_REQUEST, 17) == 0)
	{
		std::string rm_cmd = "rm -rf /home/ftp/Uploads/*";
		system(rm_cmd.c_str());

		sendResponseOverEthernet(response.c_str(), strlen(response.c_str()));
	}
	else if (strncmp(cmd, DEVICE_LOAD_BACKUP_END_REQUEST, 15) == 0)
	{
		std::string ssl_cmd = "openssl aes-256-cbc -a -d -salt -pbkdf2 -in /data/ignite/Ignite_Files/backup.img -out /data/ignite/Ignite_Files/backup.tar -k 'MTBTMIIGNITE'; tar -xvf /data/ignite/Ignite_Files/backup.tar; rm -f /data/ignite/Ignite_Files/backup*";
		system(ssl_cmd.c_str());

		frontPannelManager->engine.SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::ParamEngineRestoreDataBackup, (uint8_t)EngineDataFolderId::AllDataInternalStorage, false);

		sendResponseOverEthernet(response.c_str(), strlen(response.c_str()));
	}
	else if (strncmp(cmd, DEVICE_SAVE_BACKUP, 11) == 0)
	{
		std::string tar_cmd = "rm -f /data/ignite/Ignite_Files/backup* ; tar -cvf /data/ignite/Ignite_Files/backup.tar /data/ignite/Ignite_Files/Ignite_folder_backup; openssl aes-256-cbc -a -salt -pbkdf2 -in /data/ignite/Ignite_Files/backup.tar -out /data/ignite/Ignite_Files/backup.img -k 'MTBTMIIGNITE'; rm -f /data/ignite/Ignite_Files/backup.tar";

		frontPannelManager->engine.SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::ParamEngineCreateDataBackup, (uint8_t)EngineDataFolderId::AllDataInternalStorage, false);
		system(tar_cmd.c_str());

		sendResponseOverEthernet(response.c_str(), strlen(response.c_str()));
}
	else if (strncmp(cmd, DEVICE_FAC_RESET_REQUEST, 13) == 0)
	{
#if defined ( JUCE_LINUX ) || defined( __linux__ )
		IgniteSettings::GetAppSettingsInstance()->startFactoryReset();
#endif
	}
	else if (strncmp(cmd, DEVICE_FW_UPDATE_REQUEST, 15) == 0)
	{
		std::string fileName = cmd + 16;
		std::string cp_cmd = "cp /home/ftp/Uploads/" + fileName + "  /home/ignite/Firmware_Update";
		system(cp_cmd.c_str());

		std::string rm_cmd = "rm -rf /home/ftp/Uploads/" + fileName;
		system(rm_cmd.c_str());

		IgniteDeviceStatus* deviceStatus = IgniteDeviceStatus::GetDevStatusInstance();
		FwStatus_t status = deviceStatus->startFirmwareUpgrade(SwUdateSource_t::PC);
		std::string state = fwstatusToString(status);
		std::cout << "FP: FwStatus : " << state << std::endl;
	}
	else if (strncmp(cmd, DEVICE_FW_UPDATE_STATUS_REQUEST, 9) == 0)
	{
		this->postCommandMessage(3);
	}
	else if (strncmp(cmd, DEVICE_FAC_RESET_STATUS_REQUEST, 14) == 0)
	{
		this->postCommandMessage(4);
	}
}

void PlayTribeConnectionManager::sendResponseOverEthernet(const char* response, size_t len)
{
#if defined( __linux__ ) || defined( __linux ) || defined( linux )
	int socket = clientSocket.load();
	if (socket != -1)
	{
		ssize_t bytesSent = send(socket, response, (int)len, 0);
		if (bytesSent == -1)
		{
			std::cout << "PlayTribeConnectionManager:ETH ERROR: Send failed Socket reset" << errno << std::endl;
			if (errno != EAGAIN)
				clientSocket.store(-1);
		}
		else
			std::cout << "PlayTribeConnectionManager:ETH INFO: Sent " << bytesSent << " bytes" << std::endl;
	}
#endif
}

void PlayTribeConnectionManager::sendDeviceInfoRequestResponse()
{
	struct DeviceInfo info = frontPannelManager->fillDeviceStatusInfo();

	std::string response = std::string("Device Info:serial:") + info.serialNumber + " fwversion:" + info.firmwareVersion + " fwbuild:" + info.firmwareBuildDate +
		" memtotal:" + info.memoryStatus.total + " memavail:" + info.memoryStatus.available + " memused:" + info.memoryStatus.used +
		" inttotal:" + info.internalStorageStatus.totalSpace + " intavail:" + info.internalStorageStatus.availableSpace +
		" intused:" + info.internalStorageStatus.usedSpace + " usbtotal:" + info.usbStorageStatus.totalSpace + " usbavail:" + info.usbStorageStatus.availableSpace;

	sendResponseOverEthernet(response.c_str(), strlen(response.c_str()));
}

std::string PlayTribeConnectionManager::fwstatusToString(FwStatus_t status)
{
	static std::map<FwStatus_t, std::string> statusStrings = {
		{FwStatus_t::INVALID_FW, "INVALID_FW"},
		{FwStatus_t::FW_NOT_PRESENT, "FW_NOT_PRESENT"},
		{FwStatus_t::OLD_FW, "OLD_FW"},
		{FwStatus_t::SAME_FW, "SAME_FW"},
		{FwStatus_t::NEW_FW, "NEW_FW"}
	};

	auto it = statusStrings.find(status);
	if (it != statusStrings.end())
	{
		return it->second;
	}
	else
	{
		return "UNKNOWN";
	}
}

std::string PlayTribeConnectionManager::getIPAddress()
{
#if defined( __linux__ ) || defined( __linux ) || defined( linux )
	std::array<char, 128> buffer;
	std::string result;

	// Open a pipe to run the command
	std::shared_ptr<FILE> pipe(popen("ifconfig", "r"), pclose);
	if (!pipe)
	{
		std::cerr << "Failed to run command." << std::endl;
		return "";
	}

	// Read command output
	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
	{
		result += buffer.data();
	}

	// Look for "inet addr:" in the output
	std::istringstream stream(result);
	std::string line;
	while (std::getline(stream, line))
	{
		size_t pos = line.find("inet addr:");
		if (pos != std::string::npos)
		{	// Extract the IP address
			std::string ip = line.substr(pos + 10);
			ip = ip.substr(0, ip.find(' '));	// Extract up to the next space
			std::cout << "server ip : " << ip << std::endl;
			if (ip != "127.0.0.1")
				return ip;
		}
	}
	return ""; // Return empty if IP address not found
#else
	return ""; // Return empty if IP address not found
#endif
}

std::string PlayTribeConnectionManager::generate_random_password(int length) 
{
	std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	std::string password;
	srand((int)(time(0))); // Initialize random seed

	for (int i = 0; i < length; ++i)
	{
		password += charset[rand() % charset.length()];
	}
	return password;
}

std::string PlayTribeConnectionManager::encrypt_password(const std::string& password, const std::string& key)
{
	std::string encrypted = password;
	for (size_t i = 0; i < password.length(); ++i)
	{
		encrypted[i] = password[i] ^ key[i % key.length()];
	}
	return encrypted;
}

std::string PlayTribeConnectionManager::decrypt_password(const std::string& password, const std::string& key)
{
	return encrypt_password(password, key); // XOR encryption and decryption are the same
}

bool PlayTribeConnectionManager::set_ftp_user_password(const std::string& username, const std::string& password)
{
	//std::string command = "echo '" + username + ":" + password + "' | chpasswd";
	//int result = std::system(command.c_str());
	//return result == 0; // 0 means success
	return 0;
}
