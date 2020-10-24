#include "Engine.hpp"

wc::GameEngine* engine = new wc::GameEngine;

int main() {
	wc::Log::Init();
	engine->Start();
	delete engine;
}