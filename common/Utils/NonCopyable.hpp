#ifndef NON_COPYABLE_HPP
#define NON_COPYABLE_HPP

class NonCopyable
{
public:
	NonCopyable() = default;
	NonCopyable& operator = (const NonCopyable&) = delete;
	NonCopyable(const NonCopyable&) = delete;
};
#endif 