// Spatial Index Library
//
// Copyright (C) 2003 Navel Ltd.
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

#ifndef __spatialindex_h
#define __spatialindex_h

#include <assert.h>
#include <iostream>
#include <vector>
#include <map>
#include <limits>
#include <stack>
#include <queue>
#include <set>
#include <cmath>
#include <cstring>
#include <sstream>

#include "Tools.h"

using namespace Tools::Geometry;

# if !HAVE_BZERO
#  define bzero(d, n) memset((d), 0, (n))
# endif

namespace SpatialIndex
{
  //const std::string VERSION;
  //const std::string DATE;

  enum CommandType
  {
    CT_NODEREAD = 0x0,
    CT_NODEDELETE,
    CT_NODEWRITE
  };

  //
  // Interfaces
  //
interface IEntry : public Tools::IObject
  {
  public:
    virtual long getIdentifier() const = 0;
    virtual void getShape( IShape** out ) const = 0;
    virtual ~IEntry() {}
  }; // IEntry

interface INode : public IEntry, public Tools::ISerializable
  {
  public:
    virtual unsigned long getChildrenCount() const = 0;
    virtual long getChildIdentifier( unsigned long index ) const = 0;
    virtual void getChildShape( unsigned long index, IShape** out ) const = 0;
    virtual unsigned long getLevel() const = 0;
    virtual bool isIndex() const = 0;
    virtual bool isLeaf() const = 0;
    virtual ~INode() {}
  }; // INode

interface IData : public IEntry
  {
  public:
    virtual void getData( unsigned long& len, byte** data ) const = 0;
    virtual ~IData() {}
  }; // IData

interface IDataStream : public Tools::IObjectStream
  {
  public:
    virtual IData* getNext() = 0;
    virtual ~IDataStream() {}
  }; // IDataStream

  interface ICommand
  {
  public:
    virtual void execute( const INode& in ) = 0;
    virtual ~ICommand() {}
  }; // ICommand

  interface INearestNeighborComparator
  {
  public:
    virtual double getMinimumDistance( const IShape& query, const IShape& entry ) = 0;
    virtual double getMinimumDistance( const IShape& query, const IData& data ) = 0;
    virtual ~INearestNeighborComparator() {}
  }; // INearestNeighborComparator

  interface IStorageManager
  {
  public:
    virtual void loadByteArray( const long id, unsigned long& len, byte** data ) = 0;
    virtual void storeByteArray( long& id, const unsigned long len, const byte* const data ) = 0;
    virtual void deleteByteArray( const long id ) = 0;
    virtual ~IStorageManager() {}
  }; // IStorageManager

  interface IVisitor
  {
  public:
    virtual void visitNode( const INode& in ) = 0;
    virtual void visitData( const IData& in ) = 0;
    virtual void visitData( std::vector<const IData*>& v ) = 0;
    virtual ~IVisitor() {}
  }; // IVisitor

  interface IQueryStrategy
  {
  public:
    virtual void getNextEntry( const IEntry& previouslyFetched, long& nextEntryToFetch, bool& bFetchNextEntry ) = 0;
    virtual ~IQueryStrategy() {}
  }; // IQueryStrategy

  interface IStatistics
  {
  public:
    virtual unsigned long getReads() const = 0;
    virtual unsigned long getWrites() const = 0;
    virtual unsigned long getNumberOfNodes() const = 0;
    virtual unsigned long getNumberOfData() const = 0;
    virtual ~IStatistics() {}
  }; // IStatistics

  interface ISpatialIndex
  {
  public:
    virtual void insertData( unsigned long len, const byte* pData, const IShape& shape, long shapeIdentifier ) = 0;
    virtual bool deleteData( const IShape& shape, long shapeIdentifier ) = 0;
    virtual void containsWhatQuery( const IShape& query, IVisitor& v )  = 0;
    virtual void intersectsWithQuery( const IShape& query, IVisitor& v ) = 0;
    virtual void pointLocationQuery( const Tools::Geometry::Point& query, IVisitor& v ) = 0;
    virtual void nearestNeighborQuery( long k, const IShape& query, IVisitor& v, INearestNeighborComparator& nnc ) = 0;
    virtual void nearestNeighborQuery( long k, const IShape& query, IVisitor& v ) = 0;
    virtual void selfJoinQuery( const IShape& s, IVisitor& v ) = 0;
    virtual void queryStrategy( IQueryStrategy& qs ) = 0;
    virtual void getIndexProperties( Tools::PropertySet& out ) const = 0;
    virtual void addCommand( ICommand* in, CommandType ct ) = 0;
    virtual bool isIndexValid() = 0;
    virtual void getStatistics( IStatistics** out ) const = 0;
    virtual ~ISpatialIndex() {}
  }; // ISpatialIndex

  namespace StorageManager
  {
    enum StorageManagerConstants
    {
      EmptyPage = -0x1,
      NewPage = -0x1
    };

  interface IBuffer : public IStorageManager
    {
    public:
      virtual unsigned long getHits() = 0;
      virtual void clear() = 0;
      virtual ~IBuffer() {}
    }; // IBuffer

    extern IStorageManager* returnMemoryStorageManager( Tools::PropertySet& in );
    extern IStorageManager* createNewMemoryStorageManager();

    extern IStorageManager* returnDiskStorageManager( Tools::PropertySet& in );
    extern IStorageManager* createNewDiskStorageManager( std::string& baseName, unsigned long pageSize );
    extern IStorageManager* loadDiskStorageManager( std::string& baseName );

#ifdef _MSC_VER
    // MSVC didn't like the difference in parameter names between declaration
    // definition
    extern IBuffer* returnRandomEvictionsBuffer( IStorageManager& sm, Tools::PropertySet& ps );
#else
    extern IBuffer* returnRandomEvictionsBuffer( IStorageManager& in0, Tools::PropertySet& in1 );
#endif//_MSC_VER
    extern IBuffer* createNewRandomEvictionsBuffer( IStorageManager& in, unsigned int capacity, bool bWriteThrough );
  }

  //
  // Global functions
  //
  extern std::ostream& operator<<( std::ostream&, const ISpatialIndex& );
  extern std::ostream& operator<<( std::ostream&, const IStatistics& );
}

//#include "TimePoint.h"
//#include "TimeRegion.h"
//#include "MovingPoint.h"
//#include "MovingRegion.h"
#include "RTree.h"
//#include "MVRTree.h"
//#include "TPRTree.h"

#endif /*__spatialindex_h*/
