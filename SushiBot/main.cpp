#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <algorithm>

#include "Bot.h"

#pragma comment(lib, "Ws2_32.lib")

#define PORT 6667
#define IP "72.70.182.240"
#define BUFFER_SIZE 512

int main() {
	WSADATA wsadata; 

	int result;

	result = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (result != NO_ERROR) {
		std::cout << "WSAStartup failed: " << result << std::endl;
		WSACleanup();
		return 1;
	}

	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		std::cout << "Error at socket: " << WSAGetLastError() << std::endl;
		WSACleanup();
	}

	SOCKADDR_IN sockaddr;
	sockaddr.sin_port = htons(PORT);
	sockaddr.sin_family = AF_INET;
	inet_pton(AF_INET, IP, &sockaddr.sin_addr);

	std::cout << "here?" << std::endl;

	result = connect(sock, (SOCKADDR*) &sockaddr, sizeof(sockaddr));
	if (result != 0) {
		std::cout << "Unable to connect to server: " << WSAGetLastError() << std::endl;
		closesocket(sock);
		WSACleanup();
		getchar();
		return 1;
	}

	Sleep(250);

	Bot sushiBot("SushiBot", "#lounge", sock);
	
	sushiBot.add_command(
		"info",
		"show general info about SushiBot",
		0,
		0,
		[](std::vector<std::string> args, std::string user, Bot& bot) {
			bot.privmsg("SushiBot is a bot written in C++ by the great almightly god-like being that is undoall. It's pretty shit.");
		});

	sushiBot.add_command(
		"help",
		"show commands and their descriptions",
		0,
		0,
		[](std::vector<std::string> args, std::string user, Bot& bot) {
			for (std::pair<std::string, std::string> name_and_desc : bot.get_command_descriptors()) {
				bot.privmsg(name_and_desc.first + " - " + name_and_desc.second + ".");
			}
		});

	sushiBot.add_command(
		"slap",
		"slap someone with a large trout",
		1,
		1,
		[](std::vector<std::string> args, std::string user, Bot& bot) {
			bot.action("slaps " + args[0] + " with a large trout.");
		});

	sushiBot.add_command(
		"mix",
		"mix two or more drinks",
		2,
		[](std::vector<std::string> args, std::string user, Bot& bot) {
			std::string action = "skillfully mixes ";

			for (int i = 0; i < args.size(); ++i) {
				if (i == args.size() - 2) {
					action.append(args[i] + " and " + args[i + 1]);
					break;
				} else {
					action.append(args[i] + ", ");
				}
			}

			action.append(" and slides it to " + user + ".");
			bot.action(action);
		});

	sushiBot.add_command(
		"kill",
		"kill someone",
		1,
		1,
		[](std::vector<std::string> args, std::string user, Bot& bot) {
			std::string who = args[0];

			std::transform(who.begin(), who.end(), who.begin(), ::tolower);
			std::transform(user.begin(), user.end(), user.begin(), ::tolower);

			if (who == "itself" || who == "himself" || who == "herself" || who == "self" || who == "yourself" || who == "themselves")
				bot.privmsg("Hey, fuck you too buddy.");
			else if (who == "me" || who == user)
				bot.privmsg("I would link to a suicide hotline, but considering the fact that you're trying to use an IRC bot to kill yourself, I'm not too worried.");
			else
				bot.privmsg("If a shitty IRC bot coded in C++ could kill " + args[0] + ", then someone would've already done it by now.");
		});

	sushiBot.run();

	return 0;
}
