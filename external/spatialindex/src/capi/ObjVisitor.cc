/******************************************************************************
 * Project:  libsidx - A C API wrapper around libspatialindex
 * Purpose:  C++ objects to implement the object visitor.
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

ObjVisitor::ObjVisitor(): nResults(0)
{
}

ObjVisitor::~ObjVisitor()
{
	std::vector<SpatialIndex::IData*>::iterator it;
	for (it = m_vector.begin(); it != m_vector.end(); it++) {
		delete *it;
	}

}

void ObjVisitor::visitNode(const SpatialIndex::INode& )
{
}

void ObjVisitor::visitData(const SpatialIndex::IData& d)
{

	SpatialIndex::IData* item = dynamic_cast<SpatialIndex::IData*>(const_cast<SpatialIndex::IData&>(d).clone()) ; 
	
	nResults += 1;
	
	m_vector.push_back(item);
}

void ObjVisitor::visitData(std::vector<const SpatialIndex::IData*>& )
{
}

