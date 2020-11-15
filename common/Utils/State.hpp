#pragma once
#include "NonCopyable.hpp"

enum class EngineStatus { OK, FAIL, ENGINE_ERROR, ENGINE_FATAL_ERROR };

class Engine : NonCopyable {
public:
	virtual ~Engine() = default;
	virtual void OnCreate() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnEvent() = 0;
	virtual void OnInput() = 0;
	virtual void OnDelete() = 0;
	virtual EngineStatus GetEngineStatus() = 0;
	virtual void Start() {
		OnCreate();
		while (GetEngineStatus() == EngineStatus::OK) {
			// Game Events
			OnUpdate();
			// Events
			OnEvent();
			// Input handler
			OnInput();
		}
		OnDelete();
	}
};