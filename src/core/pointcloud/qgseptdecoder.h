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

#include "laz-perf/io.hpp"
#include "laz-perf/common/common.hpp"

///@cond PRIVATE
#define SIP_NO_FILE

#include <QString>

namespace QgsEptDecoder
{
  struct ExtraBytesAttributeDetails
  {
    ExtraBytesAttributeDetails( const QString &attribute, QgsPointCloudAttribute::DataType type, int size, int offset )
      : attribute( attribute )
      , type( type )
      , size( size )
      , offset( offset )
    {}

    QString attribute;
    QgsPointCloudAttribute::DataType type;
    int size;
    int offset;
  };

  QgsPointCloudBlock *decompressBinary( const QString &filename, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset );
  QgsPointCloudBlock *decompressBinary( const QByteArray &data, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset );
  QgsPointCloudBlock *decompressZStandard( const QString &filename, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset );
  QgsPointCloudBlock *decompressZStandard( const QByteArray &data, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset );
  QgsPointCloudBlock *decompressLaz( const QString &filename, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset );
  QgsPointCloudBlock *decompressLaz( const QByteArray &data, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset );

  //! Returns the list of extrabytes attributes with their type, size and offsets represented in the LAS file
  template<typename FileType>
  QVector<QgsEptDecoder::ExtraBytesAttributeDetails> readExtraByteAttributes( FileType &file )
  {
    if ( !file.good() )
      return QVector<QgsEptDecoder::ExtraBytesAttributeDetails>();

    auto pastFilePos = file.tellg();

    file.seekg( 0 );

    laszip::io::reader::basic_file<FileType> f( file );

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

    file.seekg( f.get_header().header_size );
    for ( unsigned int i = 0; i < f.get_header().vlr_count && file.good() && !file.eof(); ++i )
    {
      VlrHeader vlrHeader;
      file.read( ( char * )&vlrHeader, sizeof( VlrHeader ) );
      file.seekg( vlrHeader.record_length, std::ios::cur );
      if ( std::equal( vlrHeader.user_id, vlrHeader.user_id + 9, "LASF_Spec" ) && vlrHeader.record_id == 4 )
      {
        extraBytesVlrHeader = vlrHeader;
        extraBytesDescriptorsOffset = f.get_header().header_size + sizeof( VlrHeader );
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

    for ( ExtraByteDescriptor &eb : extraBytes )
    {
      int accOffset = extrabytesAttr.empty() ? 0 : extrabytesAttr.back().offset + extrabytesAttr.back().size;
      // TODO: manage other data types
      switch ( eb.data_type )
      {
        case 0:
          extrabytesAttr.push_back( ExtraBytesAttributeDetails( QString::fromStdString( eb.name ), QgsPointCloudAttribute::Char, eb.options, accOffset ) );
          break;
        case 1:
          extrabytesAttr.push_back( ExtraBytesAttributeDetails( QString::fromStdString( eb.name ), QgsPointCloudAttribute::Char, 1, accOffset ) );
          break;
        case 2:
          extrabytesAttr.push_back( ExtraBytesAttributeDetails( QString::fromStdString( eb.name ), QgsPointCloudAttribute::Char, 1, accOffset ) );
          break;
        case 3:
          extrabytesAttr.push_back( ExtraBytesAttributeDetails( QString::fromStdString( eb.name ), QgsPointCloudAttribute::UShort, 2, accOffset ) );
          break;
        case 4:
          extrabytesAttr.push_back( ExtraBytesAttributeDetails( QString::fromStdString( eb.name ), QgsPointCloudAttribute::Short, 2, accOffset ) );
          break;
        case 5:
          extrabytesAttr.push_back( ExtraBytesAttributeDetails( QString::fromStdString( eb.name ), QgsPointCloudAttribute::Int32, 4, accOffset ) );
          break;
        case 6:
          extrabytesAttr.push_back( ExtraBytesAttributeDetails( QString::fromStdString( eb.name ), QgsPointCloudAttribute::Int32, 4, accOffset ) );
          break;
        case 7:
          extrabytesAttr.push_back( ExtraBytesAttributeDetails( QString::fromStdString( eb.name ), QgsPointCloudAttribute::Int32, 8, accOffset ) );
          break;
        case 8:
          extrabytesAttr.push_back( ExtraBytesAttributeDetails( QString::fromStdString( eb.name ), QgsPointCloudAttribute::Int32, 8, accOffset ) );
          break;
        case 9:
          extrabytesAttr.push_back( ExtraBytesAttributeDetails( QString::fromStdString( eb.name ), QgsPointCloudAttribute::Float, 4, accOffset ) );
          break;
        case 10:
          extrabytesAttr.push_back( ExtraBytesAttributeDetails( QString::fromStdString( eb.name ), QgsPointCloudAttribute::Double, 8, accOffset ) );
          break;
        default:
          break;
      }
    }

    file.seekg( pastFilePos );

    return extrabytesAttr;
  }
};

///@endcond
#endif // QGSEPTDECODER_H
