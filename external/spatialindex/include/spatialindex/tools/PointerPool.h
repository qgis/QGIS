/******************************************************************************
 * Project:  libspatialindex - A C++ library for spatial indexing
 * Author:   Marios Hadjieleftheriou, mhadji@gmail.com
 ******************************************************************************
 * Copyright (c) 2004, Marios Hadjieleftheriou
 *
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
******************************************************************************/

#pragma once

#include "PoolPointer.h"

namespace Tools
{
	template <class X> class PointerPool
	{
	public:
		explicit PointerPool(uint32_t capacity) : m_capacity(capacity)
		{
		}

		~PointerPool()
		{
			assert(m_pool.size() <= m_capacity);

			while (! m_pool.empty())
			{
				X* x = m_pool.top(); m_pool.pop();
				delete x;
			}
		}

		PoolPointer<X> acquire()
		{
			X* p = nullptr;

			if (! m_pool.empty())
			{
				p = m_pool.top(); m_pool.pop();
			}
			else
			{
				p = new X();
			}

			return PoolPointer<X>(p, this);
		}

		void release(X* p)
		{
			if (m_pool.size() < m_capacity)
			{
				m_pool.push(p);
			}
			else
			{
				delete p;
			}

			assert(m_pool.size() <= m_capacity);
		}

		uint32_t getCapacity() const { return m_capacity; }
		void setCapacity(uint32_t c)
		{
			assert (c >= 0);
			m_capacity = c;
		}

	private:
		uint32_t m_capacity;
		std::stack<X*> m_pool;

	};
}

