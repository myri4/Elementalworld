#pragma once

#include <string>
#include <filesystem>
#include <wc/Utils/YAML.h>

namespace wc {
	
	class Project {
	public:
		Project() = default;

		void Serialize(const std::string& filepath) {
			YAML::Node config;
			config["Name"] = Name;
			config["Version"] = Version;
			YAMLUtils::saveFile(filepath, config);
		}

		void Deserialize(const std::string& filepath) {
			YAML::Node config = YAML::LoadFile(filepath);

			// TODO: add errors when something doesn't load 
			if (config["Name"]) Name = config["Name"].as<std::string>();
			if (config["Version"]) Version = config["Version"].as<uint32_t>();
		}

		std::string Name = "UnnamedProject";
		uint32_t Version = 0;
	private:
	};

}