#pragma once

class NonCopyable
{
public:
	NonCopyable() = default;
	NonCopyable& operator = (const NonCopyable&) = delete;
	NonCopyable(const NonCopyable&) = delete;
};

enum class EngineStatus {OK, FAIL, ENGINE_ERROR, FILECANTBELOCATED};

class Engine : NonCopyable {
public:
	virtual ~Engine() = default;
	virtual void OnCreate() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnEvent() = 0;
	virtual void OnInput() = 0;
	virtual EngineStatus GetEngineStatus() = 0;
	virtual void Start() {
		OnCreate();
		while (EngineStatus() == EngineStatus::OK)
		{
			OnEvent();
			OnUpdate();
		}
	}
};