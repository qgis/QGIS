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
    QgsMessageLog::logMessage( u"Wrong wkb size: min expected = %1, actual = %2"_s.arg( minWkbSize ).arg( wkb.size() ), u"PostGIS"_s, Qgis::MessageLevel::Critical );

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
  result[u"endianness"_s] = static_cast<unsigned short int>( wkb[0] );
  // NOTE: For now only little endian is supported
  // TODO: endianness
  Q_ASSERT( result[u"endianness"_s] == 1 );
  result[u"version"_s] = *reinterpret_cast<const unsigned short int *>( &wkb.constData()[1] );
  const unsigned short int nBands { *reinterpret_cast<const unsigned short int *>( &wkb.constData()[3] ) };
  result[u"nBands"_s] = nBands;
  result[u"scaleX"_s] = *reinterpret_cast<const double *>( &wkb.constData()[5] );
  result[u"scaleY"_s] = *reinterpret_cast<const double *>( &wkb.constData()[13] );
  result[u"ipX"_s] = *reinterpret_cast<const double *>( &wkb.constData()[21] );
  result[u"ipY"_s] = *reinterpret_cast<const double *>( &wkb.constData()[29] );
  result[u"skewX"_s] = *reinterpret_cast<const double *>( &wkb.constData()[37] );
  result[u"skewY"_s] = *reinterpret_cast<const double *>( &wkb.constData()[45] );
  result[u"srid"_s] = static_cast<int>( *reinterpret_cast<const long int *>( &wkb.constData()[53] ) );
  result[u"width"_s] = *reinterpret_cast<const unsigned short int *>( &wkb.constData()[57] );
  result[u"height"_s] = *reinterpret_cast<const unsigned short int *>( &wkb.constData()[59] );

  // Band data starts at index 61
  int offset = 61;

  auto readBandHeader = [&]() {
    result[u"pxType"_s] = *reinterpret_cast<const unsigned short int *>( &wkb.constData()[offset] ) & 0x0F;
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
    switch ( result[u"pxType"_s].toInt() )
    {
      case 3: // int8
        pxSize = 1;
        result[u"nodata"_s] = *reinterpret_cast<const short int *>( &wkb.constData()[offset] );
        break;
      case 4: // uint8
        result[u"nodata"_s] = *reinterpret_cast<const unsigned short int *>( &wkb.constData()[offset] );
        pxSize = 1;
        break;
      case 5: // int16
        result[u"nodata"_s] = *reinterpret_cast<const int *>( &wkb.constData()[offset] );
        pxSize = 2;
        break;
      case 6: // uint16
        result[u"nodata"_s] = *reinterpret_cast<const unsigned int *>( &wkb.constData()[offset] );
        pxSize = 2;
        break;
      case 7: // int32
        result[u"nodata"_s] = static_cast<long long>( *reinterpret_cast<const long int *>( &wkb.constData()[offset] ) );
        pxSize = 4;
        break;
      case 8: // uint32
        result[u"nodata"_s] = static_cast<unsigned long long>( *reinterpret_cast<const unsigned long int *>( &wkb.constData()[offset] ) );
        pxSize = 4;
        break;

        // Note: 9 is missing from the specs

      case 10: // float 32 bit
        result[u"nodata"_s] = *reinterpret_cast<const float *>( &wkb.constData()[offset] );
        pxSize = 4;
        break;
      case 11: // double 64 bit
        result[u"nodata"_s] = *reinterpret_cast<const double *>( &wkb.constData()[offset] );
        pxSize = 8;
        break;
      default:
        result[u"nodata"_s] = std::numeric_limits<double>::min();
        QgsMessageLog::logMessage( u"Unsupported pixel type: %1"_s.arg( result[u"pxType"_s].toInt() ), u"PostGIS"_s, Qgis::MessageLevel::Critical );
    }
    result[u"pxSize"_s] = pxSize;
    offset += pxSize; // Init of band data
    result[u"dataSize"_s] = static_cast<unsigned int>( pxSize ) * result[u"width"_s].toUInt() * result[u"height"_s].toUInt();
  };

  if ( static_cast<unsigned int>( bandNo ) > nBands )
  {
    QgsMessageLog::logMessage( u"Band number is not valid: %1 (nBands: %2"_s.arg( bandNo ).arg( nBands ), u"PostGIS"_s, Qgis::MessageLevel::Critical );
    return result;
  }

  // Read bands (all bands if bandNo is 0)
  for ( unsigned int bandCnt = 1; bandCnt <= ( bandNo == 0 ? nBands : static_cast<unsigned int>( bandNo ) ); ++bandCnt )
  {
    readBandHeader();
    if ( bandNo == 0 || static_cast<unsigned int>( bandNo ) == bandCnt )
    {
      result[u"band%1"_s.arg( bandCnt )] = wkb.mid( offset, result[u"dataSize"_s].toUInt() );
      // Invert rows?
      if ( result[u"scaleY"_s].toDouble() > 0 )
      {
        const unsigned int numRows { result[u"height"_s].toUInt() };
        const auto rowSize { result[u"dataSize"_s].toUInt() / numRows };
        const QByteArray &oldBa { result[u"band%1"_s.arg( bandCnt )].toByteArray() };
        QByteArray ba;
        for ( qlonglong rowOffset = ( numRows - 1 ) * rowSize; rowOffset >= 0; rowOffset -= rowSize )
        {
          ba.append( oldBa.mid( rowOffset, rowSize ) );
        }
        result[u"band%1"_s.arg( bandCnt )] = ba;
      }
    }
    else
    {
      // Skip
    }
    offset += result[u"dataSize"_s].toUInt();
  }
  return result;
}
