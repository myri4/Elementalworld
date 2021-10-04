#pragma once

class NonCopyable
{
public:
	NonCopyable() = default;
	NonCopyable& operator = (const NonCopyable&) = delete;
	NonCopyable(const NonCopyable&) = delete;
};