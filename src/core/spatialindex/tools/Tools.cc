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

#include <cstring>
#include <limits>
#include <cfloat>
#include <Tools.h>
#include "ExternalSort.h"
//#include "SHA1.h"

Tools::IndexOutOfBoundsException::IndexOutOfBoundsException( int i )
{
  std::ostringstream s;
  s << "Invalid index " << i;
  m_error = s.str();
}

std::string Tools::IndexOutOfBoundsException::what()
{
  return "IndexOutOfBoundsException: " + m_error;
}

Tools::IllegalArgumentException::IllegalArgumentException( std::string s ) : m_error( s )
{
}

std::string Tools::IllegalArgumentException::what()
{
  return "IllegalArgumentException: " + m_error;
}

Tools::IllegalStateException::IllegalStateException( std::string s ) : m_error( s )
{
}

std::string Tools::IllegalStateException::what()
{
  return "IllegalStateException: " + m_error + "\n"; //Please contact " + PACKAGE_BUGREPORT;
}

Tools::EndOfStreamException::EndOfStreamException( std::string s ) : m_error( s )
{
}

std::string Tools::EndOfStreamException::what()
{
  return "EndOfStreamException: " + m_error;
}

Tools::ResourceLockedException::ResourceLockedException( std::string s ) : m_error( s )
{
}

std::string Tools::ResourceLockedException::what()
{
  return "ResourceLockedException: " + m_error;
}

Tools::InvalidPageException::InvalidPageException( long id )
{
  std::ostringstream s;
  s << "Unknown page id " << id;
  m_error = s.str();
}

std::string Tools::InvalidPageException::what()
{
  return "InvalidPageException: " + m_error;
}

Tools::NotSupportedException::NotSupportedException( std::string s ) : m_error( s )
{
}

std::string Tools::NotSupportedException::what()
{
  return "NotSupportedException: " + m_error;
}

Tools::ParseErrorException::ParseErrorException( std::string s ) : m_error( s )
{
}

std::string Tools::ParseErrorException::what()
{
  return "ParseErrorException: " + m_error;
}

Tools::Variant::Variant() : m_varType( VT_EMPTY )
{
}

Tools::PropertySet::PropertySet( const byte* data )
{
  loadFromByteArray( data );
}

void Tools::PropertySet::loadFromByteArray( const byte* ptr )
{
  m_propertySet.clear();

  unsigned long numberOfProperties;
  memcpy( &numberOfProperties, ptr, sizeof( unsigned long ) );
  ptr += sizeof( unsigned long );

  Variant v;

  for ( unsigned long cIndex = 0; cIndex < numberOfProperties; cIndex++ )
  {
    std::string s( reinterpret_cast<const char*>( ptr ) );
    ptr += s.size() + 1;
    memcpy( &( v.m_varType ), ptr, sizeof( VariantType ) );
    ptr += sizeof( VariantType );

    switch ( v.m_varType )
    {
      case VT_LONG:
        long l;
        memcpy( &l, ptr, sizeof( long ) );
        ptr += sizeof( long );
        v.m_val.lVal = l;
        break;
      case VT_LONGLONG:
        long long ll;
        memcpy( &ll, ptr, sizeof( long long ) );
        ptr += sizeof( long long );
        v.m_val.llVal = ll;
        break;
      case VT_BYTE:
        byte b;
        memcpy( &b, ptr, sizeof( byte ) );
        ptr += sizeof( byte );
        v.m_val.bVal = b;
        break;
      case VT_SHORT:
        short s;
        memcpy( &s, ptr, sizeof( short ) );
        ptr += sizeof( short );
        v.m_val.iVal = s;
        break;
      case VT_FLOAT:
        float f;
        memcpy( &f, ptr, sizeof( float ) );
        ptr += sizeof( float );
        v.m_val.fltVal = f;
        break;
      case VT_DOUBLE:
        double d;
        memcpy( &d, ptr, sizeof( double ) );
        ptr += sizeof( double );
        v.m_val.dblVal = d;
        break;
      case VT_CHAR:
        char c;
        memcpy( &c, ptr, sizeof( char ) );
        ptr += sizeof( char );
        v.m_val.cVal = c;
        break;
      case VT_USHORT:
        unsigned short us;
        memcpy( &us, ptr, sizeof( unsigned short ) );
        ptr += sizeof( unsigned short );
        v.m_val.uiVal = us;
        break;
      case VT_ULONG:
        unsigned long ul;
        memcpy( &ul, ptr, sizeof( unsigned long ) );
        ptr += sizeof( unsigned long );
        v.m_val.ulVal = ul;
        break;
      case VT_ULONGLONG:
        unsigned long long ull;
        memcpy( &ull, ptr, sizeof( unsigned long long ) );
        ptr += sizeof( unsigned long long );
        v.m_val.ullVal = ull;
        break;
      case VT_INT:
        int i;
        memcpy( &i, ptr, sizeof( int ) );
        ptr += sizeof( int );
        v.m_val.intVal = i;
        break;
      case VT_UINT:
        unsigned int ui;
        memcpy( &ui, ptr, sizeof( unsigned int ) );
        ptr += sizeof( unsigned int );
        v.m_val.uintVal = ui;
        break;
      case VT_BOOL:
        byte bl;
        memcpy( &bl, ptr, sizeof( byte ) );
        ptr += sizeof( byte );
        v.m_val.blVal = bl != 0;
        break;
      default:
        throw IllegalStateException(
          "Tools::PropertySet::PropertySet: Deserialization problem."
        );
    }

    m_propertySet.insert( std::pair<std::string, Variant>( s, v ) );
  }
}

unsigned long Tools::PropertySet::getByteArraySize()
{
  unsigned long size = sizeof( unsigned long );
  std::map<std::string, Variant>::iterator it;

  for ( it = m_propertySet.begin(); it != m_propertySet.end(); it++ )
  {
    switch (( *it ).second.m_varType )
    {
      case VT_LONG:
        size += sizeof( long );
        break;
      case VT_LONGLONG:
        size += sizeof( long long );
        break;
      case VT_BYTE:
        size += sizeof( byte );
        break;
      case VT_SHORT:
        size += sizeof( short );
        break;
      case VT_FLOAT:
        size += sizeof( float );
        break;
      case VT_DOUBLE:
        size += sizeof( double );
        break;
      case VT_CHAR:
        size += sizeof( char );
        break;
      case VT_USHORT:
        size += sizeof( unsigned short );
        break;
      case VT_ULONG:
        size += sizeof( unsigned long );
        break;
      case VT_ULONGLONG:
        size += sizeof( unsigned long long );
        break;
      case VT_INT:
        size += sizeof( int );
        break;
      case VT_UINT:
        size += sizeof( unsigned int );
        break;
      case VT_BOOL:
        size += sizeof( byte );
        break;
      default:
        throw NotSupportedException(
          "Tools::PropertySet::getSize: Unknown type."
        );
    }
    size += ( *it ).first.size() + 1 + sizeof( VariantType );
  }

  return size;
}

void Tools::PropertySet::storeToByteArray( byte** data, unsigned long& length )
{
  length = getByteArraySize();
  *data = new byte[length];
  byte* ptr = *data;

  unsigned long numberOfProperties = m_propertySet.size();
  memcpy( ptr, &numberOfProperties, sizeof( unsigned long ) );
  ptr += sizeof( unsigned long );

  std::map<std::string, Variant>::iterator it;

  for ( it = m_propertySet.begin(); it != m_propertySet.end(); it++ )
  {
    unsigned long strSize = ( *it ).first.size();
    memcpy( ptr, ( *it ).first.c_str(), strSize );
    ptr += strSize;
    *ptr = 0;
    ptr++;

    memcpy( ptr, &(( *it ).second.m_varType ), sizeof( VariantType ) );
    ptr += sizeof( VariantType );

    switch (( *it ).second.m_varType )
    {
      case VT_LONG:
        memcpy( ptr, &(( *it ).second.m_val.lVal ), sizeof( long ) );
        ptr += sizeof( long );
        break;
      case VT_LONGLONG:
        memcpy( ptr, &(( *it ).second.m_val.llVal ), sizeof( long long ) );
        ptr += sizeof( long long );
        break;
      case VT_BYTE:
        memcpy( ptr, &(( *it ).second.m_val.bVal ), sizeof( byte ) );
        ptr += sizeof( byte );
        break;
      case VT_SHORT:
        memcpy( ptr, &(( *it ).second.m_val.iVal ), sizeof( short ) );
        ptr += sizeof( short );
        break;
      case VT_FLOAT:
        memcpy( ptr, &(( *it ).second.m_val.fltVal ), sizeof( float ) );
        ptr += sizeof( float );
        break;
      case VT_DOUBLE:
        memcpy( ptr, &(( *it ).second.m_val.dblVal ), sizeof( double ) );
        ptr += sizeof( double );
        break;
      case VT_CHAR:
        memcpy( ptr, &(( *it ).second.m_val.cVal ), sizeof( char ) );
        ptr += sizeof( char );
        break;
      case VT_USHORT:
        memcpy( ptr, &(( *it ).second.m_val.uiVal ), sizeof( unsigned short ) );
        ptr += sizeof( unsigned short );
        break;
      case VT_ULONG:
        memcpy( ptr, &(( *it ).second.m_val.ulVal ), sizeof( unsigned long ) );
        ptr += sizeof( unsigned long );
        break;
      case VT_ULONGLONG:
        memcpy( ptr, &(( *it ).second.m_val.ullVal ), sizeof( unsigned long long ) );
        ptr += sizeof( unsigned long long );
        break;
      case VT_INT:
        memcpy( ptr, &(( *it ).second.m_val.intVal ), sizeof( int ) );
        ptr += sizeof( int );
        break;
      case VT_UINT:
        memcpy( ptr, &(( *it ).second.m_val.uintVal ), sizeof( unsigned int ) );
        ptr += sizeof( unsigned int );
        break;
      case VT_BOOL:
        byte bl;
        bl = ( *it ).second.m_val.blVal;
        memcpy( ptr, &bl, sizeof( byte ) );
        ptr += sizeof( byte );
        break;
      default:
        throw NotSupportedException(
          "Tools::PropertySet::getData: Cannot serialize a variant of this type."
        );
    }
  }

  assert( ptr == ( *data ) + length );
}

Tools::Variant Tools::PropertySet::getProperty( std::string property )
{
  std::map<std::string, Variant>::iterator it = m_propertySet.find( property );
  if ( it != m_propertySet.end() ) return ( *it ).second;
  else return Variant();
}

void Tools::PropertySet::setProperty( std::string property, Variant& v )
{
  m_propertySet.insert( std::pair<std::string, Variant>( property, v ) );
}

void Tools::PropertySet::removeProperty( std::string property )
{
  std::map<std::string, Variant>::iterator it = m_propertySet.find( property );
  if ( it != m_propertySet.end() ) m_propertySet.erase( it );
}

Tools::IObjectStream* Tools::externalSort( IObjectStream& source, unsigned long bufferSize )
{
  return new ExternalSort( source, bufferSize );

}

Tools::IObjectStream* Tools::externalSort( IObjectStream& source, IObjectComparator& comp, unsigned long bufferSize )
{
  return new ExternalSort( source, comp, bufferSize );
}

Tools::Interval::Interval() : m_type( IT_RIGHTOPEN ), m_low( 0.0 ), m_high( 0.0 )
{
}

Tools::Interval::Interval( IntervalType t, double l, double h ) : m_type( t ), m_low( l ), m_high( h )
{
  assert( l < h );
}

Tools::Interval::Interval( double l, double h ) : m_type( IT_RIGHTOPEN ), m_low( l ), m_high( h )
{
  assert( l < h );
}

Tools::Interval::Interval( const Interval& iv )
{
  m_low = iv.m_low;
  m_high = iv.m_high;
  m_type = iv.m_type;
}

Tools::IInterval& Tools::Interval::operator=( const IInterval & iv )
{
  if ( this != &iv )
  {
    m_low = iv.getLowerBound();
    m_high = iv.getUpperBound();
    m_type = iv.getIntervalType();
  }

  return *this;
}

Tools::Interval& Tools::Interval::operator=( const Interval & iv )
{
  *this = *( static_cast<const IInterval*>( &iv ) );
  return *this;
}

bool Tools::Interval::operator==( const Interval& iv ) const
{
  if (
    m_type == iv.m_type &&
    m_low >= iv.m_low - std::numeric_limits<double>::epsilon() &&
    m_low <= iv.m_low + std::numeric_limits<double>::epsilon() &&
    m_high >= iv.m_high - std::numeric_limits<double>::epsilon() &&
    m_high <= iv.m_high + std::numeric_limits<double>::epsilon() )
    return true;

  return false;
}

bool Tools::Interval::operator!=( const Interval& iv ) const
{
  return !( *this == iv );
}

double Tools::Interval::getLowerBound() const
{
  return m_low;
}

double Tools::Interval::getUpperBound() const
{
  return m_high;
}

void Tools::Interval::setBounds( double l, double h )
{
  assert( l <= h );

  m_low = l;
  m_high = h;
}

bool Tools::Interval::intersectsInterval( const IInterval& i ) const
{
  return intersectsInterval( i.getIntervalType(), i.getLowerBound(), i.getUpperBound() );
}

bool Tools::Interval::intersectsInterval( IntervalType type, const double low, const double high ) const
{
  if ( m_high < m_low )
    throw IllegalStateException(
      "Tools::Interval::intersectsInterval: high boundary is smaller than low boundary."
    );

  if ( m_low > high || m_high < low ) return false;
  if (( m_low > low && m_low < high ) || ( m_high > low && m_high < high ) ) return true;

  switch ( m_type )
  {
    case IT_CLOSED:
      if ( m_low == high )
      {
        if ( type == IT_CLOSED || type == IT_LEFTOPEN ) return true;
        else return false;
      }
      else if ( m_high == low )
      {
        if ( type == IT_CLOSED || type == IT_RIGHTOPEN ) return true;
        else return false;
      }
      break;
    case IT_OPEN:
      if ( m_low == high || m_high == low ) return false;
      break;
    case IT_RIGHTOPEN:
      if ( m_low == high )
      {
        if ( type == IT_CLOSED || type == IT_LEFTOPEN ) return true;
        else return false;
      }
      else if ( m_high == low )
      {
        return false;
      }
      break;
    case IT_LEFTOPEN:
      if ( m_low == high )
      {
        return false;
      }
      else if ( m_high == low )
      {
        if ( type == IT_CLOSED || type == IT_RIGHTOPEN ) return true;
        else return false;
      }
      break;
  }

  return true;
}

bool Tools::Interval::containsInterval( const IInterval& i ) const
{
  if ( m_high < m_low )
    throw IllegalStateException(
      "Tools::Interval::containsInterval: high boundary is smaller than low boundary."
    );

  double low = i.getLowerBound();
  double high = i.getUpperBound();
  IntervalType type = i.getIntervalType();

  if ( m_low < low && m_high > high ) return true;
  if ( m_low > low || m_high < high ) return false;

  switch ( m_type )
  {
    case IT_CLOSED:
      break;
    case IT_OPEN:
      if (( m_low == low && m_high == high && type != IT_OPEN ) ||
          ( m_low == low && ( type == IT_CLOSED || type == IT_RIGHTOPEN ) ) ||
          ( m_high == high && ( type == IT_CLOSED || type == IT_LEFTOPEN ) ) )
        return false;
      break;
    case IT_RIGHTOPEN:
      if ( m_high == high && ( type == IT_CLOSED || type == IT_LEFTOPEN ) )
        return false;
      break;
    case IT_LEFTOPEN:
      if ( m_low == low && ( type == IT_CLOSED || type == IT_RIGHTOPEN ) )
        return false;
      break;
  }

  return true;
}

Tools::IntervalType Tools::Interval::getIntervalType() const
{
  return m_type;
}


void Tools::uncompressRLE(
  unsigned long blockSize,
  byte* in, unsigned long lin,
  byte** out, unsigned long& lout )
{
  if ( lin == 0 ) { *out = 0; lout = 0; return; }

  byte *data = 0, *pdata = 0, *pin;
  byte* cv = new byte[blockSize];
  byte* pv = new byte[blockSize];
  byte rl;
  unsigned long bufferLength = 2 * lin;

  pin = in;
  std::memcpy( cv, pin, blockSize );
  pv[0] = ~cv[0]; // force next character to be different.
  assert( pv[0] != cv[0] );
  data = new byte[bufferLength];
  pdata = data;

  while ( pin < in + lin )
  {
    std::memcpy( cv, pin, blockSize );
    pin += blockSize;

    assert( pin <= in + lin );

    if (
      bufferLength - static_cast<unsigned long>( pdata - data ) <=
      blockSize
    )
    {
      byte* tmp;
      try
      {
        tmp = new byte[2 * bufferLength];
      }
      catch ( ... )
      {
        delete[] data;
        delete[] cv;
        delete[] pv;
        throw;
      }

      std::memcpy( tmp, data, bufferLength );
      pdata = tmp + ( pdata - data );
      byte* tmp2 = data;
      data = tmp;
      delete[] tmp2;
      bufferLength *= 2;
    }

    std::memcpy( pdata, cv, blockSize );
    pdata += blockSize;

    if ( memcmp( cv, pv, blockSize ) == 0 && pin < in + lin )
    {
      std::memcpy( &rl, pin, sizeof( byte ) );
      pin += sizeof( byte );
      assert( pin <= in + lin );
      if (
        bufferLength - static_cast<unsigned long>( pdata - data ) <=
        rl * blockSize
      )
      {
        unsigned long l = std::max( bufferLength, rl * blockSize );

        byte* tmp;
        try
        {
          tmp = new byte[2 * l];
        }
        catch ( ... )
        {
          delete[] data;
          delete[] cv;
          delete[] pv;
          throw;
        }

        std::memcpy( tmp, data, bufferLength );
        pdata = tmp + ( pdata - data );
        byte* tmp2 = data;
        data = tmp;
        delete[] tmp2;
        bufferLength = 2 * l;
      }

      while ( rl > 0 )
      {
        std::memcpy( pdata, cv, blockSize );
        pdata += blockSize;
        rl--;
      }
      std::memcpy( cv, pin, blockSize );
      pv[0] = ~cv[0];
      assert( pv[0] != cv[0] );
    }
    else std::memcpy( pv, cv, blockSize );
  }

  lout = pdata - data;

  try
  {
    *out = new byte[lout];
  }
  catch ( ... )
  {
    delete[] data;
    delete[] cv;
    delete[] pv;
    throw;
  }

  std::memcpy( *out, data, lout );
  delete[] data;
  delete[] cv;
  delete[] pv;
}

#if HAVE_GETTIMEOFDAY
Tools::ResourceUsage::ResourceUsage()
{
  reset();
}

void Tools::ResourceUsage::start()
{
  struct timezone dummy;

  if ( getrusage( RUSAGE_SELF, &m_tmpRU ) != 0 )
    throw IllegalStateException(
      "Tools::ResourceUsage::start: getrusage failed."
    );

  if ( gettimeofday( &m_tmpTV, &dummy ) != 0 )
    throw IllegalStateException(
      "Tools::ResourceUsage::start: gettimeofday failed."
    );

  // maximum resident set size
  m_peakMemory = std::max( m_peakMemory, m_tmpRU.ru_maxrss );

  // total memory
  m_totalMemory = std::max( m_totalMemory, m_tmpRU.ru_ixrss + m_tmpRU.ru_idrss + m_tmpRU.ru_isrss + m_tmpRU.ru_maxrss );
}

void Tools::ResourceUsage::stop()
{
  struct timezone dummy;
  struct timeval dif;
  struct rusage ru;
  struct timeval tv;

  if ( getrusage( RUSAGE_SELF, &ru ) != 0 )
    throw IllegalStateException(
      "Tools::ResourceUsage::stop: getrusage failed."
    );

  if ( gettimeofday( &tv, &dummy ) != 0 )
    throw IllegalStateException(
      "Tools::ResourceUsage::stop: gettimeofday failed."
    );

  // total_time
  subtractTimeval( dif, tv, m_tmpTV );
  addTimeval( m_totalTime, dif );

  // system_time
  subtractTimeval( dif, ru.ru_stime, m_tmpRU.ru_stime );
  addTimeval( m_systemTime, dif );

  // user_time
  subtractTimeval( dif, ru.ru_utime, m_tmpRU.ru_utime );
  addTimeval( m_userTime, dif );

  // readIO, writeIOs
  m_readIO += ru.ru_inblock - m_tmpRU.ru_inblock;
  m_writeIO += ru.ru_oublock - m_tmpRU.ru_oublock;

  // maximum resident set size
  m_peakMemory = std::max( m_peakMemory, ru.ru_maxrss );

  // total memory
  m_totalMemory = std::max( m_totalMemory, ru.ru_ixrss + ru.ru_idrss + ru.ru_isrss + ru.ru_maxrss );

  // page faults
  m_pageFaults += ru.ru_majflt - m_tmpRU.ru_majflt;
}

void Tools::ResourceUsage::reset()
{
  m_pageFaults = 0;
  m_readIO = 0;
  m_writeIO = 0;
  m_peakMemory = 0;
  m_totalMemory = 0;
  m_totalTime.tv_sec = 0;
  m_totalTime.tv_usec = 0;
  m_userTime.tv_sec = 0;
  m_userTime.tv_usec = 0;
  m_systemTime.tv_sec = 0;
  m_systemTime.tv_usec = 0;
}

double Tools::ResourceUsage::combineTime( const struct timeval& t )
{
  return
    static_cast<double>( t.tv_sec ) +
    static_cast<double>( t.tv_usec ) / 1000000.0;
}

void Tools::ResourceUsage::addTimeval( struct timeval& result, const struct timeval& a )
{
  result.tv_sec += a.tv_sec;
  result.tv_usec += a.tv_usec;

  if ( result.tv_usec > 1000000 )
  {
    long div = result.tv_usec / 1000000;
    result.tv_sec += div;
    result.tv_usec -= div * 1000000;
  }
}

void Tools::ResourceUsage::subtractTimeval( struct timeval& result, const struct timeval& a, const struct timeval& b )
{
  result.tv_sec = a.tv_sec - b.tv_sec;
  result.tv_usec = a.tv_usec - b.tv_usec;

  if ( result.tv_usec < 0 )
  {
    result.tv_sec -= 1;
    result.tv_usec += 1000000;
  }
}

double Tools::ResourceUsage::getTotalTime()
{
  return combineTime( m_totalTime );
}

double Tools::ResourceUsage::getUserTime()
{
  return combineTime( m_userTime );
}

double Tools::ResourceUsage::getSystemTime()
{
  return combineTime( m_systemTime );
}

long Tools::ResourceUsage::getPageFaults()
{
  return m_pageFaults;
}

long Tools::ResourceUsage::getReadIO()
{
  return m_readIO;
}

long Tools::ResourceUsage::getWriteIO()
{
  return m_writeIO;
}

long Tools::ResourceUsage::getPeakResidentMemoryUsage()
{
  return m_peakMemory;
}

long Tools::ResourceUsage::getTotalMemoryUsage()
{
  return m_totalMemory;
}
#endif

#if BUILD_CPU_I686
Tools::CycleCounter::CycleCounter() : m_totalCycles( 0 ), m_bRunning( false )
{
}

double Tools::CycleCounter::getCPUMHz()
{
  unsigned long long v1 = rdtsc();
  sleep( 10 );
  unsigned long long v2 = rdtsc();
  return (( v2 - v1 ) / 1e7 );
}

double Tools::CycleCounter::getCyclesPerSecond()
{
  unsigned long long v1 = rdtsc();
  sleep( 10 );
  unsigned long long v2 = rdtsc();
  return (( v2 - v1 ) / 10 );
}

inline unsigned long long Tools::CycleCounter::rdtsc()
{
  unsigned long long ret;
__asm__ __volatile__( "rdtsc": "=A"( ret ): );
  return ret;
}

void Tools::CycleCounter::start()
{
  if ( ! m_bRunning )
  {
    m_tmpCycles = rdtsc();
    m_bRunning = true;
  }
}

void Tools::CycleCounter::stop()
{
  if ( m_bRunning )
  {
    unsigned long long t = rdtsc();
    m_totalCycles += t - m_tmpCycles;
    m_bRunning = false;
  }
}

void Tools::CycleCounter::reset()
{
  m_totalCycles = 0;
  m_bRunning = false;
}

double Tools::CycleCounter::getTotalCycles()
{
  return m_totalCycles;
}
#endif

#if HAVE_PTHREAD_H
Tools::SharedLock::SharedLock( pthread_rwlock_t* pLock )
    : m_pLock( pLock )
{
  pthread_rwlock_rdlock( m_pLock );
}

Tools::SharedLock::~SharedLock()
{
  pthread_rwlock_unlock( m_pLock );
}

Tools::ExclusiveLock::ExclusiveLock( pthread_rwlock_t* pLock )
    : m_pLock( pLock )
{
  pthread_rwlock_wrlock( m_pLock );
}

Tools::ExclusiveLock::~ExclusiveLock()
{
  pthread_rwlock_unlock( m_pLock );
}
#endif

Tools::StringTokenizer::StringTokenizer( const std::string& str, const std::string& delimiters )
    : m_index( 0 )
{
  // Skip delimiters at beginning.
  std::string::size_type lastPos = str.find_first_not_of( delimiters, 0 );
  // Find first "non-delimiter".
  std::string::size_type pos = str.find_first_of( delimiters, lastPos );

  while ( std::string::npos != pos || std::string::npos != lastPos )
  {
    // Found a token, add it to the vector.
    m_token.push_back( str.substr( lastPos, pos - lastPos ) );
    // Skip delimiters.  Note the "not_of"
    lastPos = str.find_first_not_of( delimiters, pos );
    // Find next "non-delimiter"
    pos = str.find_first_of( delimiters, lastPos );
  }
}

std::string Tools::StringTokenizer::getNextToken()
{
  return m_token.at( m_index++ );
}

bool Tools::StringTokenizer::hasMoreTokens()
{
  return ( m_index < m_token.size() );
}

void Tools::StringTokenizer::reset()
{
  m_index = 0;
}

std::string Tools::trimLeft( const std::string& source, const std::string& t )
{
  std::string str = source;
  return str.erase( 0, source.find_first_not_of( t ) );
}

std::string Tools::trimRight( const std::string& source, const std::string& t )
{
  std::string str = source;
  return str.erase( str.find_last_not_of( t ) + 1 );
}

std::string Tools::trim( const std::string& source, const std::string& t )
{
  std::string str = source;
  return trimLeft( trimRight( str, t ), t );
}

char Tools::toLower( char c )
{
#ifdef _MSC_VER
  // MSVC doesn't seem to have std::tolower(char)
  std::locale loc;
  return std::tolower( c, loc );
#else
  return std::tolower( c );
#endif//_MSC_VER
}

char Tools::toUpper( char c )
{
#ifdef _MSC_VER
  // MSVC doesn't seem to have std::toupper(char)
  std::locale loc;
  return std::toupper( c, loc );
#else
  return std::toupper( c );
#endif//_MSC_VER
}

std::string Tools::toUpperCase( const std::string& s )
{
  std::string t = s;
  transform( t.begin(), t.end(), t.begin(), Tools::toUpper );
  return t;
}

std::string Tools::toLowerCase( const std::string& s )
{
  std::string t = s;
  transform( t.begin(), t.end(), t.begin(), Tools::toLower );
  return t;
}

unsigned long long choose( unsigned long a, unsigned long k )
{
  unsigned long long cnm = 1, n = a, m = k;
  unsigned long long i, f;

  if ( m * 2 > n ) m = n - m;
  for ( i = 1 ; i <= m; n--, i++ )
  {
    if (( f = n ) % i == 0 ) f /= i;
    else cnm /= i;
    cnm *= f;
  }
  return cnm;
}

Tools::Architecture Tools::System::getArchitecture()
{
  union {double f; unsigned long i[2];} convert;
  convert.f = 1.0;

  // Note: Old versions of the Gnu g++ compiler may make an error here,
  // compile with the option  -fenum-int-equiv  to fix the problem
  if ( convert.i[1] == 0x3FF00000 ) return ARCH_LITTLEENDIAN;
  else if ( convert.i[0] == 0x3FF00000 ) return ARCH_BIGENDIAN;
  else return ARCH_NONIEEE;
}

std::ostream& Tools::operator<<( std::ostream& os, const Tools::PropertySet& p )
{
  std::map<std::string, Variant>::const_iterator it;

  for ( it = p.m_propertySet.begin(); it != p.m_propertySet.end(); it++ )
  {
    if ( it != p.m_propertySet.begin() ) os << ", ";

    switch (( *it ).second.m_varType )
    {
      case VT_LONG:
        os << ( *it ).first << ": " << ( *it ).second.m_val.lVal;
        break;
      case VT_LONGLONG:
        os << ( *it ).first << ": " << ( *it ).second.m_val.llVal;
        break;
      case VT_BYTE:
        os << ( *it ).first << ": " << ( *it ).second.m_val.bVal;
        break;
      case VT_SHORT:
        os << ( *it ).first << ": " << ( *it ).second.m_val.iVal;
        break;
      case VT_FLOAT:
        os << ( *it ).first << ": " << ( *it ).second.m_val.fltVal;
        break;
      case VT_DOUBLE:
        os << ( *it ).first << ": " << ( *it ).second.m_val.dblVal;
        break;
      case VT_CHAR:
        os << ( *it ).first << ": " << ( *it ).second.m_val.cVal;
        break;
      case VT_USHORT:
        os << ( *it ).first << ": " << ( *it ).second.m_val.uiVal;
        break;
      case VT_ULONG:
        os << ( *it ).first << ": " << ( *it ).second.m_val.ulVal;
        break;
      case VT_ULONGLONG:
        os << ( *it ).first << ": " << ( *it ).second.m_val.ullVal;
        break;
      case VT_INT:
        os << ( *it ).first << ": " << ( *it ).second.m_val.intVal;
        break;
      case VT_UINT:
        os << ( *it ).first << ": " << ( *it ).second.m_val.uintVal;
        break;
      case VT_BOOL:
        os << ( *it ).first << ": " << ( *it ).second.m_val.blVal;
        break;
      case VT_PCHAR:
        os << ( *it ).first << ": " << ( *it ).second.m_val.pcVal;
        break;
      case VT_PVOID:
        os << ( *it ).first << ": ?";
        break;
      case VT_EMPTY:
        os << ( *it ).first << ": empty";
        break;
      default:
        os << ( *it ).first << ": unknown";
    }
  }

  return os;
}

std::ostream& Tools::operator<<( std::ostream& os, const Tools::Interval& iv )
{
  os << iv.m_type << " " << iv.m_low << " " << iv.m_high;
  return os;
}

std::ostream& Tools::operator<<( std::ostream& os, const Tools::UniversalHash& h )
{
  os << h.m_k;

  for ( unsigned long i = 0; i < h.m_k; i++ )
    os << " " << h.m_a[i];

  return os;
}

