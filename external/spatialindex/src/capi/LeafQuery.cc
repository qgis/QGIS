/******************************************************************************
 * Project:  libsidx - A C API wrapper around libspatialindex
 * Purpose:	 C++ objects to implement a query of the index's leaves.
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

LeafQuery::LeafQuery() 
{

}

LeafQueryResult get_results(const SpatialIndex::INode* n)
{
	LeafQueryResult result (n->getIdentifier());

	SpatialIndex::IShape* ps;
	n->getShape(&ps);
	SpatialIndex::Region* pr = dynamic_cast<SpatialIndex::Region*>(ps);
	std::vector<SpatialIndex::id_type> ids;
	for (uint32_t cChild = 0; cChild < n->getChildrenCount(); cChild++)
	{
		ids.push_back(n->getChildIdentifier(cChild));
	}

	result.SetIDs(ids);
	result.SetBounds(pr);
    delete ps;
	
	return result;
}
void LeafQuery::getNextEntry(	const SpatialIndex::IEntry& entry, 
								SpatialIndex::id_type& nextEntry, 
								bool& hasNext) 
{

	const SpatialIndex::INode* n = dynamic_cast<const SpatialIndex::INode*>(&entry);

	if (n != 0)
	{
		// traverse only index nodes at levels 2 and higher.
		if (n->getLevel() > 0)
		{
			for (uint32_t cChild = 0; cChild < n->getChildrenCount(); cChild++)
			{
				m_ids.push(n->getChildIdentifier(cChild));
			}
		}

		if (n->isLeaf())
		{
			m_results.push_back(get_results(n));
		}
	}
			
	if (! m_ids.empty())
	{
		nextEntry = m_ids.front(); m_ids.pop();
		hasNext = true;
	}
	else
	{
		hasNext = false;
	}	 
}


std::vector<SpatialIndex::id_type> const& LeafQueryResult::GetIDs() const
{
    return ids;
}

void LeafQueryResult::SetIDs(std::vector<SpatialIndex::id_type>& v) 
{
    ids.resize(v.size());
    std::copy(v.begin(), v.end(), ids.begin());
}
const SpatialIndex::Region*  LeafQueryResult::GetBounds() const
{
    return bounds;
}

void LeafQueryResult::SetBounds(const SpatialIndex::Region*  b) 
{
    bounds = new SpatialIndex::Region(*b);
}

LeafQueryResult::LeafQueryResult(LeafQueryResult const& other)
{
    ids.resize(other.ids.size());
    std::copy(other.ids.begin(), other.ids.end(), ids.begin());
    m_id = other.m_id;
    
    bounds = other.bounds->clone();
}

LeafQueryResult& LeafQueryResult::operator=(LeafQueryResult const& rhs)
{
    if (&rhs != this)
    {
        ids.resize(rhs.ids.size());
        std::copy(rhs.ids.begin(), rhs.ids.end(), ids.begin());
        m_id = rhs.m_id;
        bounds = rhs.bounds->clone();
    }
    return *this;
}

