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

        virtual void loadByteArray( const id_type page, uint32_t& len, byte** data );
        virtual void storeByteArray( id_type& page, const uint32_t len, const byte* const data );
        virtual void deleteByteArray( const id_type page );

      private:
        class Entry
        {
          public:
            uint32_t m_length;
            std::vector<id_type> m_pages;
        };

        std::fstream m_dataFile;
        std::fstream m_indexFile;
        uint32_t m_pageSize;
        id_type m_nextPage;
        std::priority_queue<id_type, std::vector<id_type>, std::greater<id_type> > m_emptyPages;
        std::map<id_type, Entry*> m_pageIndex;

        byte* m_buffer;
    }; // DiskStorageManager
  }
}
