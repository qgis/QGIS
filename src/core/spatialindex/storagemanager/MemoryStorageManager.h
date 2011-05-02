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

#ifndef __storagemanager_memorystoragemanager_h
#define __storagemanager_memorystoragemanager_h

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

        virtual void loadByteArray( const long id, unsigned long& len, byte** data );
        virtual void storeByteArray( long& id, const unsigned long len, const byte* const data );
        virtual void deleteByteArray( const long id );

      private:
        class Entry
        {
          public:
            byte* m_pData;
            unsigned long m_length;

            Entry( unsigned long l, const byte* const d ) : m_pData( 0 ), m_length( l )
            {
              m_pData = new byte[m_length];
              memcpy( m_pData, d, m_length );
            }

            ~Entry() { delete[] m_pData; }
        }; // Entry

        std::vector<Entry*> m_buffer;
        std::stack<long> m_emptyPages;
    }; // MemoryStorageManager
  }
}

#endif /*__storagemanager_memorystoragemanager_h*/
