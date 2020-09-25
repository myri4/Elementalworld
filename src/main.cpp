#include "Engine.h"

int main() {
	wc::GameEngine* engine = new wc::GameEngine;
	engine->Start();
	delete engine;
}