// Tools Library
//
// Copyright (C) 2004  Navel Ltd.
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

#ifndef __tools_external_sort_h
#define __tools_external_sort_h

namespace Tools
{
  class ExternalSort : public IObjectStream
  {
    public:
      ExternalSort(
        Tools::IObjectStream& source,
        unsigned long bufferSize
      );
      ExternalSort(
        Tools::IObjectStream& source,
        Tools::IObjectComparator& comp,
        unsigned long bufferSize
      );
      virtual ~ExternalSort();

      virtual IObject* getNext();
      virtual bool hasNext() throw();
      virtual unsigned long size() throw();
      virtual void rewind();

    private:
      class PQEntry
      {
        public:
          PQEntry(
            ISerializable* pS,
            IObjectComparator* pC,
            SmartPointer<TemporaryFile>& f
          );
          ~PQEntry();

          struct ascendingComparator
                : public std::binary_function <
                PQEntry*, PQEntry*, bool
                >
          {
            bool operator()(
              PQEntry* x,
              PQEntry* y
            ) const;
          };

          ISerializable* m_pRecord;
          IObjectComparator* m_pComparator;
          SmartPointer<TemporaryFile> m_spFile;
      };

      void initializeRuns( std::deque<SmartPointer<TemporaryFile> >& runs );
      void mergeRuns();

      std::priority_queue <
      PQEntry*,
      std::vector<PQEntry*>,
      PQEntry::ascendingComparator > m_buffer;

      unsigned long m_cMaxBufferSize;
      bool m_bFitsInBuffer;
      unsigned long m_cNumberOfSortedRecords;
      unsigned long m_cNumberOfReturnedRecords;

      IObjectStream* m_pExternalSource;
      IObject* m_pTemplateRecord;
      IObjectComparator* m_pComparator;
      SmartPointer<TemporaryFile> m_spSortedFile;
  }; // ExternalSort
}

#endif /* __tools_external_sort_h */
