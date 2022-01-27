#pragma once
#include "NonCopyable.hpp"

class Engine : public NonCopyable{
public:
	virtual ~Engine() = default;
	virtual void OnCreate() {}
	virtual void OnUpdate() {}
	virtual void OnEvent() {}
	virtual void OnInput() {}
	virtual void OnDelete() {}
	virtual bool IsEngineOK() { return false; };
	virtual void Start() {
		OnCreate();

		while (IsEngineOK()) {
			// Input handler
			OnInput();
			// Events
			OnEvent();
			// Game Updates
			OnUpdate();
		}

		OnDelete();
	}
};