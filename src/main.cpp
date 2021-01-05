#include "Application.hpp"

wc::Application app;

int main() {
	wc::Log::Init();
	app.Start();
	return 0;
}