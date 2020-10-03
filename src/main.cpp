#include "Engine.h"

wc::GameEngine* engine = new wc::GameEngine;

int main() {
	engine->Start();
	delete engine;
}