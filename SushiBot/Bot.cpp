#include "Bot.h"
#include <sstream>
#include <iostream>

std::vector<std::string> split(const std::string& s) {
	std::vector<std::string> elems;
	std::string word;

	std::istringstream iss(s, std::istringstream::in);

	while (iss >> word) {
		elems.push_back(word);
	}

	return elems;
}

Bot::Bot(std::string name, std::string nick, std::string channel, SOCKET sock, bool logging) :
	name(name),
	nick(nick),
	channel(channel),
	sock(sock),
	logging(logging)
{
	add_rule([](std::string s) {
		return !s.compare(0, 4, "PING");
	}, [](std::string s, Bot& bot) {
		bot.send_raw("PONG" + s.substr(4));
	});
	
	add_rule([nick](std::string s) {
		return s.find("MODE") != std::string::npos;
	}, [channel](std::string s, Bot& bot) {
		bot.send_raw("JOIN " + channel + "\n");
	});
}

Bot::Bot(std::string nick, std::string channel, SOCKET sock, bool logging) :
	Bot::Bot(nick, nick, channel, sock, logging) 
{}

void Bot::add_rule(std::function<bool(std::string)> condition, std::function<void(std::string, Bot& bot)> function) {
	Rule rule;
	rule.condition = condition;
	rule.function = function;

	rules.push_back(rule);
}

void Bot::add_command(std::string name,
	std::string description,
	int num_args_from,
	std::function<void(std::vector<std::string>, std::string, Bot& bot)> function) {
	add_command(name, description, num_args_from, -1, function);
}

void Bot::add_command(std::string name,
	std::string description,
	int num_args_from,
	int num_args_to,
	std::function<void(std::vector<std::string>, std::string, Bot& bot)> function) {
	Command comm;
	comm.name = name;
	comm.description = description;
	comm.function = function;
	comm.num_args_from = num_args_from;
	comm.num_args_to = num_args_to;
	commands.push_back(comm);
}

void Bot::send_raw(std::string data) {
	send(sock, data.c_str(), data.length(), 0);
}

void Bot::privmsg(std::string msg) {
	send_raw("PRIVMSG " + channel + " :" + msg + "\n");
}

void Bot::action(std::string action) {
	privmsg("\01ACTION " + action + "\01");
}

std::vector<std::pair<std::string, std::string>> Bot::get_command_descriptors() {
	std::vector<std::pair<std::string, std::string>> ret;

	for (Command comm : commands) {
		ret.push_back(std::make_pair(comm.name, comm.description));
	}

	return ret;
}

std::string get_content(std::string msg) {
	int i = 1;

	while (msg[i++] != ':');

	return msg.substr(i);
}

std::string get_nick(std::string msg) {
	int i = 1;

	while (msg[i++] != '!');
	i -= 2;

	return msg.substr(1, i);
}

bool correct_num_args(int num_args, int min, int max) {
	if (min == -1 && max == -1) {
		return true;
	} else if (min == -1) {
		return num_args <= max;
	} else if (max == -1) {
		return num_args >= min;
	} else {
		return num_args <= max && num_args >= min;
	}
}

std::string english_num_args(int min, int max) {
	if (min == -1 && max == -1) {
		return "any number of arguments";
	} else if (min == -1) {
		return "less than " + std::to_string(max) + " arguments";
	} else if (max == -1) {
		return "more than " + std::to_string(min) + " arguments";
	} else {
		return "between " + std::to_string(min) + " and " + std::to_string(max) + " arguments";
	}
}

void Bot::run() {
	send_raw("USER " + nick + " " + nick + " " + nick + " :" + name + "\n");
	send_raw("NICK " + nick + "\n");

	Sleep(250);

	int nbytes;
	std::string msg;

	do {
		char buff[1024];
		nbytes = recv(sock, buff, 1024, 0);
		msg.assign(buff, nbytes);

		if (logging)
			std::cout << msg;

		std::string content = get_content(msg);

		if (!content.compare(0, nick.length() + 1, "!" + nick)) {
			std::vector<std::string> args = split(content);

			if (args.size() < 2) {
				privmsg("I need a command.");
				continue;
			}

			args.erase(args.begin());
			std::string comm = args[0];
			args.erase(args.begin());

			bool found = false;
			for (int i = 0; i < commands.size(); ++i) {
				if (commands[i].name == comm) {
					if (!correct_num_args(args.size(), commands[i].num_args_from, commands[i].num_args_to)) {
						privmsg("Wrong number of arguments to command " + commands[i].name + 
							" (needed " + english_num_args(commands[i].num_args_from, commands[i].num_args_to) + ", given " + 
							std::to_string(args.size()) + ")\n");
						found = true;
						continue;
					}

					commands[i].function(args, get_nick(msg), *this);
					found = true;
				}
			}

			if (!found) {
				privmsg("Command not found " + comm + "\n");
			}
		} else {
			for (Rule& rule : rules) {
				if (rule.condition(msg)) {
					rule.function(msg, *this);
				}
			}
		}
	} while (nbytes > 0);
}
