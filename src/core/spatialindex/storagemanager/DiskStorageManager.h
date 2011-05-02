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

#ifndef __storagemanager_diskstoragemanager_h
#define __storagemanager_diskstoragemanager_h

namespace SpatialIndex
{
  namespace StorageManager
  {
    class DiskStorageManager : public SpatialIndex::IStorageManager
    {
      public:
        DiskStorageManager( Tools::PropertySet& );
        virtual ~DiskStorageManager();

        void flush();

        virtual void loadByteArray( const long id, unsigned long& len, byte** data );
        virtual void storeByteArray( long& id, const unsigned long len, const byte* const data );
        virtual void deleteByteArray( const long id );

      private:
        class Entry
        {
          public:
            unsigned long m_length;
            std::vector<long> m_pages;
        };

        int m_dataFile;
        int m_indexFile;
        unsigned long m_pageSize;
        long m_nextPage;
        std::priority_queue<long, std::vector<long>, std::greater<long> > m_emptyPages;
        std::map<long, Entry*> m_pageIndex;

        unsigned char* m_buffer;
    }; // DiskStorageManager
  }
}

#endif /*__storagemanager_diskstoragemanager_h*/

