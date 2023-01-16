#pragma once

#include <pch.h>
#include <magic_enum.hpp>

namespace wc {

	//@TODO: Make a command Parser class

	enum class CommandType { UNKNOWN = -1, textMessage, fly, collide, setTime, setBlock, give, setSpeed, getBlockID }; // @TODO: remove specific set commands(setTime, setSpeed, etc.) and replace them with string arguments

	CommandType getCommandType(const std::string& text, std::string& args) {
		if (text[0] == '/') {
			int32_t length = text.find(' ');

			std::string cmd; // Bro fuck c++ this should be just = text.substr(1);
			cmd.resize(length - 1);

			for (uint32_t i = 1; i < length; i++)
				cmd[i - 1] = text[i];

			if (length != -1) 
				args = text.substr(length + 1);
			

			for (uint32_t i = (uint32_t)CommandType::textMessage; i < magic_enum::enum_count<CommandType>(); i++)
				if (cmd == magic_enum::enum_name((CommandType)i)) return (CommandType)i;

			return CommandType::UNKNOWN;
		}

		return CommandType::textMessage;
	}

	uint32_t getArgumentOffset(const std::string& args, const uint32_t& argumentID = 0) { // @NOTE: Rework?
		if (args.size() > 0) {
			uint32_t currArgID = 0;
			for (uint32_t i = 0; i < args.size(); i++) {
				if (currArgID == argumentID) return i;
				if (args[i] == ' ')
					currArgID++;
			}
		}
		else 
			WC_ERROR("No arguments provided!");

		return 0;
	}

	int32_t getArgument(const std::string& args, const uint32_t& argumentID = 0) {
		if (args.size() > 0) {

			uint32_t argumentOffset = getArgumentOffset(args, argumentID);

			return std::stoi(args.substr(argumentOffset));
		}
		else 
			WC_ERROR("Invalid arguments provided! 0 is assumed.");
		return 0;
	}

	std::string getStringArgument(const std::string& args, const uint32_t& argumentID = 0) {
		if (args.size() > 0) {

			uint32_t argumentOffset = getArgumentOffset(args, argumentID);

			return args.substr(argumentOffset);
		}
		else 
			WC_ERROR("Invalid arguments provided!");
		return "";
	}
}