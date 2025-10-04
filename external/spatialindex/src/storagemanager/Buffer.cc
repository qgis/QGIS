/******************************************************************************
 * Project:  libspatialindex - A C++ library for spatial indexing
 * Author:   Marios Hadjieleftheriou, mhadji@gmail.com
 ******************************************************************************
 * Copyright (c) 2002, Marios Hadjieleftheriou
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

#include <cstring>

#include <spatialindex/SpatialIndex.h>
#include "Buffer.h"

Buffer::Buffer(IStorageManager& sm, Tools::PropertySet& ps) :
	m_capacity(10),
	m_bWriteThrough(false),
	m_pStorageManager(&sm),
	m_u64Hits(0)
{
	Tools::Variant var = ps.getProperty("Capacity");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_ULONG) throw Tools::IllegalArgumentException("Property Capacity must be Tools::VT_ULONG");
		m_capacity = var.m_val.ulVal;
	}

	var = ps.getProperty("WriteThrough");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_BOOL) throw Tools::IllegalArgumentException("Property WriteThrough must be Tools::VT_BOOL");
		m_bWriteThrough = var.m_val.blVal;
	}
}

Buffer::~Buffer()
{
	flush();
}

void Buffer::flush()
{
	for (auto it = m_buffer.begin(); it != m_buffer.end(); ++it)
	{
		if ((*it).second->m_bDirty)
		{
			id_type page = (*it).first;
			m_pStorageManager->storeByteArray(page, (*it).second->m_length, (*it).second->m_pData);
		}
		delete (*it).second;
	}
}

void Buffer::loadByteArray(const id_type page, uint32_t& len, uint8_t** data)
{
	auto it = m_buffer.find(page);

	if (it != m_buffer.end())
	{
		++m_u64Hits;
		len = (*it).second->m_length;
		*data = new uint8_t[len];
		memcpy(*data, (*it).second->m_pData, len);
	}
	else
	{
		m_pStorageManager->loadByteArray(page, len, data);
		addEntry(page, new Entry(len, static_cast<const uint8_t*>(*data)));
	}
}

void Buffer::storeByteArray(id_type& page, const uint32_t len, const uint8_t* const data)
{
	if (page == NewPage)
	{
		m_pStorageManager->storeByteArray(page, len, data);
		assert(m_buffer.find(page) == m_buffer.end());
		addEntry(page, new Entry(len, data));
	}
	else
	{
		if (m_bWriteThrough)
		{
			m_pStorageManager->storeByteArray(page, len, data);
		}

		Entry* e = new Entry(len, data);
		if (m_bWriteThrough == false) e->m_bDirty = true;

		auto it = m_buffer.find(page);
		if (it != m_buffer.end())
		{
			delete (*it).second;
			(*it).second = e;
			if (m_bWriteThrough == false) ++m_u64Hits;
		}
		else
		{
			addEntry(page, e);
		}
	}
}

void Buffer::deleteByteArray(const id_type page)
{
	auto it = m_buffer.find(page);
	if (it != m_buffer.end())
	{
		delete (*it).second;
		m_buffer.erase(it);
	}

	m_pStorageManager->deleteByteArray(page);
}

void Buffer::clear()
{
	for (auto it = m_buffer.begin(); it != m_buffer.end(); ++it)
	{
		if ((*it).second->m_bDirty)
		{
			id_type page = (*it).first;
			m_pStorageManager->storeByteArray(page, ((*it).second)->m_length, static_cast<const uint8_t*>(((*it).second)->m_pData));
		}

		delete (*it).second;
	}

	m_buffer.clear();
	m_u64Hits = 0;
}

uint64_t Buffer::getHits()
{
	return m_u64Hits;
}
