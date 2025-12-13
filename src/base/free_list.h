#pragma once

#include <base/common.h>

template <typename T, size_t N>
struct FreeList
{
public:
	inline void init()
	{
		m_first_free = static_cast<size_t>(-1);
		m_count = 0;
	}

	inline size_t push(const T& v = {})
	{
		size_t idx;

		if (m_first_free == static_cast<size_t>(-1))
		{
			idx = m_count++;
		} 
		else
		{
			idx = m_first_free;
			m_first_free = m_elements[idx].next_free;
		}

		m_elements[idx].data = v;

		return idx;
	}

	inline void remove(size_t idx)
	{
		m_first_free = idx;
		m_elements[idx].next_free = m_first_free;
	}

	inline T& operator[](size_t idx)
	{
		return m_elements[idx].data;
	}

private:
	union 
	{
		T data;
		size_t next_free;
	} m_elements[N];

	size_t m_count;
	size_t m_first_free;
};