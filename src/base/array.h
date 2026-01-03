#pragma once

#include <base/common.h>

template <typename T, size_t N>
struct Array
{
public:
	constexpr inline Array()
		: m_count(0) {}

	inline T& push(const T& v)
	{
		return m_buffer[m_count++] = v;
	}

	inline T& push()
	{
		return m_buffer[m_count++];
	}

	template <typename ... Args>
	inline T& emplace(const Args& ... args)
	{
		return m_buffer[m_count++] = T(args...);
	}

	inline T& pop()
	{
		return m_buffer[--m_count];
	}

	inline T* begin()
	{
		return m_buffer;	
	}

	inline T* end()
	{
		return m_buffer + m_count;
	}	

	inline T* begin() const
	{
		return m_buffer;	
	}

	inline T* end() const
	{
		return m_buffer + m_count;
	}	

	inline void trim_end(size_t from)
	{
		m_count = from;
	}

	inline T& remove(size_t at)
	{
		return m_buffer[at] = pop();
	}

	inline size_t count()
	{
		return m_count;
	}

	inline T& operator[](size_t idx)
	{
		return m_buffer[idx];
	}

private:
	T m_buffer[N];
	size_t m_count;
};