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

#include "PointerPool.h"

namespace Tools
{
	template <class X> class PointerPool;

	template <class X> class PoolPointer
	{
	public:
		explicit PoolPointer(X* p = nullptr) : m_pointer(p), m_pPool(nullptr) { m_prev = m_next = this; }
		explicit PoolPointer(X* p, PointerPool<X>* pPool) noexcept : m_pointer(p), m_pPool(pPool) { m_prev = m_next = this; }
		~PoolPointer() { release(); }
		PoolPointer(const PoolPointer& p) noexcept { acquire(p); }
		PoolPointer& operator=(const PoolPointer& p)
		{
			if (this != &p)
			{
				release();
				acquire(p);
			}
			return *this;
		}

		X& operator*() const noexcept { return *m_pointer; }
		X* operator->() const noexcept { return m_pointer; }
		X* get() const noexcept { return m_pointer; }
		bool unique() const noexcept { return m_prev ? m_prev == this : true; }
		void relinquish() noexcept
		{
			m_pPool = nullptr;
			m_pointer = nullptr;
			release();
		}

	private:
		X* m_pointer;
		mutable const PoolPointer* m_prev;
		mutable const PoolPointer* m_next;
		PointerPool<X>* m_pPool;

		void acquire(const PoolPointer& p) noexcept
		{
			m_pPool = p.m_pPool;
			m_pointer = p.m_pointer;
			m_next = p.m_next;
			m_next->m_prev = this;
			m_prev = &p;
			#ifndef mutable
			p.m_next = this;
			#else
			(const_cast<linked_ptr<X>*>(&p))->m_next = this;
			#endif
		}

		void release()
		{
			if (unique())
			{
				if (m_pPool != nullptr) m_pPool->release(m_pointer);
				else delete m_pointer;
			}
			else
			{
				m_prev->m_next = m_next;
				m_next->m_prev = m_prev;
				m_prev = m_next = nullptr;
			}
			m_pointer = nullptr;
			m_pPool = nullptr;
		}
	};
}

