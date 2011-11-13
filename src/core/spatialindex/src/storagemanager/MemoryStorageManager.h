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

namespace SpatialIndex
{
  namespace StorageManager
  {
    class MemoryStorageManager : public SpatialIndex::IStorageManager
    {
      public:
        MemoryStorageManager( Tools::PropertySet& );

        virtual ~MemoryStorageManager();

        virtual void loadByteArray( const id_type page, uint32_t& len, byte** data );
        virtual void storeByteArray( id_type& page, const uint32_t len, const byte* const data );
        virtual void deleteByteArray( const id_type page );

      private:
        class Entry
        {
          public:
            byte* m_pData;
            uint32_t m_length;

            Entry( uint32_t l, const byte* const d ) : m_pData( 0 ), m_length( l )
            {
              m_pData = new byte[m_length];
              memcpy( m_pData, d, m_length );
            }

            ~Entry() { delete[] m_pData; }
        }; // Entry

        std::vector<Entry*> m_buffer;
        std::stack<id_type> m_emptyPages;
    }; // MemoryStorageManager
  }
}
