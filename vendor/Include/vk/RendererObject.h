#pragma once

template <class T>
class RendererObject {
protected:
	T m_RendererID;
public:

	operator T& () { return m_RendererID; }
	operator const T& () const { return m_RendererID; }

	T* GetPointer() { return &m_RendererID; }
	const T* GetPointer() const { return &m_RendererID; }
};