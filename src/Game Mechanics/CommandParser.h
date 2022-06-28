#pragma once

#include <wc/pch.h>
#include <magic_enum.hpp>

namespace wc {

	enum class CommandType { UNKNOWN = -1, textMessage, fly, collide, setTime, setBlock, give, setSpeed, getBlockID, Length };

	CommandType getCommandType(const std::string& text, std::string& args) {
		if (text[0] == '/') {
			uint32_t length = 0;
			for (; length < text.size(); length++)
				if (text[length] == ' ') break;

			std::string buffer;
			buffer.resize(length - 1);

			for (uint32_t i = 1; i < length; i++)
				buffer[i - 1] = text[i];

			if (text.size() != length) {
				args.resize(text.size() - length - 1);
				for (uint32_t i = length + 1; i < text.size(); i++)
					args[i - length - 1] = text[i];
			}

			for (uint32_t i = (uint32_t)CommandType::textMessage; i < (uint32_t)CommandType::Length; i++)
				if (buffer == magic_enum::enum_name((CommandType)i)) return (CommandType)i;

			return CommandType::UNKNOWN;
		}

		return CommandType::textMessage;
	}

	uint32_t getArgumentOffset(const std::string& args, const uint32_t& argumentID = 0) {
		if (args.size() > 0) {
			uint32_t currArgID = 0;
			for (uint32_t i = 0; i < args.size(); i++) {
				if (currArgID == argumentID) return i;
				if (args[i] == ' ')
					currArgID++;
			}
		}
		else WC_ERROR("No arguments provided!");

		return 0;
	}

	int getArgument(const std::string& args, const uint32_t& argumentID = 0) {
		if (args.size() > 0) {

			uint32_t argumentOffset = getArgumentOffset(args, argumentID);

			std::string buffer;
			buffer.resize(argumentOffset);

			for (uint32_t i = argumentOffset; i < args.size(); i++)
				buffer[i - argumentOffset] = args[i];

			return std::stoi(buffer);
		}
		else WC_ERROR("Invalid arguments provided! 0 is assumed.");
		return 0;
	}
}