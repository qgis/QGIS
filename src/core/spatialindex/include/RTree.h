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

#pragma once

namespace SpatialIndex
{
  namespace RTree
  {
    SIDX_DLL enum RTreeVariant
    {
      RV_LINEAR = 0x0,
      RV_QUADRATIC,
      RV_RSTAR
    };

    SIDX_DLL enum BulkLoadMethod
    {
      BLM_STR = 0x0
    };

    SIDX_DLL enum PersistenObjectIdentifier
    {
      PersistentIndex = 0x1,
      PersistentLeaf = 0x2
    };

    SIDX_DLL enum RangeQueryType
    {
      ContainmentQuery = 0x1,
      IntersectionQuery = 0x2
    };

    class SIDX_DLL Data : public IData, public Tools::ISerializable
    {
      public:
        Data( uint32_t len, byte* pData, Region& r, id_type id );
        virtual ~Data();

        virtual Data* clone();
        virtual id_type getIdentifier() const;
        virtual void getShape( IShape** out ) const;
        virtual void getData( uint32_t& len, byte** data ) const;
        virtual uint32_t getByteArraySize();
        virtual void loadFromByteArray( const byte* data );
        virtual void storeToByteArray( byte** data, uint32_t& len );

        id_type m_id;
        Region m_region;
        byte* m_pData;
        uint32_t m_dataLength;
    }; // Data

    SIDX_DLL ISpatialIndex* returnRTree( IStorageManager& ind, Tools::PropertySet& in );
    SIDX_DLL ISpatialIndex* createNewRTree(
      IStorageManager& sm,
      double fillFactor,
      uint32_t indexCapacity,
      uint32_t leafCapacity,
      uint32_t dimension,
      RTreeVariant rv,
      id_type& indexIdentifier
    );
    SIDX_DLL ISpatialIndex* createAndBulkLoadNewRTree(
      BulkLoadMethod m,
      IDataStream& stream,
      IStorageManager& sm,
      double fillFactor,
      uint32_t indexCapacity,
      uint32_t leafCapacity,
      uint32_t dimension,
      RTreeVariant rv,
      id_type& indexIdentifier
    );
    SIDX_DLL ISpatialIndex* createAndBulkLoadNewRTree(
      BulkLoadMethod m,
      IDataStream& stream,
      IStorageManager& sm,
      Tools::PropertySet& ps,
      id_type& indexIdentifier
    );
    SIDX_DLL ISpatialIndex* loadRTree( IStorageManager& in, id_type indexIdentifier );
  }
}
