/***************************************************************************
                         qgslazdecoder.h
                         --------------------
    begin                : March 2022
    copyright            : (C) 2022 by Belgacem Nedjima
    email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAZDECODER_H
#define QGSLAZDECODER_H


#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgspointcloudblock.h"
#include "qgspointcloudattribute.h"

#include "lazperf/lazperf.hpp"
#include "lazperf/readers.hpp"

///@cond PRIVATE
#define SIP_NO_FILE

#include <QString>

class QgsPointCloudExpression;

class QgsLazDecoder
{
  public:
    struct ExtraBytesAttributeDetails
    {
      QString attribute;
      QgsPointCloudAttribute::DataType type;
      int size;
      int offset;
    };

    enum class LazAttribute
    {
      X,
      Y,
      Z,
      Classification,
      Intensity,
      ReturnNumber,
      NumberOfReturns,
      ScanDirectionFlag,
      EdgeOfFlightLine,
      ScanAngleRank,
      UserData,
      PointSourceId,
      GpsTime,
      Red,
      Green,
      Blue,
      ExtraBytes,
      MissingOrUnknown
    };

    struct RequestedAttributeDetails
    {
      RequestedAttributeDetails( LazAttribute attribute, QgsPointCloudAttribute::DataType type, int size, int offset = -1 )
        : attribute( attribute )
        , type( type )
        , size( size )
        , offset( offset )
      {}

      LazAttribute attribute;
      QgsPointCloudAttribute::DataType type;
      int size;
      int offset; // Used in case the attribute is an extra byte attribute
    };

    static QgsPointCloudBlock *decompressLaz( const QString &filename, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset, QgsPointCloudExpression &filterExpression );
    static QgsPointCloudBlock *decompressLaz( const QByteArray &data, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset, QgsPointCloudExpression &filterExpression );
    static QgsPointCloudBlock *decompressCopc( const QString &filename, const lazperf::header14 &header, uint64_t blockOffset, uint64_t blockSize, int32_t pointCount, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &_scale, const QgsVector3D &_offset, QgsPointCloudExpression &filterExpression );
    static QgsPointCloudBlock *decompressCopc( const QByteArray &data, const lazperf::header14 &header, lazperf::eb_vlr &extraBytesVlr, int32_t pointCount, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &_scale, const QgsVector3D &_offset, QgsPointCloudExpression &filterExpression );
    //! Returns the list of extrabytes attributes with their type, size and offsets from LazPerf VLR struct
    static QVector<QgsLazDecoder::ExtraBytesAttributeDetails> readExtraByteAttributesFromVlr( lazperf::eb_vlr &ebVlr, int point_record_length )
    {
      QVector<ExtraBytesAttributeDetails> extrabytesAttr;

      for ( std::vector<lazperf::eb_vlr::ebfield>::reverse_iterator it = ebVlr.items.rbegin(); it != ebVlr.items.rend(); ++it )
      {
        lazperf::eb_vlr::ebfield &field = *it;
        ExtraBytesAttributeDetails ebAtrr;
        ebAtrr.attribute = QString::fromStdString( field.name );
        switch ( field.data_type )
        {
          case 0:
            ebAtrr.type = QgsPointCloudAttribute::Char;
            ebAtrr.size = field.options;
            break;
          case 1:
            ebAtrr.type = QgsPointCloudAttribute::UChar;
            ebAtrr.size = 1;
            break;
          case 2:
            ebAtrr.type = QgsPointCloudAttribute::Char;
            ebAtrr.size = 1;
            break;
          case 3:
            ebAtrr.type = QgsPointCloudAttribute::UShort;
            ebAtrr.size = 2;
            break;
          case 4:
            ebAtrr.type = QgsPointCloudAttribute::Short;
            ebAtrr.size = 2;
            break;
          case 5:
            ebAtrr.type = QgsPointCloudAttribute::UInt32;
            ebAtrr.size = 4;
            break;
          case 6:
            ebAtrr.type = QgsPointCloudAttribute::Int32;
            ebAtrr.size = 4;
            break;
          case 7:
            ebAtrr.type = QgsPointCloudAttribute::UInt64;
            ebAtrr.size = 8;
            break;
          case 8:
            ebAtrr.type = QgsPointCloudAttribute::Int64;
            ebAtrr.size = 8;
            break;
          case 9:
            ebAtrr.type = QgsPointCloudAttribute::Float;
            ebAtrr.size = 4;
            break;
          case 10:
            ebAtrr.type = QgsPointCloudAttribute::Double;
            ebAtrr.size = 8;
            break;
          default:
            ebAtrr.type = QgsPointCloudAttribute::Char;
            ebAtrr.size = field.options;
            break;
        }
        int accOffset = ( extrabytesAttr.empty() ? point_record_length : extrabytesAttr.back().offset ) - ebAtrr.size;
        ebAtrr.offset = accOffset;
        extrabytesAttr.push_back( ebAtrr );
      }
      return extrabytesAttr;
    }

    //! Returns LazPerf VLR struct from LAZ file
    template<typename FileType>
    static std::pair<lazperf::eb_vlr, int> extractExtrabytesVlr( FileType &file )
    {
      lazperf::eb_vlr ebVlr;
      if ( !file.good() )
        return { ebVlr, 0 };

      auto pastFilePos = file.tellg();
      file.seekg( 0 );

      lazperf::reader::generic_file f( file );
      std::vector<char> ebData = f.vlrData( "LASF_Spec", 4 );
      ebVlr.fill( ebData.data(), ebData.size() );

      file.seekg( pastFilePos );

      return { ebVlr, f.header().point_record_length };
    }
};

///@endcond
#endif // QGSLAZDECODER_H
