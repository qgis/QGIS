#ifndef QGSWKBPTR_H
#define QGSWKBPTR_H

#include "qgswkbtypes.h"
#include "qgsapplication.h"
#include "qgis.h"

class CORE_EXPORT QgsWkbPtr
{
    mutable unsigned char *mP;

  public:
    QgsWkbPtr( unsigned char *p ): mP( p ) {}

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
};

class CORE_EXPORT QgsConstWkbPtr
{
    mutable unsigned char *mP;
    mutable bool mEndianSwap;

  public:
    QgsConstWkbPtr( const unsigned char *p );
    QgsWKBTypes::Type readHeader() const;

    inline const QgsConstWkbPtr &operator>>( double &v ) const { read( v ); return *this; }
    inline const QgsConstWkbPtr &operator>>( int &v ) const { read( v ); return *this; }
    inline const QgsConstWkbPtr &operator>>( unsigned int &v ) const { read( v ); return *this; }
    inline const QgsConstWkbPtr &operator>>( char &v ) const { read( v ); return *this; }
    inline const QgsConstWkbPtr &operator>>( QGis::WkbType &v ) const { read( v ); return *this; }
    inline const QgsConstWkbPtr &operator>>( QgsWKBTypes::Type &v ) const { read( v ); return *this; }
#ifdef QT_ARCH_ARM
    inline const QgsConstWkbPtr &operator>>( qreal &r ) const { double v; read( v ); r = v; return *this; }
#endif

    inline void operator+=( int n ) { mP += n; }
    inline void operator-=( int n ) { mP -= n; }

    inline operator const unsigned char *() const { return mP; }

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
