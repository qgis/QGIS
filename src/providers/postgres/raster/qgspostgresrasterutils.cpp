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
    QgsMessageLog::logMessage( QStringLiteral( "Wrong wkb size: min expected = %1, actual = %2" ).arg( minWkbSize ).arg( wkb.size() ), QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Critical );

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
  result[QStringLiteral( "endianness" )] = static_cast<unsigned short int>( wkb[0] );
  // NOTE: For now only little endian is supported
  // TODO: endianness
  Q_ASSERT( result[QStringLiteral( "endianness" )] == 1 );
  result[QStringLiteral( "version" )] = *reinterpret_cast<const unsigned short int *>( &wkb.constData()[1] );
  const unsigned short int nBands { *reinterpret_cast<const unsigned short int *>( &wkb.constData()[3] ) };
  result[QStringLiteral( "nBands" )] = nBands;
  result[QStringLiteral( "scaleX" )] = *reinterpret_cast<const double *>( &wkb.constData()[5] );
  result[QStringLiteral( "scaleY" )] = *reinterpret_cast<const double *>( &wkb.constData()[13] );
  result[QStringLiteral( "ipX" )] = *reinterpret_cast<const double *>( &wkb.constData()[21] );
  result[QStringLiteral( "ipY" )] = *reinterpret_cast<const double *>( &wkb.constData()[29] );
  result[QStringLiteral( "skewX" )] = *reinterpret_cast<const double *>( &wkb.constData()[37] );
  result[QStringLiteral( "skewY" )] = *reinterpret_cast<const double *>( &wkb.constData()[45] );
  result[QStringLiteral( "srid" )] = static_cast<int>( *reinterpret_cast<const long int *>( &wkb.constData()[53] ) );
  result[QStringLiteral( "width" )] = *reinterpret_cast<const unsigned short int *>( &wkb.constData()[57] );
  result[QStringLiteral( "height" )] = *reinterpret_cast<const unsigned short int *>( &wkb.constData()[59] );

  // Band data starts at index 61
  int offset = 61;

  auto readBandHeader = [&]() {
    result[QStringLiteral( "pxType" )] = *reinterpret_cast<const unsigned short int *>( &wkb.constData()[offset] ) & 0x0F;
    /*
    | 0 'Bool1'  // unsupported
    | 1 'Uint2'  // unsupported
    | 2 'Uint4'  // unsupported
    | 3 'Int8'
    | 4 'Uint8'
    | 5 'Int16'
    | 6 'Uint16'
    | 7 'Int32'
    | 8 'Uint32'
    !!!!!!!!!!!!!!!!!!!!!!!!!!!
    | 9 is missing!!!!
    !!!!!!!!!!!!!!!!!!!!!!!!!!!
    | 10 'Float32'
    | 11 'Float64'
    */
    offset++;
    int pxSize = 0; // in bytes
    switch ( result[QStringLiteral( "pxType" )].toInt() )
    {
      case 3: // int8
        pxSize = 1;
        result[QStringLiteral( "nodata" )] = *reinterpret_cast<const short int *>( &wkb.constData()[offset] );
        break;
      case 4: // uint8
        result[QStringLiteral( "nodata" )] = *reinterpret_cast<const unsigned short int *>( &wkb.constData()[offset] );
        pxSize = 1;
        break;
      case 5: // int16
        result[QStringLiteral( "nodata" )] = *reinterpret_cast<const int *>( &wkb.constData()[offset] );
        pxSize = 2;
        break;
      case 6: // uint16
        result[QStringLiteral( "nodata" )] = *reinterpret_cast<const unsigned int *>( &wkb.constData()[offset] );
        pxSize = 2;
        break;
      case 7: // int32
        result[QStringLiteral( "nodata" )] = static_cast<long long>( *reinterpret_cast<const long int *>( &wkb.constData()[offset] ) );
        pxSize = 4;
        break;
      case 8: // uint32
        result[QStringLiteral( "nodata" )] = static_cast<unsigned long long>( *reinterpret_cast<const unsigned long int *>( &wkb.constData()[offset] ) );
        pxSize = 4;
        break;

        // Note: 9 is missing from the specs

      case 10: // float 32 bit
        result[QStringLiteral( "nodata" )] = *reinterpret_cast<const float *>( &wkb.constData()[offset] );
        pxSize = 4;
        break;
      case 11: // double 64 bit
        result[QStringLiteral( "nodata" )] = *reinterpret_cast<const double *>( &wkb.constData()[offset] );
        pxSize = 8;
        break;
      default:
        result[QStringLiteral( "nodata" )] = std::numeric_limits<double>::min();
        QgsMessageLog::logMessage( QStringLiteral( "Unsupported pixel type: %1" ).arg( result[QStringLiteral( "pxType" )].toInt() ), QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Critical );
    }
    result[QStringLiteral( "pxSize" )] = pxSize;
    offset += pxSize; // Init of band data
    result[QStringLiteral( "dataSize" )] = static_cast<unsigned int>( pxSize ) * result[QStringLiteral( "width" )].toUInt() * result[QStringLiteral( "height" )].toUInt();
  };

  if ( static_cast<unsigned int>( bandNo ) > nBands )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Band number is not valid: %1 (nBands: %2" ).arg( bandNo ).arg( nBands ), QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Critical );
    return result;
  }

  // Read bands (all bands if bandNo is 0)
  for ( unsigned int bandCnt = 1; bandCnt <= ( bandNo == 0 ? nBands : static_cast<unsigned int>( bandNo ) ); ++bandCnt )
  {
    readBandHeader();
    if ( bandNo == 0 || static_cast<unsigned int>( bandNo ) == bandCnt )
    {
      result[QStringLiteral( "band%1" ).arg( bandCnt )] = wkb.mid( offset, result[QStringLiteral( "dataSize" )].toUInt() );
      // Invert rows?
      if ( result[QStringLiteral( "scaleY" )].toDouble() > 0 )
      {
        const unsigned int numRows { result[QStringLiteral( "height" )].toUInt() };
        const auto rowSize { result[QStringLiteral( "dataSize" )].toUInt() / numRows };
        const QByteArray &oldBa { result[QStringLiteral( "band%1" ).arg( bandCnt )].toByteArray() };
        QByteArray ba;
        for ( qlonglong rowOffset = ( numRows - 1 ) * rowSize; rowOffset >= 0; rowOffset -= rowSize )
        {
          ba.append( oldBa.mid( rowOffset, rowSize ) );
        }
        result[QStringLiteral( "band%1" ).arg( bandCnt )] = ba;
      }
    }
    else
    {
      // Skip
    }
    offset += result[QStringLiteral( "dataSize" )].toUInt();
  }
  return result;
}
