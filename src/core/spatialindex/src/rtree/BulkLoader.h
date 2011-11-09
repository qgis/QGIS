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
  namespace RTree
  {
    class ExternalSorter
    {
      public:
        class Record
        {
          public:
            Record();
            Record( const Region& r, id_type id, uint32_t len, byte* pData, uint32_t s );
            ~Record();

            bool operator<( const Record& r ) const;

            void storeToFile( Tools::TemporaryFile& f );
            void loadFromFile( Tools::TemporaryFile& f );

            struct SortAscending : public std::binary_function<Record* const, Record* const, bool>
            {
              bool operator()( Record* const r1, Record* const r2 )
              {
                if ( *r1 < *r2 ) return true;
                else return false;
              }
            };

          public:
            Region m_r;
            id_type m_id;
            byte* m_pData;
            uint32_t m_len;
            uint32_t m_s;
        };

      public:
        ExternalSorter( uint32_t u32PageSize, uint32_t u32BufferPages );
        virtual ~ExternalSorter();

        void insert( Record* r );
        void sort();
        Record* getNextRecord();
        uint64_t getTotalEntries() const;

      private:
        class PQEntry
        {
          public:
            PQEntry( Record* r, uint32_t u32Index ) : m_r( r ), m_u32Index( u32Index ) {}

            struct SortAscending : public std::binary_function<const PQEntry&, const PQEntry&, bool>
            {
              bool operator()( const PQEntry& e1, const PQEntry& e2 )
              {
                if ( *( e1.m_r ) < *( e2.m_r ) ) return true;
                else return false;
              }
            };

            Record* m_r;
            uint32_t m_u32Index;
        };

      private:
        bool m_bInsertionPhase;
        uint32_t m_u32PageSize;
        uint32_t m_u32BufferPages;
        Tools::SmartPointer<Tools::TemporaryFile> m_sortedFile;
        std::list<Tools::SmartPointer<Tools::TemporaryFile> > m_runs;
        std::vector<Record*> m_buffer;
        uint64_t m_u64TotalEntries;
        uint32_t m_stI;
    };

    class BulkLoader
    {
      public:
        void bulkLoadUsingSTR(
          RTree* pTree,
          IDataStream& stream,
          uint32_t bindex,
          uint32_t bleaf,
          uint32_t pageSize, // The number of node entries per page.
          uint32_t numberOfPages // The total number of pages to use.
        );

      protected:
        void createLevel(
          RTree* pTree,
          Tools::SmartPointer<ExternalSorter> es,
          uint32_t dimension,
          uint32_t indexSize,
          uint32_t leafSize,
          uint32_t level,
          Tools::SmartPointer<ExternalSorter> es2,
          uint32_t pageSize,
          uint32_t numberOfPages
        );

        Node* createNode(
          RTree* pTree,
          std::vector<ExternalSorter::Record*>& e,
          uint32_t level
        );
    };
  }
}
