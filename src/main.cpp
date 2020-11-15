#include "Game.hpp"

int main() {
	wc::GameEngine* engine = new wc::GameEngine;
	wc::Log::Init();
	engine->Start();
}