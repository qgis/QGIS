#include "qgswkbptr.h"

QgsConstWkbPtr::QgsConstWkbPtr( const unsigned char *p ): mEndianSwap( false )
{
  mP = const_cast< unsigned char * >( p );
}

QgsWKBTypes::Type QgsConstWkbPtr::readHeader() const
{
  if ( !mP )
  {
    return QgsWKBTypes::Unknown;
  }

  char wkbEndian;
  ( *this ) >> wkbEndian;
  mEndianSwap = ( wkbEndian != QgsApplication::endian() );

  QgsWKBTypes::Type wkbType;
  ( *this ) >> wkbType;
  if ( mEndianSwap )
  {
    QgsApplication::endian_swap( wkbType );
  }
  return wkbType;
}
