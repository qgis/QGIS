/******************************************************************************
 * Project:  libsidx - A C API wrapper around libspatialindex
 * Purpose:	 C++ objects to implement the datastream.
 * Author:   Howard Butler, hobu.inc@gmail.com
 ******************************************************************************
 * Copyright (c) 2009, Howard Butler
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

#include <spatialindex/capi/sidx_impl.h>


DataStream::DataStream(int (*readNext)(SpatialIndex::id_type * id,
					   double **pMin,
					   double **pMax,
					   uint32_t *nDimension,
					   const uint8_t** pData,
					   size_t *nDataLength) )
  : m_pNext(0),
    m_bDoneReading(false)
{
	iterfunct = readNext;

	// Read the first one.
	readData();
}

DataStream::~DataStream()
{
	if (m_pNext != 0) delete m_pNext;
}

bool DataStream::readData()
{
	SpatialIndex::id_type id;
	double *pMin=0;
	double *pMax=0;
	uint32_t nDimension=0;
	uint8_t *p_data=0;
	size_t nDataLength=0;

	if (m_bDoneReading == true) {
		return false;
	}

	int ret = iterfunct(&id, &pMin, &pMax, &nDimension, const_cast<const uint8_t**>(&p_data), &nDataLength);

	// The callback should return anything other than 0
	// when it is done.
	if (ret != 0)
	{
		m_bDoneReading = true;
		return false;
	}

	SpatialIndex::Region r = SpatialIndex::Region(pMin, pMax, nDimension);

	// Data gets copied here anyway. We should fix this part of SpatialIndex::RTree::Data's constructor
	m_pNext = new SpatialIndex::RTree::Data((uint32_t)nDataLength, p_data, r, id);

	return true;
}


SpatialIndex::IData* DataStream::getNext()
{
	if (m_pNext == 0) return 0;

	SpatialIndex::RTree::Data* ret = m_pNext;
	m_pNext = 0;
	readData();
	return ret;
}

bool DataStream::hasNext()
{
	return (m_pNext != 0);
}

uint32_t DataStream::size()
{
	throw Tools::NotSupportedException("Operation not supported.");
}

void DataStream::rewind()
{
	throw Tools::NotSupportedException("Operation not supported.");

/*	if (m_pNext != 0)
	{
	 delete m_pNext;
	 m_pNext = 0;
	}
*/
}
