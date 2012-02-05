// Spatial Index Library
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

#pragma once

#if defined _WIN32 || defined _WIN64 || defined WIN32 || defined WIN64
typedef __int8 int8_t;
typedef __int16 int16_t;
typedef __int32 int32_t;
typedef __int64 int64_t;
typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;

// Nuke this annoying warning.  See http://www.unknownroad.com/rtfm/VisualStudio/warningC4251.html
#pragma warning( disable: 4251 )

#else
#include <stdint.h>
#endif

#if defined _WIN32 || defined _WIN64 || defined WIN32 || defined WIN64
#ifdef SPATIALINDEX_CREATE_DLL
#define SIDX_DLL __declspec(dllexport)
#else
#define SIDX_DLL __declspec(dllimport)
#endif
#else
#define SIDX_DLL
#endif

#include <assert.h>
#include <iomanip>
#include <iterator>
#include <string>
#include <sstream>
#include <fstream>
#include <queue>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <list>
#include <algorithm>
// #include <cmath>
// #include <limits>
// #include <climits>

#if HAVE_PTHREAD_H
#include <pthread.h>
#endif

#include "SmartPointer.h"
#include "PointerPool.h"
#include "PoolPointer.h"

typedef uint8_t byte;

namespace Tools
{
  SIDX_DLL enum IntervalType
  {
    IT_RIGHTOPEN = 0x0,
    IT_LEFTOPEN,
    IT_OPEN,
    IT_CLOSED
  };

  SIDX_DLL enum VariantType
  {
    VT_LONG = 0x0,
    VT_BYTE,
    VT_SHORT,
    VT_FLOAT,
    VT_DOUBLE,
    VT_CHAR,
    VT_USHORT,
    VT_ULONG,
    VT_INT,
    VT_UINT,
    VT_BOOL,
    VT_PCHAR,
    VT_PVOID,
    VT_EMPTY,
    VT_LONGLONG,
    VT_ULONGLONG
  };

  SIDX_DLL enum FileMode
  {
    APPEND = 0x0,
    CREATE
  };

  //
  // Exceptions
  //
  class SIDX_DLL Exception
  {
    public:
      virtual std::string what() = 0;
      virtual ~Exception() {}
  };

  class SIDX_DLL IndexOutOfBoundsException : public Exception
  {
    public:
      IndexOutOfBoundsException( size_t i );
      virtual ~IndexOutOfBoundsException() {}
      virtual std::string what();

    private:
      std::string m_error;
  }; // IndexOutOfBoundsException

  class SIDX_DLL IllegalArgumentException : public Exception
  {
    public:
      IllegalArgumentException( std::string s );
      virtual ~IllegalArgumentException() {}
      virtual std::string what();

    private:
      std::string m_error;
  }; // IllegalArgumentException

  class SIDX_DLL IllegalStateException : public Exception
  {
    public:
      IllegalStateException( std::string s );
      virtual ~IllegalStateException() {}
      virtual std::string what();

    private:
      std::string m_error;
  }; // IllegalStateException

  class SIDX_DLL EndOfStreamException : public Exception
  {
    public:
      EndOfStreamException( std::string s );
      virtual ~EndOfStreamException() {}
      virtual std::string what();

    private:
      std::string m_error;
  }; // EndOfStreamException

  class SIDX_DLL ResourceLockedException : public Exception
  {
    public:
      ResourceLockedException( std::string s );
      virtual ~ResourceLockedException() {}
      virtual std::string what();

    private:
      std::string m_error;
  }; // ResourceLockedException

  class SIDX_DLL NotSupportedException : public Exception
  {
    public:
      NotSupportedException( std::string s );
      virtual ~NotSupportedException() {}
      virtual std::string what();

    private:
      std::string m_error;
  }; // NotSupportedException

  //
  // Interfaces
  //
  class SIDX_DLL IInterval
  {
    public:
      virtual ~IInterval() {}

      virtual double getLowerBound() const = 0;
      virtual double getUpperBound() const = 0;
      virtual void setBounds( double, double ) = 0;
      virtual bool intersectsInterval( const IInterval& ) const = 0;
      virtual bool intersectsInterval( IntervalType type, const double start, const double end ) const = 0;
      virtual bool containsInterval( const IInterval& ) const = 0;
      virtual IntervalType getIntervalType() const = 0;
  }; // IInterval

  class SIDX_DLL IObject
  {
    public:
      virtual ~IObject() {}

      virtual IObject* clone() = 0;
      // return a new object that is an exact copy of this one.
      // IMPORTANT: do not return the this pointer!
  }; // IObject

  class SIDX_DLL ISerializable
  {
    public:
      virtual ~ISerializable() {}

      virtual uint32_t getByteArraySize() = 0;
      // returns the size of the required byte array.
      virtual void loadFromByteArray( const byte* data ) = 0;
      // load this object using the byte array.
      virtual void storeToByteArray( byte** data, uint32_t& length ) = 0;
      // store this object in the byte array.
  };

  class SIDX_DLL IComparable
  {
    public:
      virtual ~IComparable() {}

      virtual bool operator<( const IComparable& o ) const = 0;
      virtual bool operator>( const IComparable& o ) const = 0;
      virtual bool operator==( const IComparable& o ) const = 0;
  }; //IComparable

  class SIDX_DLL IObjectComparator
  {
    public:
      virtual ~IObjectComparator() {}

      virtual int compare( IObject* o1, IObject* o2 ) = 0;
  }; // IObjectComparator

  class SIDX_DLL IObjectStream
  {
    public:
      virtual ~IObjectStream() {}

      virtual IObject* getNext() = 0;
      // returns a pointer to the next entry in the
      // stream or 0 at the end of the stream.

      virtual bool hasNext() = 0;
      // returns true if there are more items in the stream.

      virtual uint32_t size() = 0;
      // returns the total number of entries available in the stream.

      virtual void rewind() = 0;
      // sets the stream pointer to the first entry, if possible.
  }; // IObjectStream

  //
  // Classes & Functions
  //

  class SIDX_DLL Variant
  {
    public:
      Variant();

      VariantType m_varType;

      union
      {
        int16_t iVal;              // VT_SHORT
        int32_t lVal;              // VT_LONG
        int64_t llVal;             // VT_LONGLONG
        byte bVal;                 // VT_BYTE
        float fltVal;              // VT_FLOAT
        double dblVal;             // VT_DOUBLE
        char cVal;                 // VT_CHAR
        uint16_t uiVal;            // VT_USHORT
        uint32_t ulVal;            // VT_ULONG
        uint64_t ullVal;           // VT_ULONGLONG
        bool blVal;                // VT_BOOL
        char* pcVal;               // VT_PCHAR
        void* pvVal;               // VT_PVOID
      } m_val;
  }; // Variant

  class SIDX_DLL PropertySet;
  SIDX_DLL std::ostream& operator<<( std::ostream& os, const Tools::PropertySet& p );

  class SIDX_DLL PropertySet : public ISerializable
  {
    public:
      PropertySet();
      PropertySet( const byte* data );
      virtual ~PropertySet();

      Variant getProperty( std::string property );
      void setProperty( std::string property, Variant& v );
      void removeProperty( std::string property );

      virtual uint32_t getByteArraySize();
      virtual void loadFromByteArray( const byte* data );
      virtual void storeToByteArray( byte** data, uint32_t& length );

    private:
      std::map<std::string, Variant> m_propertySet;
#ifdef HAVE_PTHREAD_H
      pthread_rwlock_t m_rwLock;
#else
      bool m_rwLock;
#endif
      friend SIDX_DLL std::ostream& Tools::operator<<( std::ostream& os, const Tools::PropertySet& p );
  }; // PropertySet

  // does not support degenerate intervals.
  class SIDX_DLL Interval : public IInterval
  {
    public:
      Interval();
      Interval( IntervalType, double, double );
      Interval( double, double );
      Interval( const Interval& );
      virtual ~Interval() {}
      virtual IInterval& operator=( const IInterval& );

      virtual bool operator==( const Interval& ) const;
      virtual bool operator!=( const Interval& ) const;
      virtual double getLowerBound() const;
      virtual double getUpperBound() const;
      virtual void setBounds( double, double );
      virtual bool intersectsInterval( const IInterval& ) const;
      virtual bool intersectsInterval( IntervalType type, const double start, const double end ) const;
      virtual bool containsInterval( const IInterval& ) const;
      virtual IntervalType getIntervalType() const;

      IntervalType m_type;
      double m_low;
      double m_high;
  }; // Interval

  SIDX_DLL std::ostream& operator<<( std::ostream& os, const Tools::Interval& iv );

  class SIDX_DLL Random
  {
    public:
      Random();
      Random( uint32_t seed, uint16_t xsubi0 );
      virtual ~Random();

      int32_t nextUniformLong();
      // returns a uniformly distributed long.
      uint32_t nextUniformUnsignedLong();
      // returns a uniformly distributed unsigned long.
      int32_t nextUniformLong( int32_t low, int32_t high );
      // returns a uniformly distributed long in the range [low, high).
      uint32_t nextUniformUnsignedLong( uint32_t low, uint32_t high );
      // returns a uniformly distributed unsigned long in the range [low, high).
      int64_t nextUniformLongLong();
      // returns a uniformly distributed long long.
      uint64_t nextUniformUnsignedLongLong();
      // returns a uniformly distributed unsigned long long.
      int64_t nextUniformLongLong( int64_t low, int64_t high );
      // returns a uniformly distributed unsigned long long in the range [low, high).
      uint64_t nextUniformUnsignedLongLong( uint64_t low, uint64_t high );
      // returns a uniformly distributed unsigned long long in the range [low, high).
      int16_t nextUniformShort();
      // returns a uniformly distributed short.
      uint16_t nextUniformUnsignedShort();
      // returns a uniformly distributed unsigned short.
      double nextUniformDouble();
      // returns a uniformly distributed double in the range [0, 1).
      double nextUniformDouble( double low, double high );
      // returns a uniformly distributed double in the range [low, high).

      bool flipCoin();

    private:
      void initDrand( uint32_t seed, uint16_t xsubi0 );

      uint16_t* m_pBuffer;
  }; // Random

  class SIDX_DLL SharedLock
  {
    public:
#if HAVE_PTHREAD_H
      SharedLock( pthread_rwlock_t* pLock );
      ~SharedLock();

    private:
      pthread_rwlock_t* m_pLock;
#endif
  }; // SharedLock

  class SIDX_DLL ExclusiveLock
  {
    public:
#if HAVE_PTHREAD_H
      ExclusiveLock( pthread_rwlock_t* pLock );
      ~ExclusiveLock();

    private:
      pthread_rwlock_t* m_pLock;
#endif
  }; // ExclusiveLock

  class SIDX_DLL BufferedFile
  {
    public:
      BufferedFile( uint32_t u32BufferSize = 16384 );
      virtual ~BufferedFile();

      virtual void close();
      virtual bool eof();
      virtual void rewind() = 0;
      virtual void seek( std::fstream::off_type offset ) = 0;

    protected:
      std::fstream m_file;
      char* m_buffer;
      uint32_t m_u32BufferSize;
      bool m_bEOF;
  };

  class SIDX_DLL BufferedFileReader : public BufferedFile
  {
    public:
      BufferedFileReader();
      BufferedFileReader( const std::string& sFileName, uint32_t u32BufferSize = 32768 );
      virtual ~BufferedFileReader();

      virtual void open( const std::string& sFileName );
      virtual void rewind();
      virtual void seek( std::fstream::off_type offset );

      virtual uint8_t readUInt8();
      virtual uint16_t readUInt16();
      virtual uint32_t readUInt32();
      virtual uint64_t readUInt64();
      virtual float readFloat();
      virtual double readDouble();
      virtual bool readBoolean();
      virtual std::string readString();
      virtual void readBytes( uint32_t u32Len, byte** pData );
  };

  class SIDX_DLL BufferedFileWriter : public BufferedFile
  {
    public:
      BufferedFileWriter();
      BufferedFileWriter( const std::string& sFileName, FileMode mode = CREATE, uint32_t u32BufferSize = 32768 );
      virtual ~BufferedFileWriter();

      virtual void open( const std::string& sFileName, FileMode mode = CREATE );
      virtual void rewind();
      virtual void seek( std::fstream::off_type offset );

      virtual void write( uint8_t i );
      virtual void write( uint16_t i );
      virtual void write( uint32_t i );
      virtual void write( uint64_t i );
      virtual void write( float i );
      virtual void write( double i );
      virtual void write( bool b );
      virtual void write( const std::string& s );
      virtual void write( uint32_t u32Len, byte* pData );
  };

  class SIDX_DLL TemporaryFile
  {
    public:
      TemporaryFile();
      virtual ~TemporaryFile();

      void rewindForReading();
      void rewindForWriting();
      bool eof();
      std::string getFileName() const;

      uint8_t readUInt8();
      uint16_t readUInt16();
      uint32_t readUInt32();
      uint64_t readUInt64();
      float readFloat();
      double readDouble();
      std::string readString();
      void readBytes( uint32_t u32Len, byte** pData );

      void write( uint8_t i );
      void write( uint16_t i );
      void write( uint32_t i );
      void write( uint64_t i );
      void write( float i );
      void write( double i );
      void write( const std::string& s );
      void write( uint32_t u32Len, byte* pData );

    private:
      std::string m_sFile;
      BufferedFile* m_pFile;
  };
}

