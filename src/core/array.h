#pragma once

#include <core/core.h>

template <typename T, size_t N>
struct Array
{
public:
	inline void init()
	{
		m_count = 0;
	}

	inline T& push(const T& v = {})
	{
		return m_data[m_count++] = v;
	}

	inline T& pop()
	{
		return m_data[--m_count];
	}

	inline T& last()
	{
		return m_data[m_count - 1];
	}	

	inline void trim_end(size_t from)
	{
		m_count = from;
	}

	inline T& remove(size_t at)
	{
		m_data[at] = pop();
	}

	inline size_t count()
	{
		return m_count;
	}

	inline T& operator[](size_t idx)
	{
		return m_data[idx];
	}

private:
	T m_data[N];
	size_t m_count;
};