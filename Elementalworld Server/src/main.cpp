#include <Utils/YAML.hpp>
#include "Server.hpp"

int main()
{
	wc::Log::Init();
	uint32_t port = 60000;
	if (std::filesystem::exists("server.settings")) {
		YAML::Node config = YAML::LoadFile("server.settings");
		if (config["port"]) port = config["port"].as<uint32_t>();
	}
	else {
		YAML::Node config;
		config["port"] = port;

		YAMLUtils::saveFile("server.settings", config);
	}
	wc::Server server(port);
	
	server.Start();

	while (1)
	{
		server.Update(-1, true);
	}
	return 0;
}