#include "Engine.h"

#include <irrKlang/irrKlang.h>

irrklang::ISoundEngine* SoundEngine = irrklang::createIrrKlangDevice();

int main() {
	SoundEngine->play2D("assets/sounds/Alan Walker - The Spectre_wJnBTPUQS5A_youtube.mp3");
	wc::GameEngine* engine = new wc::GameEngine;
	engine->Start();
	delete engine;
}