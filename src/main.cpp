#include "Application.hpp"

wc::Application engine;

int main() {
	wc::Log::Init();
	engine.Start();
}