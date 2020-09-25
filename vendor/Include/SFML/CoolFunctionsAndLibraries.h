#pragma once
#include <SFML/SFML.hpp>

void error(std::string error) {
	std::cout << error << std::endl;
	std::cout << "Press enter to exit..." << std::endl;
	std::cin.ignore(10000, '\n');
	exit(1);
}