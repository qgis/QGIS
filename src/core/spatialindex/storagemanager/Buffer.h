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

#ifndef __storagemanager_buffer_h
#define __storagemanager_buffer_h

#include <cstring>

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

        virtual void loadByteArray( const long id, unsigned long& len, byte** data );
        virtual void storeByteArray( long& id, const unsigned long len, const byte* const data );
        virtual void deleteByteArray( const long id );

        virtual void clear();
        virtual unsigned long getHits();

      protected:
        class Entry
        {
          public:
            Entry( unsigned long l, const byte* const d ) : m_pData( 0 ), m_length( l ), m_bDirty( false )
            {
              m_pData = new byte[m_length];
              memcpy( m_pData, d, m_length );
            }

            ~Entry() { delete[] m_pData; }

            byte* m_pData;
            unsigned long m_length;
            bool m_bDirty;
        }; // Entry

        virtual void addEntry( long id, Entry* pEntry ) = 0;
        virtual void removeEntry() = 0;

        unsigned long m_capacity;
        bool m_bWriteThrough;
        IStorageManager* m_pStorageManager;
        std::map<long, Entry*> m_buffer;
        unsigned long m_hits;
    }; // Buffer
  }
}

#endif /*__storagemanager_buffer_h*/
