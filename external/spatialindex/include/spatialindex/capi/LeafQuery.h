/******************************************************************************
 * Project:  libsidx - A C API wrapper around libspatialindex
 * Purpose:	 C++ object declarations to implement a query of the index's leaves.
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

#pragma once

#include "sidx_export.h"

class LeafQueryResult;

class SIDX_DLL LeafQuery : public SpatialIndex::IQueryStrategy
{
private:
	std::queue<SpatialIndex::id_type> m_ids;
	std::vector<LeafQueryResult> m_results;
public:

	LeafQuery();
	~LeafQuery() { }
	void getNextEntry(	const SpatialIndex::IEntry& entry, 
						SpatialIndex::id_type& nextEntry, 
						bool& hasNext);
	std::vector<LeafQueryResult> const& GetResults() const {return m_results;}
};

class SIDX_DLL LeafQueryResult 
{
private:
    std::vector<SpatialIndex::id_type> ids;
    SpatialIndex::Region* bounds;
    SpatialIndex::id_type m_id;
    LeafQueryResult();
public:
    LeafQueryResult(SpatialIndex::id_type id) : bounds(0), m_id(id){}
    ~LeafQueryResult() {if (bounds!=0) delete bounds;}

    /// Copy constructor.
    LeafQueryResult(LeafQueryResult const& other);

    /// Assignment operator.
    LeafQueryResult& operator=(LeafQueryResult const& rhs);
        
    std::vector<SpatialIndex::id_type> const& GetIDs() const;
    void SetIDs(std::vector<SpatialIndex::id_type>& v);
    const SpatialIndex::Region* GetBounds() const;
    void SetBounds(const SpatialIndex::Region*  b);
    SpatialIndex::id_type getIdentifier() const {return m_id;}
    void setIdentifier(uint32_t v) {m_id = v;}
};
