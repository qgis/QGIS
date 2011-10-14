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

#pragma once

#include <cstring>

using namespace SpatialIndex;
using namespace SpatialIndex::StorageManager;

namespace SpatialIndex
{
  namespace StorageManager
  {
    class Buffer : public IBuffer
    {
      public:
        Buffer( IStorageManager& sm, Tools::PropertySet& ps );
        // String                   Value     Description
        // ----------------------------------------------
        // Capacity  VT_ULONG Buffer maximum capacity.
        // WriteThrough VT_BOOL Enable or disable write through policy.

        virtual ~Buffer();

        virtual void loadByteArray( const id_type page, uint32_t& len, byte** data );
        virtual void storeByteArray( id_type& page, const uint32_t len, const byte* const data );
        virtual void deleteByteArray( const id_type page );

        virtual void clear();
        virtual uint64_t getHits();

      protected:
        class Entry
        {
          public:
            Entry( uint32_t l, const byte* const d ) : m_pData( 0 ), m_length( l ), m_bDirty( false )
            {
              m_pData = new byte[m_length];
              memcpy( m_pData, d, m_length );
            }

            ~Entry() { delete[] m_pData; }

            byte* m_pData;
            uint32_t m_length;
            bool m_bDirty;
        }; // Entry

        virtual void addEntry( id_type page, Entry* pEntry ) = 0;
        virtual void removeEntry() = 0;

        uint32_t m_capacity;
        bool m_bWriteThrough;
        IStorageManager* m_pStorageManager;
        std::map<id_type, Entry*> m_buffer;
        uint64_t m_u64Hits;
    }; // Buffer
  }
}
