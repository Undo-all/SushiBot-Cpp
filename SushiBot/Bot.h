#pragma once

#include <WinSock2.h>

#include <map>
#include <functional>
#include <vector>
#include <cstdint>

class Bot {
private:
	struct Command {
		std::string description;
		std::string name;
		int num_args_from;
		int num_args_to;
		std::function<void(std::vector<std::string>, std::string, Bot& bot)> function;
	};

	struct Rule {
		std::function<bool(std::string)> condition;
		std::function<void(std::string, Bot& bot)> function;
	};

	std::vector<Command> commands;
	std::vector<Rule> rules;
	std::string name;
	std::string nick;
	std::string channel;
	SOCKET sock;
	bool logging;
public:
	Bot(std::string nick, std::string name, std::string channel, SOCKET sock, bool logging = true);
	Bot(std::string nick, std::string channel, SOCKET sock, bool logging = true);
	void add_command(std::string name,
		std::string description,
		int num_args_from,
		std::function<void(std::vector<std::string>, std::string, Bot& bot)> function);
	void add_command(std::string name,
		std::string description,
		int num_args_from,
		int num_args_to,
		std::function<void(std::vector<std::string>, std::string, Bot& bot)> function);
	void add_rule(std::function<bool(std::string)> condition, 
		std::function<void(std::string, Bot& bot)> function);

	void privmsg(std::string msg);
	void action(std::string action);
	void send_raw(std::string data);

	std::vector<std::pair<std::string, std::string>> get_command_descriptors();

	// void connect();
	void run();
};
