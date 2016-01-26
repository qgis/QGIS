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

#include "qgswkbtypes.h"
#include "qgsapplication.h"
#include "qgis.h"

class QgsWkbException: public std::runtime_error
{
  public:
    QgsWkbException( const char *msg ) : std::runtime_error( msg ) {}
};

class QgsWkbPrematureEnd: public QgsWkbException
{
  public:
    QgsWkbPrematureEnd() : QgsWkbException( "Premature end of WKB" ) {}
};

/** \class QgsWkbPtr
 * \note not available in Python bindings
 */

class CORE_EXPORT QgsWkbPtr
{
    mutable unsigned char *mP;
    unsigned char *mE; // pointer to one-byte-past-the-end

  public:
    /// @param p pointer to an array of bytes
    /// @deprecated, use the version taking a length
    Q_DECL_DEPRECATED QgsWkbPtr( unsigned char *p ): mP( p ), mE( std::numeric_limits<unsigned char *>::max() ) {}

    /// @param p pointer to an array of bytes
    /// @param l number of bytes in the array (length)
    QgsWkbPtr( unsigned char *p, unsigned int l ): mP( p ), mE( p + l ) {}

    inline const QgsWkbPtr &operator>>( double &v ) const { memcpy( &v, mP, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline const QgsWkbPtr &operator>>( int &v ) const { memcpy( &v, mP, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline const QgsWkbPtr &operator>>( unsigned int &v ) const { memcpy( &v, mP, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline const QgsWkbPtr &operator>>( char &v ) const { memcpy( &v, mP, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline const QgsWkbPtr &operator>>( QgsWKBTypes::Type &v ) const { memcpy( &v, mP, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline const QgsWkbPtr &operator>>( QGis::WkbType &v ) const { memcpy( &v, mP, sizeof( v ) ); mP += sizeof( v ); return *this; }
#ifdef QT_ARCH_ARM
    inline const QgsWkbPtr &operator>>( qreal &r ) const { double v; memcpy( &v, mP, sizeof( v ) ); mP += sizeof( v ); r = v; return *this; }
#endif

    inline QgsWkbPtr &operator<<( const double &v ) { memcpy( mP, &v, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline QgsWkbPtr &operator<<( const int &v ) { memcpy( mP, &v, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline QgsWkbPtr &operator<<( const unsigned int &v ) { memcpy( mP, &v, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline QgsWkbPtr &operator<<( const char &v ) { memcpy( mP, &v, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline QgsWkbPtr &operator<<( const QgsWKBTypes::Type &v ) { memcpy( mP, &v, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline QgsWkbPtr &operator<<( const QGis::WkbType &v ) { memcpy( mP, &v, sizeof( v ) ); mP += sizeof( v ); return *this; }
#ifdef QT_ARCH_ARM
    inline QgsWkbPtr &operator<<( const qreal &r ) { double v = r; memcpy( mP, &v, sizeof( v ) ); mP += sizeof( v ); return *this; }
#endif

    inline void operator+=( int n ) { mP += n; }

    inline operator unsigned char *() const { return mP; }

    inline unsigned int bytesLeft() const { return mE - mP; }

    inline bool eof() const { return mP >= mE; }
};

/** \class QgsConstWkbPtr
 * \note not available in Python bindings
 */

class CORE_EXPORT QgsConstWkbPtr
{
    mutable unsigned char *mP;
    const unsigned char *mE; // pointer to one-byte-past-the-end
    mutable bool mEndianSwap;

  public:
    /// @param p pointer to an array of bytes
    /// @deprecated, use the version taking a length
    Q_DECL_DEPRECATED QgsConstWkbPtr( const unsigned char *p );

    /// @param p pointer to an array of bytes
    /// @param l number of bytes in the array (length)
    QgsConstWkbPtr( const unsigned char *p, unsigned int l );

    QgsWKBTypes::Type readHeader() const;

    inline const QgsConstWkbPtr &operator>>( double &v ) const { read( v ); return *this; }
    inline const QgsConstWkbPtr &operator>>( float &r ) const { double v; read( v ); r = v; return *this; }
    inline const QgsConstWkbPtr &operator>>( int &v ) const { read( v ); return *this; }
    inline const QgsConstWkbPtr &operator>>( unsigned int &v ) const { read( v ); return *this; }
    inline const QgsConstWkbPtr &operator>>( char &v ) const { read( v ); return *this; }
    inline const QgsConstWkbPtr &operator>>( QGis::WkbType &v ) const { read( v ); return *this; }
    inline const QgsConstWkbPtr &operator>>( QgsWKBTypes::Type &v ) const { read( v ); return *this; }

    inline void operator+=( int n ) { mP += n; }
    inline void operator-=( int n ) { mP -= n; }

    inline operator const unsigned char *() const { return mP; }

    inline unsigned int bytesLeft() const { return mE - mP; }

    inline bool eof() const { return mP >= mE; }

    template<typename T> void read( T& v ) const
    {
      memcpy( &v, mP, sizeof( v ) );
      mP += sizeof( v );
      if ( mEndianSwap )
      {
        QgsApplication::endian_swap( v );
      }
    }
};

#endif // QGSWKBPTR_H
