
/*
 * Copyright (c) - 2021 - MUSIC Tribe, MY.
 * This file is part of IGNITE.
 *
 * IGNITE is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * IGNITE is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with IGNITE. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "usbFunctionFS.h"

#if( JUCE_LINUX || __linux__ )
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/time.h>
#include <dirent.h>
#include <limits.h>

#endif

using namespace Ignite;

USBFunctionFS::USBFunctionFS(FrontPannelManager* frontpannelmangr, IgniteEngine& model)
	:frontPannelManager(frontpannelmangr), juce::Thread("USBFS"), engine(model), usbFilePath("/dev/usb-utp0/ep0")
{
	DBG("USBFunctionFS Constructor called.. ");
#if( JUCE_LINUX || __linux__ )
	init();
#endif
}

USBFunctionFS::~USBFunctionFS()
{
#if( JUCE_LINUX || __linux__ )
	DBG("USBFunctionFS Destructor called.. ");
	signalThreadShouldExit();
	stopThread(2000);

	closeFileDescriptor(g_ep_write);
	closeFileDescriptor(g_ep_read);
#endif
}

void USBFunctionFS::init()
{
#if( JUCE_LINUX || __linux__ )
	juce::String epSinkPath = usbFilePath.substring(0, usbFilePath.length() - 1) + "1";
	g_ep_write = openFileDescriptor(epSinkPath);
	if (g_ep_write < 0)
		handleOpenFileError(epSinkPath);

	juce::String epSourcePath = usbFilePath.substring(0, usbFilePath.length() - 1) + "2";
	g_ep_read = openFileDescriptor(epSourcePath);
	if (g_ep_read < 0)
		handleOpenFileError(epSourcePath);
#endif
}

void USBFunctionFS::run()
{
#if( JUCE_LINUX || __linux__ )
	DBG("In USBFunctionFS run func...");
	constexpr int timeoutSeconds = 15000; // Adjust the timeout value as needed

	while (!threadShouldExit())
	{
		if (g_ep_read > 0 && g_ep_write > 0)
		{
			if (!timerRunning)
			{
				startTimer(timeoutSeconds * 1000); // Convert seconds to milliseconds
				timerRunning = true;
			}

			juce::String command;
			readCommandFromUSB(command);
			if (!command.isEmpty())
			{
				responseReceived = true;
				isConnected.store(true);
				handleCommand(command);
				stopTimer();
				timerRunning = false;
			}
		}
		else
		{
			DBG("Invalid usbfiles and calling init");
			init();
		}
		responseReceived = false; // Reset flag for the next iteration

		// Sleep for a short duration to avoid busy-waiting
		juce::Thread::sleep(100);
	}
	DBG("USBFunctionFS thread exited...");
#endif
}

void USBFunctionFS::timerCallback()
{
	DBG("Timer Callback Called");
	if (!responseReceived)
	{
		DBG("No response from PC app within timeout period");
		signalThreadShouldExit();
		usbmsg_t data = { U_FAILED, 0, 0 };
		data.command_status = USB_STATUS::U_TIMEOUT;
		data.downloadfilesize = 0;
		data.percentage = 0;
		if (usbCallback)
		{
			usbCallback(data);
		}
		timerRunning = false;
		stopTimer();
	}
	// Handle timeout here
}

void USBFunctionFS::start()
{
	DBG("USBFunctionFS Thread Start Called..");
	startThread();
}

void USBFunctionFS::stop()
{
	if (isThreadRunning())
		signalThreadShouldExit();
}

void USBFunctionFS::setFirmwareUpdateStatus(DEV_STATUS state)
{
	fw_update_status = state;
}

int USBFunctionFS::openFileDescriptor(const juce::String& filePath)
{
	int fd = -1;
#if( JUCE_LINUX || __linux__ )
	fd = open(filePath.toRawUTF8(), O_RDWR);
#endif
	return fd;
}

void USBFunctionFS::closeFileDescriptor(int fd)
{
#if( JUCE_LINUX || __linux__ )
	if (fd >= 0)
		::close(fd);
#endif
}

void USBFunctionFS::handleOpenFileError(const juce::String& filePath)
{
#if( JUCE_LINUX || __linux__ )
	DBG("Can't open file " << filePath);
#endif
}

int USBFunctionFS::writeData(void* p, size_t size)
{
	int r = -1;
#if( JUCE_LINUX || __linux__ )
	int flags = fcntl(g_ep_write, F_GETFL, 0);
	if (flags == -1)
	{
		// Handle error
	}
	flags |= O_NONBLOCK;
	if (fcntl(g_ep_write, F_SETFL, flags) == -1)
	{
		// Handle error
	}
	r = write(g_ep_write, p, size);
	if (r < 0)
		printf("failure write to usb ep\n");
#endif
	return r;
}

int USBFunctionFS::writeFile(int fp, void* p, size_t size)
{
#if( JUCE_LINUX || __linux__ )
	int            flag;
	fd_set         rfds;
	struct timeval tv;
	ssize_t        sz;
	uint8_t* buff = (uint8_t*)p;
	union FSdata   fm;
	int            flags;

	tv.tv_sec = 0;
	tv.tv_usec = 100000;

	flags = fcntl(fp, F_GETFL);
	flags |= O_NONBLOCK;
	if (fcntl(fp, F_SETFL, flags))
	{
		printf("fctl failure\n");
		return -1;
	}

	FD_ZERO(&rfds);
	FD_SET(fp, &rfds);

	while (size > 0)
	{
		sz = write(fp, buff, size);

		if (sz == -1)
		{
			if (errno == EAGAIN)
				sz = 0;
			else
				return -errno;
		}

		buff += sz;
		size -= sz;
		fm.key = INFO;
		writeData(&fm, 4);
		select(fp + 1, NULL, &rfds, NULL, &tv);
	}

	if (size == 0)
		return buff - (uint8_t*)p;
	else
#endif
		return -1;
}

void USBFunctionFS::readCommandFromUSB(juce::String& command)
{
#if( JUCE_LINUX || __linux__ )
	constexpr int bufferSize = 512;
	char          buffer[bufferSize];
	memset(buffer, 0, bufferSize);

	int flags = fcntl(g_ep_read, F_GETFL, 0);
	fcntl(g_ep_read, F_SETFL, flags | O_NONBLOCK);

	ssize_t bytesRead = ::read(g_ep_read, buffer, bufferSize - 1);
	if (bytesRead > 0)
	{
		buffer[bytesRead] = '\0';
		command = juce::String(buffer);
	}
	else
	{
	//	DBG("Failed to read comamnd from Read FD");
	}
#endif
}

int USBFunctionFS::handleAckCommand()
{
	union FSdata fs = { 0 };
	fs.key = OKAY;
	if (fw_update_status != INVALID_STATE)
	{

	}
	// Send the result back
	writeData(&fs, 4);
	isConnected.store(true);
	// Implement your Ack handling logic here
	return 0;
}

int USBFunctionFS::handleWFileCommand(juce::String& filename, int size)
{
	union FSdata  fs = { 0 };;
	static size_t tfilesize = 0;
	usbmsg_t data = { U_FAILED, 0, 0 };

#if defined( JUCE_LINUX ) || defined( __linux__ )
	// Open the file for writing
	write_filefd = open(filename.toRawUTF8(), O_WRONLY | O_CREAT,
		S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
#endif

	if (write_filefd < 0)
		fs.key = FAIL;
	else
		fs.key = OKAY;

	// Send the result back
	writeData(&fs, 4);
	tfilesize = size;
	data.totalfilesize = tfilesize;
	data.command_status = USB_STATUS::U_INIT;
	data.downloadfilesize = 0;

	if (usbCallback)
	{
		usbCallback(data);
	}

	// Implement your WFile handling logic here
	return 0;
}

int USBFunctionFS::handleWDataCommand(const juce::StringArray& parts, int size) // size indicates size of data  
{
#if defined( JUCE_LINUX ) || defined( __linux__ )
	union FSdata  fs = { 0 };;
	uint32_t      data_size; /* download:0x100000 */
	int           rs;
	int           ret = 0;
	usbmsg_t      data = { U_FAILED, 0, 0 };
	uint32_t      status = OKAY;
	static size_t tfilesize = 0;
	static size_t dfilesize = 0;
	int           flags;
	if (size < 0)//if( parts.size() < 2 )
	{
		DBG("Invalid command format: Missing data size");
		return -1;
	}

	// Extract the data size
	data_size = size;

	void* p = malloc(data_size);
	if (p)
	{
		fs.key = DATA;
	}
	else
	{
		fs.key = FAIL;
		writeData(&fs, 4);
		return -1;
	}
	sprintf((char*)(fs.data), "%08X", data_size);
	writeData(&fs, 4 + strlen((char*)fs.data));

	data.command_status = USB_STATUS::U_DOWNLOAD;
	dfilesize = dfilesize + data_size;
	data.downloadfilesize = dfilesize;

	if (tfilesize)
	{
		data.percentage = (static_cast<double>(dfilesize) / tfilesize) * 100;
	}
	if (usbCallback)
	{
		usbCallback(data);
	}
	if (read(g_ep_read, p, data_size) < 0)
		status = FAIL;

	ret = writeFile(write_filefd, p, data_size);
	if (ret < 0)
		status = FAIL;

	free(p);

	memset(&fs, 0, sizeof(fs));
	if (stdoutfd >= 0)
	{
		flags = fcntl(stdoutfd, F_GETFL);
		flags |= O_NONBLOCK;
		if (fcntl(stdoutfd, F_SETFL, flags))
		{
			DBG("fctl failure\n");
			return -1;
		}
		while (((rs = read(stdoutfd, fs.data, MAX_FRAME_DATA_SIZE)) > 0) && threadShouldExit())
		{
			fs.key = INFO;
			writeData(&fs, rs + 4);
		}
	}
	fs.key = status;
	if (ret == -EPIPE)
	{
		strcpy((char*)fs.data, "EPIPE");
		writeData(&fs, 4 + strlen("EPIPE"));
	}
	else
	{
		writeData(&fs, 4);
	}
#endif
	return 0;
}

int USBFunctionFS::handleReadFileCommand(const juce::String& cmdType)
{
#if defined( JUCE_LINUX ) || defined( __linux__ )
	union FSdata fs = { 0 };;

	if (cmdType.length() < 9)
	{
		DBG("Invalid command format: Missing filename");
		return -1;
	}
	std::string cmd = cmdType.toStdString();
	juce::String filename = cmd.c_str() + 9;
	//DBG("file name : " << filename);
	size_t       size = 0;

	if (filename.startsWithChar('-'))
	{
		write_filefd = stdoutfd;
	}
	else
	{
		write_filefd = open(filename.toStdString().c_str(), O_RDONLY);
		struct stat st;
		memset(&st, 0, sizeof(st));
		if (stat(filename.toRawUTF8(), &st) == 0)
		{
			size = st.st_size;
		}
		else
		{
			DBG("Failed to get file size");
			return -1;
		}
	}
	if (write_filefd < 0)
	{
		fs.key = FAIL;
		DBG("file open failed");
		writeData(&fs, 4 + std::strlen((char*)fs.data));
	}
	else
	{
		fs.key = OKAY;
		DBG("file open success");
		std::sprintf((char*)fs.data, "%016zX", size);
		writeData(&fs, 4 + std::strlen((char*)fs.data));
	}

#endif
	return 0;
}

int USBFunctionFS::handleCloseFileCommand()
{
	usbmsg_t data = { U_FAILED, 0, 0 };
#if( JUCE_LINUX || __linux__ )
	union FSdata fs = { 0 };;
	close(write_filefd);
	write_filefd = -1;
	fs.key = OKAY;
	writeData(&fs, 4);
#endif
	data.command_status = USB_STATUS::U_DONE;
	data.percentage = 100.0;
	data.downloadfilesize = data.totalfilesize;
	if (usbCallback)
	{
		usbCallback(data);
	}
	return 0;
}

int USBFunctionFS::handleUploadCommand(const juce::StringArray& parts)
{
#if defined( JUCE_LINUX ) || defined( __linux__ )
	union FSdata fs = { 0 };;
	int max = 0x10000;
	void* p = malloc(max);
	int ret = 0;
	if (p == NULL) {
		fs.key = FAIL;
		writeData(&fs, 4);
	}
	else {
		do {
			ret = read(write_filefd, p, max);
			if (ret < 0) {
				if (errno == EAGAIN) {
					//retry read
					fs.key = DATA;
					sprintf((char*)fs.data, "%08X", 0);
					writeData(&fs, 12);
				}
				else {
					fs.key = FAIL;
					writeData(&fs, 4);
					break;
				}
			}
			else {
				fs.key = DATA;
				sprintf((char*)fs.data, "%08X", ret);
				writeData(&fs, 12);
				writeData(p, ret);
				fs.key = OKAY;
				writeData(&fs, 4);
				break;
			}
		} while (1);
	}
	free(p);
#endif
	return 0;
}

int USBFunctionFS::handleFirmwareCompleteCommand()
{
	usbmsg_t data = { U_FAILED, 0, 0 };
#if( JUCE_LINUX || __linux__ )
	union FSdata fs = { 0 };;
	close(write_filefd);
	write_filefd = -1;
	fs.key = OKAY;
	writeData(&fs, 4);
#endif

	data.command_status = USB_STATUS::U_DONE;
	data.percentage = 100.0;
	data.downloadfilesize = data.totalfilesize;
	if (usbCallback)
	{
		usbCallback(data);
	}
	return 0;
}

int USBFunctionFS::handleUCmdCommand(const char* cmd)
{
#if defined( JUCE_LINUX ) || defined( __linux__ )
	int pid;
	int out;
	union FSdata fs = { 0 };;
	int size;
	int p;
	int pstat;
	int flags;
	struct timeval tv;
	fd_set rfds;
	tv.tv_sec = 0;
	tv.tv_usec = 50000;

	pid = popen2(cmd + 5, NULL, &out);
	if (pid < 0) {
		DBG("Failure excecu cmd: " + juce::String(cmd + 5));
		memset(&fs, 0, sizeof(fs));
		fs.key = FAIL;
		strcpy((char*)fs.data, "Failure to folk process");
		return -1;
	}
	memset(&fs, 0, sizeof(fs));

	flags = fcntl(out, F_GETFL);
	flags |= O_NONBLOCK;
	if (fcntl(out, F_SETFL, flags)) {
		DBG("fctl failure\n");
		return -1;
	}

	FD_ZERO(&rfds);
	FD_SET(out, &rfds);
	do {
		int retval;
		p = waitpid(pid, &pstat, WNOHANG);
		retval = select(out + 1, &rfds, NULL, NULL, &tv);
		do {
			size = read(out, fs.data, MAX_FRAME_DATA_SIZE);
			if (size >= 0) {
				fs.key = INFO;
				writeData(&fs, size + 4);
			}
		} while (size == MAX_FRAME_DATA_SIZE);

		fs.key = INFO;
		writeData(&fs, 4);

	} while (p == 0);

	fs.key = WEXITSTATUS(pstat) ? FAIL : OKAY;
	writeData(&fs, 4);

	close(out);
#endif
	return 0;
}
#if defined( JUCE_LINUX ) || defined( __linux__ )
pid_t USBFunctionFS::popen2(const char* command, int* infp, int* outfp)
{
	int p_stdin[2], p_stdout[2];
	pid_t pid;

	if (pipe(p_stdin) != 0 || pipe(p_stdout) != 0)
		return -1;

	pid = fork();

	if (pid < 0)
		return pid;
	else if (pid == 0) {
		close(p_stdin[WRITE]);
		if (infp == NULL)
			close(p_stdin[READ]);
		else
			dup2(p_stdin[READ], READ);
		close(p_stdout[READ]);
		if (outfp == NULL)
			close(p_stdout[WRITE]);
		else
			dup2(p_stdout[WRITE], WRITE);

		execl("/bin/sh", "sh", "-c", command, NULL);
		perror("execl");
		exit(1);
	}

	if (infp == NULL)
		close(p_stdin[WRITE]);
	else
		*infp = p_stdin[WRITE];

	if (outfp == NULL)
		close(p_stdout[READ]);
	else
		*outfp = p_stdout[READ];

	close(p_stdin[READ]);
	close(p_stdout[WRITE]);
	return pid;
}
#endif

int USBFunctionFS::handleDeviceInfoCommand()
{
	// pc requesting device status info like ram status,internal and usb status
#if defined ( JUCE_LINUX ) || defined( __linux__ )
	union FSdata fs = { 0 };;
	struct DeviceInfo info = frontPannelManager->fillDeviceStatusInfo();
	writeData(info.serialNumber, sizeof(info.serialNumber)); // Send the DeviceInfo struct data
	printf("Serial Number data: %s\n", info.serialNumber);

	writeData(info.firmwareVersion, sizeof(info.firmwareVersion));
	printf("firmwareVersion data: %s\n", info.firmwareVersion);

	writeData(info.firmwareBuildDate, sizeof(info.firmwareBuildDate));
	printf("firmwareBuildDate data: %s\n", info.firmwareBuildDate);

	writeData(info.memoryStatus.total, sizeof(info.memoryStatus.total)); // Send memory status struct
	printf("memoryStatus.total data: %s\n", info.memoryStatus.total);

	writeData(info.memoryStatus.available, sizeof(info.memoryStatus.available)); // Send memory status struct
	printf("memoryStatus.available data: %s\n", info.memoryStatus.available);

	writeData(info.memoryStatus.used, sizeof(info.memoryStatus.used)); // Send memory status struct
	printf("memoryStatus.used data: %s\n", info.memoryStatus.used);

	writeData(info.internalStorageStatus.totalSpace, sizeof(info.internalStorageStatus.totalSpace)); // Send internal storage status struct
	printf("internalStorageStatus.totalSpace data: %s\n", info.internalStorageStatus.totalSpace);

	writeData(info.internalStorageStatus.availableSpace, sizeof(info.internalStorageStatus.availableSpace)); // Send internal storage status struct
	printf("internalStorageStatus.availableSpace data: %s\n", info.internalStorageStatus.availableSpace);

	writeData(info.internalStorageStatus.usedSpace, sizeof(info.internalStorageStatus.usedSpace)); // Send internal storage status struct
	printf("internalStorageStatus.usedSpace data: %s\n", info.internalStorageStatus.usedSpace);

	writeData(info.usbStorageStatus.totalSpace, sizeof(info.usbStorageStatus.totalSpace));
	printf("usbStorageStatus.totalSpace data: %s\n", info.usbStorageStatus.totalSpace);

	writeData(info.usbStorageStatus.availableSpace, sizeof(info.usbStorageStatus.availableSpace));
	printf("usbStorageStatus.availableSpace data: %s\n", info.usbStorageStatus.availableSpace);

	fs.key = OKAY;
	writeData(&fs, 4);
#endif
	return 0;
}

int USBFunctionFS::handleFactoryResetCommand()
{
#if defined ( JUCE_LINUX ) || defined( __linux__ )
	IgniteSettings::GetAppSettingsInstance()->startFactoryReset();
#endif
	return 0;
}

int USBFunctionFS::handleCommand(const juce::String& command)
{
	union FSdata  fs = { 0 };;
	usbmsg_t data = { U_FAILED, 0, 0 };
	static size_t tfilesize = 0;
	static size_t dfilesize = 0;
	std::ifstream file;
	static int i;
#if defined ( JUCE_LINUX ) || defined( __linux__ )
	int flags;
	struct timeval tv;
	fd_set rfds;
	tv.tv_sec = 0;
	tv.tv_usec = 50000;
#endif
	const char* buff = command.toRawUTF8();
	if (strncmp(buff, "Ack:", 4) == 0)
	{
		fs.key = OKAY;
#if defined ( JUCE_LINUX ) || defined( __linux__ )
		if (fw_update_status == DEV_OP_ONGOING)
		{
			auto status = (int)frontPannelManager->getDevOpStatus();
			DBG("dev_status ---: " << status);
			std::string fsBuff = "?" + std::to_string(status);
			std::strncpy((char*)fs.data, fsBuff.c_str(), sizeof(fs.data) - 1);
			fs.data[sizeof(fs.data) - 1] = '\0'; // Ensure null-termination
			fs.key = OKAY;
			writeData(&fs, sizeof(fs));
		}
		else
		{
			// Send the result back
			writeData(&fs, 4);
		}
#endif
	}
	else if (strncmp(buff, "PC_OPERATION_START", 18) == 0)
	{
		DBG("PC app started comm");
		frontPannelManager->engine.SendResponse((moduleId_t)ModuleName::moduleFrontPannel, (moduleId_t)ModuleName::moduleUI, (paramId_t)FrontPannelParam::paramKey_DevStatus, (int)DEV_STATUS::PLAYTRIBE_OP_STARTED);
		// Send the result back
		fs.key = OKAY;
		writeData(&fs, 4);
	}
	else if (strncmp(buff, "PC_OPERATION_END", 16) == 0)
	{
		DBG("PC app stoped comm");
		frontPannelManager->engine.SendResponse((moduleId_t)ModuleName::moduleFrontPannel, (moduleId_t)ModuleName::moduleUI, (paramId_t)FrontPannelParam::paramKey_DevStatus, (int)DEV_STATUS::PLAYTRIBE_OP_COMPLETED);
		// Send the result back
		fs.key = OKAY;
		writeData(&fs, 4);
	}
	else if (strncmp(buff, "DEV_RESTART", 11) == 0)
	{ //handle DEV_RESTART cmd
		DBG("PC app sent Restart cmd");
		frontPannelManager->rebootDevice();
		// Send the result back
		fs.key = OKAY;
		writeData(&fs, 4);
	}
	else if (strncmp(buff, "SAVE_BACKUP", 11) == 0)
	{
		DBG("PC app sent save backup cmd");
		std::string cmd = "rm -f /data/ignite/Ignite_Files/backup* ; tar -cvf /data/ignite/Ignite_Files/backup.tar /data/ignite/Ignite_Files/Ignite_folder_backup; openssl aes-256-cbc -a -salt -pbkdf2 -in /data/ignite/Ignite_Files/backup.tar -out /data/ignite/Ignite_Files/backup.img -k 'MTBTMIIGNITE'; rm -f /data/ignite/Ignite_Files/backup.tar";

		frontPannelManager->engine.SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::ParamEngineCreateDataBackup, (uint8_t)EngineDataFolderId::AllDataInternalStorage, false);
		system(cmd.c_str());

		fs.key = OKAY;
		writeData(&fs, 4);
	}
	else if (strncmp(buff, "LOAD_BACKUP", 11) == 0)
	{
		DBG("PC app sent load backup cmd");
		std::string cmd = "openssl aes-256-cbc -a -d -salt -pbkdf2 -in /data/ignite/Ignite_Files/backup.img -out /data/ignite/Ignite_Files/backup.tar -k 'MTBTMIIGNITE'; tar -xvf /data/ignite/Ignite_Files/backup.tar; rm -f /data/ignite/Ignite_Files/backup*";
		system(cmd.c_str());

		frontPannelManager->engine.SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::ParamEngineRestoreDataBackup, (uint8_t)EngineDataFolderId::AllDataInternalStorage, false);

		fs.key = OKAY;
		writeData(&fs, 4);
	}
	else if (strncmp(buff, "SYSTEM_LOG_CMD", 14) == 0)
	{
		DBG("PC app sent SYSTEM_LOG_CMD");
		std::string cmd = "tar -cvf /data/ignite/logs.tar /var/log/; openssl aes-256-cbc -a -salt -pbkdf2 -in /data/ignite/logs.tar -out /data/ignite/logs.img -k 'MTBTMIIGNITE'; rm -f /data/ignite/logs.tar";
		system(cmd.c_str());

		fs.key = OKAY;
		writeData(&fs, 4);
	}
	else if (strncmp(buff, "WFile:", 6) == 0)
	{
		DBG("IN Wfile Command");
		const char* ptr = buff + 6; // Move the pointer to skip "WFile:"

		// Find the position of ':' after filename
		const char* colonPtr = strchr(ptr, ':');
		if (colonPtr == nullptr)
		{
			DBG("Invalid command format: Missing ':'");
			// Handle the error or return
			return -1;
		}

		// Extract the filename
		std::string filename(ptr, colonPtr - ptr);

		// Extract the size
		std::string sizeStr(colonPtr + 1); // Points to the character after ':'
		std::size_t size = std::stoul(sizeStr);

#if defined( JUCE_LINUX ) || defined( __linux__ )
		// Open the file for writing
		write_filefd = open(filename.c_str(), O_WRONLY | O_CREAT,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
#endif

		if (write_filefd < 0)
			fs.key = FAIL;
		else
			fs.key = OKAY;

		// Send the result back
		writeData(&fs, 4);
		tfilesize = size;
		data.totalfilesize = tfilesize;
		data.command_status = USB_STATUS::U_INIT;
		data.downloadfilesize = 0;

		if (usbCallback)
		{
			usbCallback(data);
		}
	}
	else if (strncmp(buff, "Wdata:", 6) == 0) //download
	{
		DBG("In Wdata Command ");
#if( JUCE_LINUX || __linux__ )
		uint32_t data_size; /* download:0x100000 */
		int      rs;
		int      ret = 0;
		uint32_t status = OKAY;
		data_size = strtoul(buff + 6, NULL, 16);

		void* p = malloc(data_size);
		if (p)
		{
			fs.key = DATA;
		}
		else
		{
			fs.key = FAIL;
			writeData(&fs, 4);
			return -1;
		}
		sprintf((char*)(fs.data), "%08X", data_size);
		writeData(&fs, 4 + strlen((char*)fs.data));

		data.command_status = USB_STATUS::U_DOWNLOAD;
		dfilesize = dfilesize + data_size;
		data.downloadfilesize = dfilesize;

		if (tfilesize)
		{
			data.percentage = (static_cast<double>(dfilesize) / tfilesize) * 100;
		}
		DBG("PERCENTAGE " << juce::String(data.percentage));
		if (usbCallback)
		{
			usbCallback(data);
		}
		if (read(g_ep_read, p, data_size) < 0)
			status = FAIL;

		ret = writeFile(write_filefd, p, data_size);
		if (ret < 0)
			status = FAIL;

		free(p);

		memset(&fs, 0, sizeof(fs));
		if (stdoutfd >= 0)
		{
			flags = fcntl(stdoutfd, F_GETFL);
			flags |= O_NONBLOCK;
			if (fcntl(stdoutfd, F_SETFL, flags))
			{
				DBG("fctl failure\n");
				return -1;
			}
			while (((rs = read(stdoutfd, fs.data, MAX_FRAME_DATA_SIZE)) > 0) && threadShouldExit())
			{
				fs.key = INFO;
				writeData(&fs, rs + 4);
			}
		}
		fs.key = status;
		if (ret == -EPIPE)
		{
			strcpy((char*)fs.data, "EPIPE");
			writeData(&fs, 4 + strlen("EPIPE"));
		}
		else
		{
			writeData(&fs, 4);
		}
#endif
	}
	else if (strncmp(buff, "Closefile", 9) == 0) //close
	{
		DBG("In Closefile Command ");
#if( JUCE_LINUX || __linux__ )
		close(write_filefd);
		close(read_filefd);
		read_filefd = -1;
		write_filefd = -1;
		fs.key = OKAY;
		writeData(&fs, 4);
#endif
		data.command_status = USB_STATUS::U_DONE;
		data.percentage = 100.0;
		data.downloadfilesize = data.totalfilesize;
		if (usbCallback)
		{
			usbCallback(data);
		}
		data.percentage = 0; // resetting to zero for next file transfer
	}
	else if (strncmp(buff, "FirmwareComplete", 16) == 0)
	{
#if( JUCE_LINUX || __linux__ )
		DBG("In FirmwareComplete Command ");
		close(write_filefd);
		write_filefd = -1;
		close(read_filefd);
		read_filefd = -1;

		int rz = 0;

		deviceStatus = IgniteDeviceStatus::GetDevStatusInstance();
		FwStatus_t status = deviceStatus->startFirmwareUpgrade(SwUdateSource_t::PC);
		std::string fwStatus = fwstatusToString(status);
		DBG("fwStatus ---: " << fwStatus);
		std::string fsBuff = ":" + fwStatus;
		std::strncpy((char*)fs.data, fsBuff.c_str(), sizeof(fs.data) - 1);
		fs.data[sizeof(fs.data) - 1] = '\0'; // Ensure null-termination
		fs.key = OKAY;
		writeData(&fs, sizeof(fs));
#endif
		data.command_status = USB_STATUS::U_DONE;
		data.percentage = 100.0;
		data.downloadfilesize = data.totalfilesize;
		if (usbCallback)
		{
			usbCallback(data);
		}
		data.percentage = 0; // resetting to zero for next file transfer
	}
	else if (strncmp(buff, "ReadFile:", 9) == 0)
	{
#if( JUCE_LINUX || __linux__ )
		printf("ROpen:%s\n", buff + 9);
		size_t size = 0;
		struct stat st;
		i = 0;
		int rz = 4;
		const char* filePath = buff + 9;
		std::string path = filePath;
		rFilePath = path;

		printf("file received :%s\n", filePath);
		file = std::ifstream(path, std::ios::binary);
		size = calculateSizeofFile(&file);
		memset(&st, 0, sizeof(st));
		stat(filePath, &st);
		size = st.st_size;
		sprintf((char*)fs.data, "%016zX", size);
		rz = 4 + strlen((char*)fs.data);
		printf("filesize:%d\n", size);

		if (!file.is_open())
		{
			fs.key = FAIL;
			DBG("file doesn't exists");
		}
		else
		{
			DBG("file exists");
			file.seekg(0, std::ios::beg);
			file.close();
			fs.key = OKAY;
			writeData(&fs, rz);
		}

#endif

	}
	else if (strncmp(buff, "UCmd:", 5) == 0)
	{
#if defined  (JUCE_LINUX) || defined(__linux__)
		int pid;
		int out;
		int in;
		int size;
		int p;
		int pstat;

		printf("run shell cmd: %s\n", buff + 5);

		pid = popen2(buff + 5, NULL, &out);
		if (pid < 0) {
			printf("Failure excecu cmd: %s\n", buff + 5);
			memset(&fs, 0, sizeof(fs));
			fs.key = FAIL;
			strcpy((char*)fs.data, "Failure to folk process");
			return -1;
		}
		memset(&fs, 0, sizeof(fs));

		flags = fcntl(out, F_GETFL);
		flags |= O_NONBLOCK;
		if (fcntl(out, F_SETFL, flags)) {
			printf("fctl failure\n");
			return -1;
		}

		FD_ZERO(&rfds);
		FD_SET(out, &rfds);
		do {
			int retval;
			p = waitpid(pid, &pstat, WNOHANG);
			retval = select(out + 1, &rfds, NULL, NULL, &tv);
			do {
				size = read(out, fs.data, MAX_FRAME_DATA_SIZE);
				if (size >= 0) {
					fs.key = INFO;
					writeData(&fs, size + 4);
				}
			} while (size == MAX_FRAME_DATA_SIZE);

			fs.key = INFO;
			writeData(&fs, 4);

		} while (p == 0);

		fs.key = WEXITSTATUS(pstat) ? FAIL : OKAY;
		writeData(&fs, 4);

		close(out);
#endif
	}
	else if (strncmp(buff, "upload", 6) == 0)
	{

#if defined ( JUCE_LINUX ) || defined( __linux__ )
		if (i == 0)
		{
			read_filefd = open(rFilePath.c_str(), O_RDONLY);
		}
		i++;
		int max = 0x10000;
		char buff[max];
		int ret = 0;

		do
		{
			ret = read(read_filefd, buff, max);
			if (ret < 0)
			{
				if (errno == EAGAIN)
				{
					//retry read
					fs.key = DATA;
					sprintf((char*)fs.data, "%08X", 0);
					writeData(&fs, 12);
				}
				else
				{
					fs.key = FAIL;
					writeData(&fs, 4);
					break;
				}
			}
			else
			{
				fs.key = DATA;
				sprintf((char*)fs.data, "%08X", ret);
				writeData(&fs, 12);
				DBG("before sending data to pc.." << juce::String(ret));
				writeData(buff, ret);
				DBG("after sending data to pc..");
				fs.key = OKAY;
				writeData(&fs, 4);
				break;
			}
		} while (1);
#endif
	}
	else if (strncmp(buff, "DeviceInfo:", 11) == 0)
	{
		handleDeviceInfoCommand();
	}
	else if (strncmp(buff, "FactoryReset", 12) == 0)
	{
		handleFactoryResetCommand();
	}
	else if (strncmp(buff, "CREATE_DIR:", 11) == 0)
	{
		handleCreateDirectoryCommand(buff, fs, (int)tfilesize, data);
	}
	else
	{
		printf("Unknow Cmd Recieved from PC %s\n", buff);
		fs.key = OKAY;
		writeData(&fs, 4);
	}
	return 0;
}

int USBFunctionFS::handleCreateDirectoryCommand(const char* buff,FSdata fs,int tfilesize,usbmsg_t data)
{
	DBG("IN CREATE_DIR Command");
	const char* ptr = buff + 11; // Move the pointer to skip "CREATE_DIR:"

	std::string command_string(ptr);

	filesystem::path file_path(command_string);
	filesystem::path directory_path = file_path.parent_path();

	if (!filesystem::exists(directory_path)) 
	{
		try
		{
			if (filesystem::create_directories(directory_path))
			{
				DBG("Directory created: ");
			}
			else 
			{
				DBG("Failed to create directory: ");
			}
		}
		catch (const filesystem::filesystem_error&) {
			DBG("Filesystem error: ");
		}
	}
	// Find the position of ':' after filename
	const char* colonPtr = strchr(ptr, ':');
	if (colonPtr == nullptr)
	{
		DBG("Invalid command format: Missing ':'");
		// Handle the error or return
		return -1;
	}

	std::string filename(ptr, colonPtr - ptr); 	// Extract the filename
	std::string sizeStr(colonPtr + 1); // Extract the size., Points to the character after ':'
	std::size_t size = std::stoul(sizeStr);

#if defined( JUCE_LINUX ) || defined( __linux__ )
	// Open the file for writing
	write_filefd = open(filename.c_str(), O_WRONLY | O_CREAT,
		S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
#endif

	if (write_filefd < 0)
		fs.key = FAIL;
	else
		fs.key = OKAY;

	// Send the result back
	writeData(&fs, 4);
	tfilesize = (int)size;
	data.totalfilesize = tfilesize;
	data.command_status = USB_STATUS::U_INIT;
	data.downloadfilesize = 0;

	if (usbCallback)
	{
		usbCallback(data);
	}
}

size_t USBFunctionFS::calculateSizeofFile(std::ifstream* file)
{
	size_t size;
	file->seekg(0, std::ios::end);
	size = file->tellg();
	file->seekg(0, std::ios::beg);
	return size;
}

std::string USBFunctionFS::fwstatusToString(FwStatus_t status)
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
