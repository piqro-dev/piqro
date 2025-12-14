#pragma once

#include <base/common.h>

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
		ASSERT(m_count < N);
		return m_data[m_count++] = v;
	}

	template <typename ... Args>
	inline T& emplace(const Args& ... args)
	{
		ASSERT(m_count < N);
		return m_data[m_count++] = T(args...);
	}

	inline T& pop()
	{
		return m_data[--m_count];
	}

	inline T* begin()
	{
		return m_data;	
	}

	inline T* end()
	{
		return m_data + m_count;
	}	

	inline T* begin() const
	{
		return m_data;	
	}

	inline T* end() const
	{
		return m_data + m_count;
	}	


	inline void trim_end(size_t from)
	{
		m_count = from;
	}

	inline T& remove(size_t at)
	{
		return m_data[at] = pop();
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