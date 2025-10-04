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

#include <stdexcept>
#include <cstring>

#include <spatialindex/SpatialIndex.h>
#include "MemoryStorageManager.h"

using namespace SpatialIndex;
using namespace SpatialIndex::StorageManager;

SpatialIndex::IStorageManager* SpatialIndex::StorageManager::returnMemoryStorageManager(Tools::PropertySet& ps)
{
	IStorageManager* sm = new MemoryStorageManager(ps);
	return sm;
}

SpatialIndex::IStorageManager* SpatialIndex::StorageManager::createNewMemoryStorageManager()
{
	Tools::PropertySet ps;
	return returnMemoryStorageManager(ps);
}

MemoryStorageManager::MemoryStorageManager(Tools::PropertySet&)
{
}

MemoryStorageManager::~MemoryStorageManager()
{
	for (std::vector<Entry*>::iterator it = m_buffer.begin(); it != m_buffer.end(); ++it) delete *it;
}

void MemoryStorageManager::flush()
{
}

void MemoryStorageManager::loadByteArray(const id_type page, uint32_t& len, uint8_t** data)
{
	Entry* e;
	try
	{
		e = m_buffer.at(page);
		if (e == nullptr) throw InvalidPageException(page);
	}
	catch (std::out_of_range&)
	{
		throw InvalidPageException(page);
	}

	len = e->m_length;
	*data = new uint8_t[len];

	memcpy(*data, e->m_pData, len);
}

void MemoryStorageManager::storeByteArray(id_type& page, const uint32_t len, const uint8_t* const data)
{
	if (page == NewPage)
	{
		Entry* e = new Entry(len, data);

		if (m_emptyPages.empty())
		{
			m_buffer.push_back(e);
			page = m_buffer.size() - 1;
		}
		else
		{
			page = m_emptyPages.top(); m_emptyPages.pop();
			m_buffer[page] = e;
		}
	}
	else
	{
		Entry* e_old;
		try
		{
			e_old = m_buffer.at(page);
			if (e_old == nullptr) throw InvalidPageException(page);
		}
		catch (std::out_of_range&)
		{
			throw InvalidPageException(page);
		}

		Entry* e = new Entry(len, data);

		delete e_old;
		m_buffer[page] = e;
	}
}

void MemoryStorageManager::deleteByteArray(const id_type page)
{
	Entry* e;
	try
	{
		e = m_buffer.at(page);
		if (e == nullptr) throw InvalidPageException(page);
	}
	catch (std::out_of_range&)
	{
		throw InvalidPageException(page);
	}

	m_buffer[page] = nullptr;
	m_emptyPages.push(page);

	delete e;
}
