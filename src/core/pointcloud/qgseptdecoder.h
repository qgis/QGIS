/***************************************************************************
                         qgseptdecoder.h
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEPTDECODER_H
#define QGSEPTDECODER_H


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

namespace QgsEptDecoder
{
  struct ExtraBytesAttributeDetails
  {
    QString attribute;
    QgsPointCloudAttribute::DataType type;
    int size;
    int offset;
  };

  QgsPointCloudBlock *decompressBinary( const QString &filename, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset, QgsPointCloudExpression &filterExpression );
  QgsPointCloudBlock *decompressBinary( const QByteArray &data, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset, QgsPointCloudExpression &filterExpression );
  QgsPointCloudBlock *decompressZStandard( const QString &filename, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset, QgsPointCloudExpression &filterExpression );
  QgsPointCloudBlock *decompressZStandard( const QByteArray &data, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset, QgsPointCloudExpression &filterExpression );
  QgsPointCloudBlock *decompressLaz( const QString &filename, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset, QgsPointCloudExpression &filterExpression );
  QgsPointCloudBlock *decompressLaz( const QByteArray &data, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset, QgsPointCloudExpression &filterExpression );

  //! Returns the list of extrabytes attributes with their type, size and offsets represented in the LAS file
  template<typename FileType>
  QVector<QgsEptDecoder::ExtraBytesAttributeDetails> readExtraByteAttributes( FileType &file )
  {
    if ( !file.good() )
      return QVector<QgsEptDecoder::ExtraBytesAttributeDetails>();

    auto pastFilePos = file.tellg();

    file.seekg( 0 );

    lazperf::reader::generic_file f( file );
    int point_record_length = f.header().point_record_length;

    QVector<ExtraBytesAttributeDetails> extrabytesAttr;

    std::vector<char> ebData = f.vlrData( "LASF_Spec", 4 );
    lazperf::eb_vlr ebVlr;
    ebVlr.fill( ebData.data(), ebData.size() );
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

    file.seekg( pastFilePos );

    return extrabytesAttr;
  }
};

///@endcond
#endif // QGSEPTDECODER_H
