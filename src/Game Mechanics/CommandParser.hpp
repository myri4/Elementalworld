#pragma once

#include <wc/pch.hpp>
#include <magic_enum.hpp>

namespace wc {

	enum class CommandType { UNKNOWN = -1, textMessage, fly, collide, setTime, Length };

CommandType getCommandType(const std::string& text, std::string& args) {
	if (text[0] == '/') {
		std::string buffer;
		uint32_t length = 0;
		for (; length < text.size(); length++) 
			if (text[length] == ' ') break;		

		buffer.resize(length - 1);

		for (uint32_t i = 1; i < length; i++) 
				buffer[i - 1] = text[i];	
		if (text.size() != length) {
			args.resize(length - 1);
			for (uint32_t i = length + 1; i < text.size(); i++)
				args[i - length - 1] = text[i];
		}

		for (uint32_t i = (uint32_t)CommandType::textMessage; i < (uint32_t)CommandType::Length; i++)
			if (buffer == magic_enum::enum_name((CommandType)i)) return (CommandType)i;

		return CommandType::UNKNOWN;
	}

	return CommandType::textMessage;
}

int getArgs(const std::string& args, const uint32_t& argumentID) {
	if (args.size() > 0) {
		if (argumentID > 0) {
			uint32_t argumentOffset;
			{
				std::string buffer;
				uint32_t length = 0;
				uint32_t argID = 0;
				for (; length < args.size(); length++) {
					if (argID == argumentID) break;
					if (args[length] == ' ')
						argID++;

				}
				argumentOffset = length + 1;
			}
			std::string buffer;

			buffer.resize(args.length() - argumentOffset);

			for (uint32_t i = argumentOffset; i < args.length(); i++)
				buffer[i - argumentOffset] = args[i];

			return std::stoi(buffer);
		}
		return std::stoi(args);
	}
	WC_ERROR("No valid arguments!");
	return 0;
}

}