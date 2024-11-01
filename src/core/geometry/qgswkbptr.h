/***************************************************************************
    qgswkbptr.h
    ---------------------
    begin                : January 2014
    copyright            : (C) 2014 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSWKBPTR_H
#define QGSWKBPTR_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgis_sip.h"
#include "qgsexception.h"
#include "qpolygon.h"

/**
 * \ingroup core
 * \brief Custom exception class for Wkb related exceptions.
 * \note not available in Python bindings
 */
#ifndef SIP_RUN
class CORE_EXPORT QgsWkbException : public QgsException
{
  public:
    QgsWkbException( QString const &what )
      : QgsException( what ) {}
};
#endif


/**
 * \ingroup core
 * \class QgsWkbPtr
 * \brief WKB pointer handler.
 */
class CORE_EXPORT QgsWkbPtr
{
    mutable unsigned char *mP;
    unsigned char *mStart;
    unsigned char *mEnd;

    void verifyBound( int size ) const;

    template<typename T> void read( T &v ) const
    {
      verifyBound( sizeof v );
      memcpy( &v, mP, sizeof v );
      mP += sizeof v;
    }

    template<typename T> void write( T &v ) const
    {
      verifyBound( sizeof v );
      memcpy( mP, &v, sizeof v );
      mP += sizeof v;
    }

    void write( const QByteArray &data ) const
    {
      verifyBound( data.length() );
      memcpy( mP, data.constData(), data.length() );
      mP += data.length();
    }

  public:
    //! Construct WKB pointer from QByteArray
    QgsWkbPtr( QByteArray &wkb ) SIP_SKIP;

    QgsWkbPtr( unsigned char *p SIP_ARRAY, int size SIP_ARRAYSIZE );

    inline const QgsWkbPtr &operator>>( double &v ) const
    {
      read( v );
      return *this;
    }
    SIP_SKIP
    inline const QgsWkbPtr &operator>>( float &r ) const
    {
      double v;
      read( v );
      r = v;
      return *this;
    }
    SIP_SKIP
    //! Reads an integer value into a qint32
    inline const QgsWkbPtr &operator>>( qint32 &v ) const
    {
      read( v );
      return *this;
    }
    SIP_SKIP
    //! Reads an integer value into a longlong
    inline const QgsWkbPtr &operator>>( qint64 &r ) const
    {
      quint32 v;
      read( v );
      r = v;
      return *this;
    }
    SIP_SKIP
    //! Reads an unsigned integer value
    inline const QgsWkbPtr &operator>>( quint32 &v ) const
    {
      read( v );
      return *this;
    }
    SIP_SKIP
    //! Reads an char value
    inline const QgsWkbPtr &operator>>( char &v ) const
    {
      read( v );
      return *this;
    }
    SIP_SKIP
    //! Reads a Qgis::WkbType enum value
    inline const QgsWkbPtr &operator>>( Qgis::WkbType &v ) const
    {
      read( v );
      return *this;
    }
    SIP_SKIP

    //! Writes a double to the pointer
    inline QgsWkbPtr &operator<<( double v )
    {
      write( v );
      return *this;
    }
    SIP_SKIP
    //! Writes a float to the pointer
    inline QgsWkbPtr &operator<<( float r )
    {
      double v = r;
      write( v );
      return *this;
    }
    SIP_SKIP
    //! Writes an int to the pointer
    inline QgsWkbPtr &operator<<( qint32 v )
    {
      write( v );
      return *this;
    }
    SIP_SKIP
    //! Writes a longlong as int to the pointer
    inline QgsWkbPtr &operator<<( qint64 r )
    {
      quint32 v = r;
      write( v );
      return *this;
    }
    SIP_SKIP
    //! Writes an unsigned int to the pointer
    inline QgsWkbPtr &operator<<( quint32 v )
    {
      write( v );
      return *this;
    }
    SIP_SKIP
    //! Writes a char to the pointer
    inline QgsWkbPtr &operator<<( char v )
    {
      write( v );
      return *this;
    }
    SIP_SKIP
    //! Writes a WKB type value to the pointer
    inline QgsWkbPtr &operator<<( Qgis::WkbType v )
    {
      write( v );
      return *this;
    }
    SIP_SKIP
    //! Append data from a byte array
    inline QgsWkbPtr &operator<<( const QByteArray &data )
    {
      write( data );
      return *this;
    }
    SIP_SKIP

    inline void operator+=( int n ) const
    {
      verifyBound( n );
      mP += n;
    }
    SIP_SKIP

    inline operator unsigned char *() const { return mP; }
    SIP_SKIP

    /**
     * \brief size
     * \note not available in Python bindings
     */
    inline int size() const { return mEnd - mStart; }
    SIP_SKIP

    /**
     * \brief remaining
     * \note not available in Python bindings
     */
    inline int remaining() const { return mEnd - mP; }
    SIP_SKIP

    /**
     * \brief writtenSize
     * \note not available in Python bindings
     */
    inline int writtenSize() const { return mP - mStart; }
    SIP_SKIP
};

/**
 * \ingroup core
 * \class QgsConstWkbPtr
 * \brief A const WKB pointer.
 */

class CORE_EXPORT QgsConstWkbPtr
{
  protected:
    mutable unsigned char *mP;
    unsigned char *mEnd;
    mutable bool mEndianSwap;
    mutable Qgis::WkbType mWkbType;

    /**
     * \brief Verify bounds
     * \note not available in Python bindings
     */
    void verifyBound( int size ) const SIP_SKIP;

    /**
     * \brief Read a value
     * \note not available in Python bindings
     */
    template<typename T> void read( T &v ) const SIP_SKIP
    {
      verifyBound( sizeof v );
      memcpy( &v, mP, sizeof( v ) );
      mP += sizeof( v );
      if ( mEndianSwap )
        endian_swap( v );
    }

  public:
    //! Construct WKB pointer from QByteArray
    explicit QgsConstWkbPtr( const QByteArray &wkb ) SIP_SKIP;
    QgsConstWkbPtr( const unsigned char *p SIP_ARRAY, int size SIP_ARRAYSIZE );

    /**
     * \brief readHeader
     * \note not available in Python bindings
     */
    Qgis::WkbType readHeader() const SIP_SKIP;

    inline const QgsConstWkbPtr &operator>>( double &v ) const
    {
      read( v );
      return *this;
    }
    SIP_SKIP
    inline const QgsConstWkbPtr &operator>>( float &r ) const
    {
      double v;
      read( v );
      r = v;
      return *this;
    }
    SIP_SKIP
    inline const QgsConstWkbPtr &operator>>( int &v ) const
    {
      read( v );
      return *this;
    }
    SIP_SKIP
    inline const QgsConstWkbPtr &operator>>( unsigned int &v ) const
    {
      read( v );
      return *this;
    }
    SIP_SKIP
    inline const QgsConstWkbPtr &operator>>( char &v ) const
    {
      read( v );
      return *this;
    }
    SIP_SKIP

    //! Read a point
    const QgsConstWkbPtr &operator>>( QPointF &point ) const;
    SIP_SKIP
    //! Read a point array
    const QgsConstWkbPtr &operator>>( QPolygonF &points ) const;
    SIP_SKIP

    inline void operator+=( int n ) const
    {
      verifyBound( n );
      mP += n;
    }
    SIP_SKIP
    inline void operator-=( int n ) const { mP -= n; }
    SIP_SKIP

    inline operator const unsigned char *() const { return mP; }
    SIP_SKIP

    /**
     * \brief remaining
     * \note not available in Python bindings
     */
    inline int remaining() const { return mEnd - mP; }
    SIP_SKIP

  private:
    template<typename T> void endian_swap( T &value ) const SIP_SKIP
    {
      char *data = reinterpret_cast<char *>( &value );
      const std::size_t n = sizeof( value );
      for ( std::size_t i = 0, m = n / 2; i < m; ++i )
      {
        std::swap( data[i], data[n - 1 - i] );
      }
    }
};

#endif // QGSWKBPTR_H
