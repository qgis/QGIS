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

#include <stdio.h>

#include <Tools.h>

#include "ExternalSort.h"
#include "qgslogger.h"

#ifdef _MSC_VER
#define UNUSED(symbol) symbol
#else
#define UNUSED(symbol)
#endif

using namespace std;

Tools::ExternalSort::PQEntry::PQEntry(
  ISerializable* pS,
  IObjectComparator* pC,
  SmartPointer<TemporaryFile>& f )
    : m_pRecord( pS ), m_pComparator( pC ), m_spFile( f )
{
}

Tools::ExternalSort::PQEntry::~PQEntry()
{
  delete m_pRecord;
}

bool Tools::ExternalSort::PQEntry::ascendingComparator::operator()(
  PQEntry* x,
  PQEntry* y
) const
{
  if ( x->m_pComparator != 0 )
  {
    int ret = x->m_pComparator->compare(
                dynamic_cast<IObject*>( x->m_pRecord ),
                dynamic_cast<IObject*>( y->m_pRecord ) );

    if ( ret == 1 ) return true;
    return false;
  }
  else
  {
    IComparable* pX = dynamic_cast<IComparable*>( x->m_pRecord );
    IComparable* pY = dynamic_cast<IComparable*>( y->m_pRecord );

    if ( pX == 0 || pY == 0 )
      throw IllegalArgumentException(
        "Tools::ExternalSort::PQEntry::ascendingComparator: "
        "argument is not comparable."
      );

    if ( *pX > *pY ) return true;
    return false;
  }
}

Tools::ExternalSort::ExternalSort(
  IObjectStream& source,
  unsigned long bufferSize
)
    : m_cMaxBufferSize( bufferSize ),
    m_bFitsInBuffer( false ),
    m_cNumberOfSortedRecords( 0 ),
    m_cNumberOfReturnedRecords( 0 ),
    m_pExternalSource( &source ),
    m_pTemplateRecord( 0 ),
    m_pComparator( 0 )
{
  mergeRuns();
}

Tools::ExternalSort::ExternalSort(
  IObjectStream& source,
  IObjectComparator& comp,
  unsigned long bufferSize
)
    : m_cMaxBufferSize( bufferSize ),
    m_bFitsInBuffer( false ),
    m_cNumberOfSortedRecords( 0 ),
    m_cNumberOfReturnedRecords( 0 ),
    m_pExternalSource( &source ),
    m_pTemplateRecord( 0 ),
    m_pComparator( &comp )
{
  mergeRuns();
}

Tools::ExternalSort::~ExternalSort()
{
  if ( m_pTemplateRecord != 0 ) delete m_pTemplateRecord;
}

void Tools::ExternalSort::initializeRuns(
  deque<SmartPointer<TemporaryFile> >& runs
)
{
  bool bEOF = false;

  while ( ! bEOF )
  {
    while ( m_buffer.size() < m_cMaxBufferSize )
    {
      IObject* o = m_pExternalSource->getNext();

      if ( o == 0 )
      {
        bEOF = true;
        break;
      }

      ISerializable* pS = dynamic_cast<ISerializable*>( o );
      if ( pS == 0 )
      {
        delete o;
        throw IllegalStateException(
          "Tools::ExternalSort::initializeRuns: "
          "object is not serializable."
        );
      }

      m_cNumberOfSortedRecords++;

#if 0
      if ( m_cNumberOfSortedRecords % 1000000 == 0 )
        QgsDebugMsg( QString( "loaded %1 objects." ).arg( m_cNumberOfSortedRecords ) );
#endif

      if ( m_pTemplateRecord == 0 )
        m_pTemplateRecord = o->clone();

      SmartPointer<TemporaryFile> tf;
      m_buffer.push( new PQEntry( pS, m_pComparator, tf ) );
    }

    if ( bEOF && runs.size() == 0 )
      m_bFitsInBuffer = true;

    if ( ! m_buffer.empty() )
    {
      TemporaryFile* tf = new TemporaryFile();
      while ( ! m_buffer.empty() )
      {
        PQEntry* pqe = m_buffer.top(); m_buffer.pop();
        tf->storeNextObject( pqe->m_pRecord );
        delete pqe;
      }

      tf->rewindForReading();
      SmartPointer<TemporaryFile> spF( tf );
      runs.push_back( spF );
    }
  }
}

void Tools::ExternalSort::mergeRuns()
{
  deque<SmartPointer<TemporaryFile> > newruns;
  deque<SmartPointer<TemporaryFile> > runs;
  initializeRuns( runs );

  while ( runs.size() > 1 )
  {
    TemporaryFile* output = new TemporaryFile();

    priority_queue <
    PQEntry*,
    vector<PQEntry*>,
    PQEntry::ascendingComparator > buffer;

    unsigned long cRun = 0, cMaxRun = 0;
    unsigned long len;
    byte* data;

    while ( buffer.size() < m_cMaxBufferSize )
    {
      try
      {
        runs[cRun]->loadNextObject( &data, len );

        ISerializable* pS =
          dynamic_cast<ISerializable*>( m_pTemplateRecord->clone() );
        pS->loadFromByteArray( data );
        delete[] data;
        buffer.push( new PQEntry( pS, m_pComparator, runs[cRun] ) );
      }
      catch ( EndOfStreamException& e )
      {
        UNUSED( e );
        // if there are no more records in the file, do nothing.
      }

      cMaxRun = qMax( cRun, cMaxRun );
      cRun++;
      if ( cRun == runs.size() ) cRun = 0;
    }

    while ( ! buffer.empty() )
    {
      PQEntry* pqe = buffer.top(); buffer.pop();
      output->storeNextObject( pqe->m_pRecord );

      try
      {
        pqe->m_spFile->loadNextObject( pqe->m_pRecord );
        buffer.push( pqe );
      }
      catch ( EndOfStreamException& e )
      {
        UNUSED( e );
        // if there are no more records in the file, do nothing.
        delete pqe;
      }
      catch ( ... )
      {
        delete pqe;
        delete output;
        throw;
      }
    }

    output->rewindForReading();
    newruns.push_back( SmartPointer<TemporaryFile>( output ) );

    for ( cRun = 0; cRun <= cMaxRun; cRun++ ) runs.pop_front();

    if ( runs.size() <= 1 )
    {
      runs.insert( runs.end(), newruns.begin(), newruns.end() );
      newruns.clear();
    }
  }

  m_spSortedFile = runs[0];
  m_spSortedFile->rewindForReading();
}

Tools::IObject* Tools::ExternalSort::getNext()
{
  if ( m_cNumberOfReturnedRecords == m_cNumberOfSortedRecords )
    return 0;
  else
  {
    m_cNumberOfReturnedRecords++;

    unsigned long len;
    byte* data;
    m_spSortedFile->loadNextObject( &data, len );

    ISerializable* ret =
      dynamic_cast<ISerializable*>( m_pTemplateRecord->clone() );
    ret->loadFromByteArray( data );
    delete[] data;
    return dynamic_cast<IObject*>( ret );
  }
}

bool Tools::ExternalSort::hasNext() throw()
{
  if ( m_cNumberOfReturnedRecords == m_cNumberOfSortedRecords )
    return false;
  else
    return true;
}

unsigned long Tools::ExternalSort::size() throw()
{
  return m_cNumberOfSortedRecords;
}

void Tools::ExternalSort::rewind()
{
  try
  {
    m_spSortedFile->rewindForReading();
    m_cNumberOfReturnedRecords = 0;
  }
  catch ( ... )
  {
    m_cNumberOfReturnedRecords = m_cNumberOfSortedRecords;
    throw;
  }
}

