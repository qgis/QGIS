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

    // Read VLR stuff

    struct VlrHeader
    {
      unsigned short reserved;
      char user_id[16];
      unsigned short record_id;
      unsigned short record_length;
      char desc[32];
    };

    struct ExtraByteDescriptor
    {
      unsigned char reserved[2]; // 2 bytes
      unsigned char data_type; // 1 byte
      unsigned char options; // 1 byte
      char name[32]; // 32 bytes
      unsigned char unused[4]; // 4 bytes
      unsigned char no_data[8]; // 8 bytes
      unsigned char deprecated1[16]; // 16 bytes
      unsigned char min[8]; // 8 bytes
      unsigned char deprecated2[16]; // 16 bytes
      unsigned char max[8]; // 8 bytes
      unsigned char deprecated3[16]; // 16 bytes
      unsigned char scale[8]; // 8 bytes
      unsigned char deprecated4[16]; // 16 bytes
      double offset; // 8 bytes
      unsigned char deprecated5[16]; // 16 bytes
      char description[32]; // 32 bytes
    };

    QVector<ExtraByteDescriptor> extraBytes;
    QVector<ExtraBytesAttributeDetails> extrabytesAttr;

    VlrHeader extraBytesVlrHeader;
    int extraBytesDescriptorsOffset = -1;

    file.seekg( f.header().header_size );
    for ( unsigned int i = 0; i < f.header().vlr_count && file.good() && !file.eof(); ++i )
    {
      VlrHeader vlrHeader;
      file.read( ( char * )&vlrHeader, sizeof( VlrHeader ) );
      file.seekg( vlrHeader.record_length, std::ios::cur );
      if ( std::equal( vlrHeader.user_id, vlrHeader.user_id + 9, "LASF_Spec" ) && vlrHeader.record_id == 4 )
      {
        extraBytesVlrHeader = vlrHeader;
        extraBytesDescriptorsOffset = f.header().header_size + sizeof( VlrHeader );
      }
    }

    // Read VLR fields
    if ( extraBytesDescriptorsOffset != -1 )
    {
      file.seekg( extraBytesDescriptorsOffset );
      int n_descriptors = extraBytesVlrHeader.record_length / sizeof( ExtraByteDescriptor );
      for ( int i = 0; i < n_descriptors; ++i )
      {
        ExtraByteDescriptor ebd;
        file.read( ( char * )&ebd, sizeof( ExtraByteDescriptor ) );
        extraBytes.push_back( ebd );
      }
    }

    for ( int i = ( int )extraBytes.size() - 1; i >= 0; --i )
    {
      ExtraByteDescriptor eb = extraBytes[i];
      ExtraBytesAttributeDetails ebAtrr;
      ebAtrr.attribute = QString::fromStdString( eb.name );
      switch ( eb.data_type )
      {
        case 0:
          ebAtrr.type = QgsPointCloudAttribute::Char;
          ebAtrr.size = eb.options;
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
          ebAtrr.size = eb.options;
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
