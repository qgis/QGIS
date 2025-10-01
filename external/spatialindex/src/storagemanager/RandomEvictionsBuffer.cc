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
#include <ctime>
#include <cstdlib>
#include <cmath>

#ifndef HAVE_SRAND48
#include <spatialindex/tools/rand48.h>
#endif

#include <spatialindex/SpatialIndex.h>
#include "RandomEvictionsBuffer.h"

using namespace SpatialIndex;
using namespace SpatialIndex::StorageManager;

IBuffer* SpatialIndex::StorageManager::returnRandomEvictionsBuffer(IStorageManager& sm, Tools::PropertySet& ps)
{
	IBuffer* b = new RandomEvictionsBuffer(sm, ps);
	return b;
}

IBuffer* SpatialIndex::StorageManager::createNewRandomEvictionsBuffer(IStorageManager& sm, uint32_t capacity, bool bWriteThrough)
{
	Tools::Variant var;
	Tools::PropertySet ps;

	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = capacity;
	ps.setProperty("Capacity", var);

	var.m_varType = Tools::VT_BOOL;
	var.m_val.blVal = bWriteThrough;
	ps.setProperty("WriteThrough", var);

	return returnRandomEvictionsBuffer(sm, ps);
}

RandomEvictionsBuffer::RandomEvictionsBuffer(IStorageManager& sm, Tools::PropertySet& ps) : Buffer(sm, ps)
{
	srand48(static_cast<uint32_t>(time(nullptr)));
}

RandomEvictionsBuffer::~RandomEvictionsBuffer()
= default;

void RandomEvictionsBuffer::addEntry(id_type page, Entry* e)
{
	assert(m_buffer.size() <= m_capacity);

	if (m_buffer.size() == m_capacity) removeEntry();
	assert(m_buffer.find(page) == m_buffer.end());
	m_buffer.insert(std::pair<id_type, Entry*>(page, e));
}

void RandomEvictionsBuffer::removeEntry()
{
	if (m_buffer.size() == 0) return;

    double random;

    random = drand48();

	uint32_t entry = static_cast<uint32_t>(floor(((double) m_buffer.size()) * random));

	std::map<id_type, Entry*>::iterator it = m_buffer.begin();
	for (uint32_t cIndex = 0; cIndex < entry; cIndex++) ++it;

	if ((*it).second->m_bDirty)
	{
		id_type page = (*it).first;
		m_pStorageManager->storeByteArray(page, ((*it).second)->m_length, (const uint8_t *) ((*it).second)->m_pData);
	}

	delete (*it).second;
	m_buffer.erase(it);
}
