// Spatial Index Library
//
// Copyright (C) 2002 Navel Ltd.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Email:
//    mhadji@gmail.com

#include "SpatialIndexImpl.h"

#include "../rtree/RTree.h"

SpatialIndex::InvalidPageException::InvalidPageException( id_type id )
{
  std::ostringstream s;
  s << "Unknown page id " << id;
  m_error = s.str();
}

std::string SpatialIndex::InvalidPageException::what()
{
  return "InvalidPageException: " + m_error;
}

std::ostream& SpatialIndex::operator<<( std::ostream& os, const ISpatialIndex& i )
{
  const SpatialIndex::RTree::RTree* pRTree = dynamic_cast<const SpatialIndex::RTree::RTree*>( &i );
  if ( pRTree != 0 )
  {
    os << *pRTree;
    return os;
  }

  std::cerr << "ISpatialIndex operator<<: Not implemented yet for this index type." << std::endl;
  return os;
}

std::ostream& SpatialIndex::operator<<( std::ostream& os, const IStatistics& s )
{
  const SpatialIndex::RTree::Statistics* pRTreeStats = dynamic_cast<const SpatialIndex::RTree::Statistics*>( &s );
  if ( pRTreeStats != 0 )
  {
    os << *pRTreeStats;
    return os;
  }

  std::cerr << "IStatistics operator<<: Not implemented yet for this index type." << std::endl;
  return os;
}

