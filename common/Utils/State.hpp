#ifndef STATE_HPP
#define STATE_HPP
#include "NonCopyable.hpp"

class Engine : public NonCopyable{
public:
	virtual ~Engine() = default;
	virtual void OnCreate() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnEvent() {}
	virtual void OnInput() = 0;
	virtual void OnDelete() = 0;
	virtual bool IsEngineOK() { return false; };
	virtual void Start() {
		OnCreate();

		while (IsEngineOK()) {
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
#endif