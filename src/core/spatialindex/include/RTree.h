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

#ifndef __spatialindex_rtree_h
#define __spatialindex_rtree_h

namespace SpatialIndex
{
  namespace RTree
  {
    enum RTreeVariant
    {
      RV_LINEAR = 0x0,
      RV_QUADRATIC,
      RV_RSTAR
    };

    enum BulkLoadMethod
    {
      BLM_STR = 0x0
    };

    enum PersistenObjectIdentifier
    {
      PersistentIndex = 0x1,
      PersistentLeaf = 0x2
    };

    enum RangeQueryType
    {
      ContainmentQuery = 0x1,
      IntersectionQuery = 0x2
    };

    class Data : public IData, public Tools::ISerializable
    {
      public:
        Data( unsigned long len, byte* pData, Tools::Geometry::Region& r, long id );
        virtual ~Data();

        virtual Data* clone();
        virtual long getIdentifier() const;
        virtual void getShape( IShape** out ) const;
        virtual void getData( unsigned long& len, byte** data ) const;
        virtual unsigned long getByteArraySize();
        virtual void loadFromByteArray( const byte* data );
        virtual void storeToByteArray( byte** data, unsigned long& len );

        long m_id;
        Tools::Geometry::Region m_region;
        byte* m_pData;
        unsigned long m_dataLength;
    }; // Data

#ifdef _MSC_VER
    // MSVC didn't like the difference in parameter names between declaration
    // definition
    extern ISpatialIndex* returnRTree( IStorageManager& sm, Tools::PropertySet& ps );
#else
    extern ISpatialIndex* returnRTree( IStorageManager& in0, Tools::PropertySet& in1 );
#endif//_MSC_VER
    extern ISpatialIndex* createNewRTree(
      IStorageManager& sm,
      double fillFactor,
      unsigned long indexCapacity,
      unsigned long leafCapacity,
      unsigned long dimension,
      RTreeVariant rv,
      long& indexIdentifier
    );
    extern ISpatialIndex* createAndBulkLoadNewRTree(
      BulkLoadMethod m,
      IDataStream& stream,
      IStorageManager& sm,
      double fillFactor,
      unsigned long indexCapacity,
      unsigned long leafCapacity,
      unsigned long dimension,
      RTreeVariant rv,
      long& indexIdentifier
    );
    extern ISpatialIndex* loadRTree( IStorageManager& in, long indexIdentifier );
  }
}

#endif /* __spatialindex_rtree_h */
