/******************************************************************************
 * Project:  libspatialindex - A C++ library for spatial indexing
 * Author:   Marios Hadjieleftheriou, mhadji@gmail.com
 ******************************************************************************
 * Copyright (c) 2004, Marios Hadjieleftheriou
 *
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
******************************************************************************/

#pragma once


#if (defined _WIN32 || defined _WIN64 || defined WIN32 || defined WIN64) && (defined _MSC_VER) && (_MSC_VER < 1900) && !defined __GNUC__
  typedef __int8 int8_t;
  typedef __int16 int16_t;
  typedef __int32 int32_t;
  typedef __int64 int64_t;
  typedef unsigned __int8 uint8_t;
  typedef unsigned __int16 uint16_t;
  typedef unsigned __int32 uint32_t;
  typedef unsigned __int64 uint64_t;

#else
  #include <cstdint>
#endif

#if (defined _WIN32 || defined _WIN64 || defined WIN32 || defined WIN64) && !defined __GNUC__
  #ifdef SIDX_DLL_EXPORT
    #define SIDX_DLL __declspec(dllexport)
  #else
    #define SIDX_DLL
  #endif

  // Nuke this annoying warning.  See http://www.unknownroad.com/rtfm/VisualStudio/warningC4251.html
#pragma warning( disable: 4251 )

#else
#include "qgis_core.h"
#define SIDX_DLL CORE_EXPORT
#endif

#include <cassert>
#include <iostream>
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
#include <cwchar>

#include "PointerPool.h"
#include "PoolPointer.h"

namespace Tools
{
	enum IntervalType
	{
		IT_RIGHTOPEN = 0x0,
		IT_LEFTOPEN,
		IT_OPEN,
		IT_CLOSED
	};

	enum VariantType
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
		VT_ULONGLONG,
        VT_PWCHAR
    };

	enum FileMode
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
		virtual ~Exception() = default;
	};

	class SIDX_DLL IndexOutOfBoundsException : public Exception
	{
	public:
		IndexOutOfBoundsException(size_t i);
		~IndexOutOfBoundsException() override = default;
		std::string what() override;

	private:
		std::string m_error;
	}; // IndexOutOfBoundsException

	class SIDX_DLL IllegalArgumentException : public Exception
	{
	public:
		IllegalArgumentException(std::string s);
		~IllegalArgumentException() override = default;
		std::string what() override;

	private:
		std::string m_error;
	}; // IllegalArgumentException

	class SIDX_DLL IllegalStateException : public Exception
	{
	public:
		IllegalStateException(std::string s);
		~IllegalStateException() override = default;
		std::string what() override;

	private:
		std::string m_error;
	}; // IllegalStateException

	class SIDX_DLL EndOfStreamException : public Exception
	{
	public:
		EndOfStreamException(std::string s);
		~EndOfStreamException() override = default;
		std::string what() override;

	private:
		std::string m_error;
	}; // EndOfStreamException

	class SIDX_DLL ResourceLockedException : public Exception
	{
	public:
		ResourceLockedException(std::string s);
		~ResourceLockedException() override = default;
		std::string what() override;

	private:
		std::string m_error;
	}; // ResourceLockedException

	class SIDX_DLL NotSupportedException : public Exception
	{
	public:
		NotSupportedException(std::string s);
		~NotSupportedException() override = default;
		std::string what() override;

	private:
		std::string m_error;
	}; // NotSupportedException

	//
	// Interfaces
	//
	class SIDX_DLL IInterval
	{
	public:
		virtual ~IInterval() = default;

		virtual double getLowerBound() const = 0;
		virtual double getUpperBound() const = 0;
		virtual void setBounds(double, double) = 0;
		virtual bool intersectsInterval(const IInterval&) const = 0;
		virtual bool intersectsInterval(IntervalType type, const double start, const double end) const = 0;
		virtual bool containsInterval(const IInterval&) const = 0;
		virtual IntervalType getIntervalType() const = 0;
	}; // IInterval

	class SIDX_DLL IObject
	{
	public:
		virtual ~IObject() = default;

		virtual IObject* clone() = 0;
			// return a new object that is an exact copy of this one.
			// IMPORTANT: do not return the this pointer!
	}; // IObject

	class SIDX_DLL ISerializable
	{
	public:
		virtual ~ISerializable() = default;

		virtual uint32_t getByteArraySize() = 0;
			// returns the size of the required uint8_t array.
		virtual void loadFromByteArray(const uint8_t* data) = 0;
			// load this object using the uint8_t array.
		virtual void storeToByteArray(uint8_t** data, uint32_t& length) = 0;
			// store this object in the uint8_t array.
	};

	class SIDX_DLL IComparable
	{
	public:
		virtual ~IComparable() = default;

		virtual bool operator<(const IComparable& o) const = 0;
		virtual bool operator>(const IComparable& o) const = 0;
		virtual bool operator==(const IComparable& o) const = 0;
	}; //IComparable

	class SIDX_DLL IObjectComparator
	{
	public:
		virtual ~IObjectComparator() = default;

		virtual int compare(IObject* o1, IObject* o2) = 0;
	}; // IObjectComparator

	class SIDX_DLL IObjectStream
	{
	public:
		virtual ~IObjectStream() = default;

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

		VariantType m_varType{VT_EMPTY};

		union
		{
			int16_t iVal;              // VT_SHORT
			int32_t lVal;              // VT_LONG
			int64_t llVal;             // VT_LONGLONG
			uint8_t bVal;                 // VT_BYTE
			float fltVal;              // VT_FLOAT
			double dblVal;             // VT_DOUBLE
			char cVal;                 // VT_CHAR
			uint16_t uiVal;            // VT_USHORT
			uint32_t ulVal;            // VT_ULONG
			uint64_t ullVal;           // VT_ULONGLONG
			bool blVal;                // VT_BOOL
			char* pcVal;               // VT_PCHAR
			void* pvVal;               // VT_PVOID
            wchar_t* pwcVal;
		} m_val;
	}; // Variant

	class SIDX_DLL PropertySet;
	SIDX_DLL std::ostream& operator<<(std::ostream& os, const Tools::PropertySet& p);

	class SIDX_DLL PropertySet : public ISerializable
	{
	public:
		PropertySet();
		PropertySet(const uint8_t* data);
		~PropertySet() override;

		Variant getProperty(std::string property) const;
		void setProperty(std::string property, Variant const& v);
		void removeProperty(std::string property);

		uint32_t getByteArraySize() override;
		void loadFromByteArray(const uint8_t* data) override;
		void storeToByteArray(uint8_t** data, uint32_t& length) override;

	private:
		std::map<std::string, Variant> m_propertySet;
// #ifdef HAVE_PTHREAD_H
//             pthread_rwlock_t m_rwLock;
// #else
//             bool m_rwLock;
// #endif
		friend SIDX_DLL std::ostream& operator<<(std::ostream& os, const Tools::PropertySet& p);
	}; // PropertySet

	// does not support degenerate intervals.
	class SIDX_DLL Interval : public IInterval
	{
	public:
		Interval();
		Interval(IntervalType, double, double);
		Interval(double, double);
		Interval(const Interval&);
		~Interval() override = default;
		virtual IInterval& operator=(const IInterval&);

		virtual bool operator==(const Interval&) const;
		virtual bool operator!=(const Interval&) const;
		double getLowerBound() const override;
		double getUpperBound() const override;
		void setBounds(double, double) override;
		bool intersectsInterval(const IInterval&) const override;
		bool intersectsInterval(IntervalType type, const double start, const double end) const override;
		bool containsInterval(const IInterval&) const override;
		IntervalType getIntervalType() const override;

		IntervalType m_type{IT_RIGHTOPEN};
		double m_low{0.0};
		double m_high{0.0};
	}; // Interval

	SIDX_DLL std::ostream& operator<<(std::ostream& os, const Tools::Interval& iv);

	class SIDX_DLL Random
	{
	public:
		Random();
		Random(uint32_t seed, uint16_t xsubi0);
		virtual ~Random();

		int32_t nextUniformLong();
			// returns a uniformly distributed long.
		uint32_t nextUniformUnsignedLong();
			// returns a uniformly distributed unsigned long.
		int32_t nextUniformLong(int32_t low, int32_t high);
			// returns a uniformly distributed long in the range [low, high).
		uint32_t nextUniformUnsignedLong(uint32_t low, uint32_t high);
			// returns a uniformly distributed unsigned long in the range [low, high).
		int64_t nextUniformLongLong();
			// returns a uniformly distributed long long.
		uint64_t nextUniformUnsignedLongLong();
			// returns a uniformly distributed unsigned long long.
		int64_t nextUniformLongLong(int64_t low, int64_t high);
			// returns a uniformly distributed unsigned long long in the range [low, high).
		uint64_t nextUniformUnsignedLongLong(uint64_t low, uint64_t high);
			// returns a uniformly distributed unsigned long long in the range [low, high).
		int16_t nextUniformShort();
			// returns a uniformly distributed short.
		uint16_t nextUniformUnsignedShort();
			// returns a uniformly distributed unsigned short.
		double nextUniformDouble();
			// returns a uniformly distributed double in the range [0, 1).
		double nextUniformDouble(double low, double high);
			// returns a uniformly distributed double in the range [low, high).

		bool flipCoin();

	private:
		void initDrand(uint32_t seed, uint16_t xsubi0);

		uint16_t* m_pBuffer;
	}; // Random

	#if HAVE_PTHREAD_H
	class SIDX_DLL LockGuard
	{
	public:
		LockGuard(pthread_mutex_t* pLock);
		~LockGuard();

	private:
		pthread_mutex_t* m_pLock;
	}; // LockGuard
	#endif

	class SIDX_DLL BufferedFile
	{
	public:
		BufferedFile(uint32_t u32BufferSize = 16384);
		virtual ~BufferedFile();

		virtual void close();
		virtual bool eof();
		virtual void rewind() = 0;
		virtual void seek(std::fstream::off_type offset) = 0;

	protected:
		std::fstream m_file;
		char* m_buffer;
		uint32_t m_u32BufferSize;
		bool m_bEOF{true};
	};

	class SIDX_DLL BufferedFileReader : public BufferedFile
	{
	public:
		BufferedFileReader();
		BufferedFileReader(const std::string& sFileName, uint32_t u32BufferSize = 32768);
		~BufferedFileReader() override;

		virtual void open(const std::string& sFileName);
		void rewind() override;
		void seek(std::fstream::off_type offset) override;

		virtual uint8_t readUInt8();
		virtual uint16_t readUInt16();
		virtual uint32_t readUInt32();
		virtual uint64_t readUInt64();
		virtual float readFloat();
		virtual double readDouble();
		virtual bool readBoolean();
		virtual std::string readString();
		virtual void readBytes(uint32_t u32Len, uint8_t** pData);
	};

	class SIDX_DLL BufferedFileWriter : public BufferedFile
	{
	public:
		BufferedFileWriter();
		BufferedFileWriter(const std::string& sFileName, FileMode mode = CREATE, uint32_t u32BufferSize = 32768);
		~BufferedFileWriter() override;

		virtual void open(const std::string& sFileName, FileMode mode = CREATE);
		void rewind() override;
		void seek(std::fstream::off_type offset) override;

		virtual void write(uint8_t i);
		virtual void write(uint16_t i);
		virtual void write(uint32_t i);
		virtual void write(uint64_t i);
		virtual void write(float i);
		virtual void write(double i);
		virtual void write(bool b);
		virtual void write(const std::string& s);
		virtual void write(uint32_t u32Len, uint8_t* pData);
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
		void readBytes(uint32_t u32Len, uint8_t** pData);

		void write(uint8_t i);
		void write(uint16_t i);
		void write(uint32_t i);
		void write(uint64_t i);
		void write(float i);
		void write(double i);
		void write(const std::string& s);
		void write(uint32_t u32Len, uint8_t* pData);

	private:
		std::string m_sFile;
		BufferedFile* m_pFile;
	};
}
