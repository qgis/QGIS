/***************************************************************************
  qgspostgresrasterutils.cpp - QgsPostgresRasterUtils

 ---------------------
 begin                : 8.1.2020
 copyright            : (C) 2020 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgspostgresrasterutils.h"
#include "qgsmessagelog.h"


QVariantMap QgsPostgresRasterUtils::parseWkb( const QByteArray &wkb, int bandNo )
{
  QVariantMap result;
  const int minWkbSize { 61 };
  if ( wkb.size() < minWkbSize )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Wrong wkb size: min expected = %1, actual = %2" )
                               .arg( minWkbSize )
                               .arg( wkb.size() ), QStringLiteral( "PostGIS" ), Qgis::Critical );

    return result;
  }

  /*
  { name: 'endianness', byteOffset: 0,  byteLength: 1, type: 'Uint8' },
  { name: 'version',    byteOffset: 1,  byteLength: 2, type: 'Uint16' },
  { name: 'nBands',     byteOffset: 3,  byteLength: 2, type: 'Uint16' },
  { name: 'scaleX',     byteOffset: 5,  byteLength: 8, type: 'Float64' },
  { name: 'scaleY',     byteOffset: 13, byteLength: 8, type: 'Float64' },
  { name: 'ipX',        byteOffset: 21, byteLength: 8, type: 'Float64' },
  { name: 'ipY',        byteOffset: 29, byteLength: 8, type: 'Float64' },
  { name: 'skewX',      byteOffset: 37, byteLength: 8, type: 'Float64' },
  { name: 'skewY',      byteOffset: 45, byteLength: 8, type: 'Float64' },
  { name: 'srid',       byteOffset: 53, byteLength: 4, type: 'Int32' },
  { name: 'width',      byteOffset: 57, byteLength: 2, type: 'Uint16' },
  { name: 'height',     byteOffset: 59, byteLength: 2, type: 'Uint16' },
  */

  // Endianness
  result[ QStringLiteral( "endiannes" ) ] = static_cast<unsigned short int>( wkb[0] );  //#spellok
  // NOTE: For now only little endian is supported
  // TODO: endianness
  Q_ASSERT( result[ QStringLiteral( "endiannes" ) ] == 1 );  //#spellok
  result[ QStringLiteral( "version" ) ] = *reinterpret_cast<const unsigned short int *>( &wkb.constData()[1] );
  result[ QStringLiteral( "nBands" ) ] = *reinterpret_cast<const unsigned int *>( &wkb.constData()[3] );
  const unsigned int nBands { *reinterpret_cast<const unsigned int *>( &wkb.constData()[3] ) };
  result[ QStringLiteral( "nBands" ) ] = nBands;
  result[ QStringLiteral( "scaleX" ) ] = *reinterpret_cast<const double *>( &wkb.constData()[5] );
  result[ QStringLiteral( "scaleY" ) ] = *reinterpret_cast<const double *>( &wkb.constData()[13] );
  result[ QStringLiteral( "ipX" ) ] = *reinterpret_cast<const double *>( &wkb.constData()[21] );
  result[ QStringLiteral( "ipY" ) ] = *reinterpret_cast<const double *>( &wkb.constData()[29] );
  result[ QStringLiteral( "skewX" ) ] = *reinterpret_cast<const double *>( &wkb.constData()[37] );
  result[ QStringLiteral( "skewY" ) ] = *reinterpret_cast<const double *>( &wkb.constData()[45] );
  result[ QStringLiteral( "srid" ) ] = static_cast<int>( *reinterpret_cast<const long int *>( &wkb.constData()[53] ) );
  result[ QStringLiteral( "width" ) ] = *reinterpret_cast<const unsigned short int *>( &wkb.constData()[57] );
  result[ QStringLiteral( "height" ) ] = *reinterpret_cast<const unsigned short int *>( &wkb.constData()[59] );

  // Band data starts at index 61
  int offset = 61;

  auto readBandHeader = [ & ]( )
  {
    result[ QStringLiteral( "pxType" ) ] = *reinterpret_cast<const unsigned short int *>( &wkb.constData()[offset] ) & 0x0F;
    /*
    | 'Bool1'  // unsupported
    | 'Uint2'  // unsupported
    | 'Uint4'  // unsupported
    | 'Int8'
    | 'Uint8'
    | 'Int16'
    | 'Uint16'
    | 'Int32'
    | 'Uint32'
    | 'Float32'
    | 'Float64'
    */
    offset++;
    int pxSize = 0; // in bytes
    switch ( result[ QStringLiteral( "pxType" ) ].toInt() )
    {
      case 4:  // int8
        pxSize = 1;
        result[ QStringLiteral( "nodata" ) ] = *reinterpret_cast<const short int *>( &wkb.constData()[ offset ] );
        break;
      case 5: // uint8
        result[ QStringLiteral( "nodata" ) ] = *reinterpret_cast<const unsigned short int *>( &wkb.constData()[ offset ] );
        pxSize = 1;
        break;
      case 6: // int16
        result[ QStringLiteral( "nodata" ) ] = *reinterpret_cast<const int *>( &wkb.constData()[ offset ] );
        pxSize = 2;
        break;
      case 7: // uint16
        result[ QStringLiteral( "nodata" ) ] = *reinterpret_cast<const unsigned int *>( &wkb.constData()[ offset ] );
        pxSize = 2;
        break;
      case 8: // int32
        result[ QStringLiteral( "nodata" ) ] = static_cast<long long>( *reinterpret_cast<const long int *>( &wkb.constData()[ offset ] ) );
        pxSize = 4;
        break;
      case 9: // uint32
        result[ QStringLiteral( "nodata" ) ] = static_cast<unsigned long long>( *reinterpret_cast<const unsigned long int *>( &wkb.constData()[ offset ] ) );
        pxSize = 4;
        break;
      case 10: // float
        result[ QStringLiteral( "nodata" ) ] = *reinterpret_cast<const float *>( &wkb.constData()[ offset ] );
        pxSize = 4;
        break;
      case 11: // double
        result[ QStringLiteral( "nodata" ) ] = *reinterpret_cast<const double *>( &wkb.constData()[ offset ] );
        pxSize = 8;
        break;
      default:
        result[ QStringLiteral( "nodata" ) ] = std::numeric_limits<double>::min();
        QgsMessageLog::logMessage( QStringLiteral( "Unsupported pixel type: %1" )
                                   .arg( result[ QStringLiteral( "pxType" ) ].toInt() ), QStringLiteral( "PostGIS" ), Qgis::Critical );

    }
    result[ QStringLiteral( "pxSize" ) ] = pxSize;
    offset += pxSize; // Init of band data
    result[ QStringLiteral( "dataSize" ) ] = static_cast<unsigned int>( pxSize * result[ QStringLiteral( "width" ) ].toInt() * result[ QStringLiteral( "height" ) ].toInt() );
  };

  if ( static_cast<unsigned int>( bandNo ) > nBands )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Band number is not valid: %1 (nBands: %2" )
                               .arg( bandNo ).arg( nBands ), QStringLiteral( "PostGIS" ), Qgis::Critical );
    return result;
  }

  // Read bands (all bands if bandNo is 0)
  for ( unsigned int bandCnt = 1; bandCnt <= ( bandNo == 0 ? nBands : static_cast<unsigned int>( bandNo ) ); ++bandCnt )
  {
    readBandHeader( );
    if ( bandNo == 0 || static_cast<unsigned int>( bandNo ) == bandCnt )
    {
      result[ QStringLiteral( "band%1" ).arg( bandCnt )] = wkb.mid( offset, result[ QStringLiteral( "dataSize" ) ].toInt() );
    }
    else
    {
      // Skip
    }
    offset += result[ QStringLiteral( "dataSize" ) ].toInt();
  }
  return result;
}
