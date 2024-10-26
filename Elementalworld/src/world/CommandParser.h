#pragma once

#include <string>
#include <magic_enum.hpp>

namespace wc {

	/* @NOTE: Maybe rethink the command syntax?
	* Example (player namer is myri4):
	* Instead of writing "/set speed myri4 5" maybe instead the syntax should be "--myri4.speed = 5"
	*/

	enum class CommandType { UNKNOWN = -1, textMessage, fly, collide, set, setBlock, give, getBlockID };

	class CommandParser {
		std::string args;
	public:

		CommandType getCommandType(const std::string& text) {
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

		//@TODO: Maybe remove argument id and replace it with a stack system?
		int32_t getArgumentOffset(uint32_t argumentID = 0) {
			uint32_t currArgID = 0;
			for (uint32_t i = 0; i < args.size(); i++) {
				if (currArgID == argumentID) return i;
				if (args[i] == ' ')
					currArgID++;
			}			
			
			return -1;
		}

		std::string getStringArgument(uint32_t argumentID = 0) {
			int32_t argumentOffset = getArgumentOffset(argumentID);
			if (argumentOffset != -1) {
				int32_t end = args.find(' ');
				if (end != -1)
					return args.substr(argumentOffset, end - argumentOffset);
				else
					return args.substr(argumentOffset);
			}

			WC_ERROR("Syntax error: Not enough arguments provided!");
			return "";
		}

		int32_t getArgument(uint32_t argumentID = 0) {
			std::string result = getStringArgument(argumentID);

			if (result.size() > 0) return std::stoi(result);

			return 0;
		}

	};
}