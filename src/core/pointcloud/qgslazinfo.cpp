/***************************************************************************
                         qgslazinfo.cpp
                         --------------------
    begin                : April 2022
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

#include "qgslazinfo.h"

#include "qgslogger.h"
#include "qgsblockingnetworkrequest.h"
#include "lazperf/readers.hpp"

// QgsLazInfo

QgsLazInfo::QgsLazInfo( std::ifstream &file )
{
  if ( !file.is_open() || !file.good() )
  {
    mIsValid = false;
    return;
  }
  // TODO: add exceptions handling
  long long previousPos = file.peek();
  lazperf::reader::generic_file f( file );
  mHeader = f.header();

  parseHeader( mHeader );

  file.seekg( 375 );
  std::unique_ptr<char> vlrDataRaw( new char[ mPointRecordsOffset - 375 ] );
  file.read( vlrDataRaw.get(), mPointRecordsOffset - 375 );

  parseVlrData( vlrDataRaw.get(), mPointRecordsOffset - 375 );

  parseCrs();

  parseAttributes();

  file.seekg( previousPos );
  mIsValid = true;
}

QgsLazInfo::QgsLazInfo( QUrl &url )
{

  // Fetch header data
  {
    QNetworkRequest nr( url );
    nr.setRawHeader( "Range", "bytes=0-374" );
    QgsBlockingNetworkRequest req;
    QgsBlockingNetworkRequest::ErrorCode errCode = req.get( nr );
    if ( errCode != QgsBlockingNetworkRequest::NoError )
    {
      QgsDebugMsg( QStringLiteral( "Request failed: " ) + url.toString() );
      mIsValid = false;
      return;
    }
    const QgsNetworkReplyContent reply = req.reply();
    QByteArray lazHeaderData = reply.content();
    std::istringstream file( lazHeaderData.toStdString() );
    mHeader = lazperf::header14::create( file );
    parseHeader( mHeader );
  }

  // Fetch VLR data
  {
    QNetworkRequest nr( url );
    QByteArray vlrRequestRange = QStringLiteral( "bytes=%1-%2" ).arg( 375 ).arg( mPointRecordsOffset - 1 ).toLocal8Bit();
    nr.setRawHeader( "Range", vlrRequestRange );
    QgsBlockingNetworkRequest req;
    QgsBlockingNetworkRequest::ErrorCode errCode = req.get( nr );
    if ( errCode != QgsBlockingNetworkRequest::NoError )
    {
      QgsDebugMsg( QStringLiteral( "Request failed: " ) + url.toString() );
      mIsValid = false;
      return;
    }
    QByteArray vlrDataRaw = req.reply().content();
    parseVlrData( vlrDataRaw.data(), vlrDataRaw.size() );
  }

  parseCrs();

  parseAttributes();

  mIsValid = true;
}

void QgsLazInfo::parseHeader( lazperf::header14 &header )
{
  mPointCount = header.point_count_14;
  mScale = QgsVector3D( header.scale.x, header.scale.y, header.scale.z );
  mOffset = QgsVector3D( header.offset.x, header.offset.y, header.offset.z );
  mCreationYearDay = QPair<uint16_t, uint16_t>( header.creation.year, header.creation.day );
  mVersion = QPair<uint8_t, uint8_t>( header.version.major, header.version.minor );
  mPointFormat = header.pointFormat();

  mProjectId = QString( QByteArray( header.guid, 16 ).toHex() );
  mSystemId = QString::fromLocal8Bit( header.system_identifier, 32 );
  mSoftwareId = QString::fromLocal8Bit( header.generating_software, 32 );

  mMinCoords = QgsVector3D( header.minx, header.miny, header.minz );
  mMaxCoords = QgsVector3D( header.maxx, header.maxy, header.maxz );

  mVlrCount = header.vlr_count;
  mPointRecordsOffset = header.point_offset;
  mPointRecordLength = header.point_record_length;
}

void QgsLazInfo::parseVlrData( char *vlrData, uint32_t size )
{
  uint64_t currentOffset = 0;
  for ( int i = 0; i < mVlrCount && currentOffset < size; ++i )
  {
    lazperf::vlr_header vlrHeader;
    vlrHeader.fill( vlrData + currentOffset, 54 );

    LazVlr vlr;
    vlr.userId = QString::fromStdString( vlrHeader.user_id );
    vlr.recordId = vlrHeader.record_id;
    vlr.data = QByteArray( vlrData + currentOffset + 54, vlrHeader.data_length );
    mVlrVector.push_back( vlr );
    currentOffset += 54 + vlrHeader.data_length;
  }
}

void QgsLazInfo::parseCrs()
{
  // TODO: handle other kind of CRS in the laz spec
  for ( LazVlr &vlr : mVlrVector )
  {
    if ( vlr.userId.trimmed() == QStringLiteral( "LASF_Projection" ) && vlr.recordId == 2112 )
    {
      mCrs = QgsCoordinateReferenceSystem::fromWkt( QString::fromStdString( vlr.data.toStdString() ) );
      break;
    }
  }
}

QVariantMap QgsLazInfo::toMetadata() const
{
  QVariantMap metadata;
  metadata[ QStringLiteral( "creation_year" ) ] = mHeader.creation.year;
  metadata[ QStringLiteral( "creation_day" ) ] = mHeader.creation.day;
  metadata[ QStringLiteral( "major_version" ) ] = mHeader.version.major;
  metadata[ QStringLiteral( "minor_version" ) ] = mHeader.version.minor;
  metadata[ QStringLiteral( "dataformat_id" ) ] = mHeader.point_format_id;
  metadata[ QStringLiteral( "scale_x" ) ] = mScale.x();
  metadata[ QStringLiteral( "scale_y" ) ] = mScale.y();
  metadata[ QStringLiteral( "scale_z" ) ] = mScale.z();
  metadata[ QStringLiteral( "offset_x" ) ] = mOffset.x();
  metadata[ QStringLiteral( "offset_y" ) ] = mOffset.y();
  metadata[ QStringLiteral( "offset_z" ) ] = mOffset.z();
  metadata[ QStringLiteral( "project_id" ) ] = QString( QByteArray( mHeader.guid, 16 ).toHex() );
  metadata[ QStringLiteral( "system_id" ) ] = QString::fromLocal8Bit( mHeader.system_identifier, 32 );
  metadata[ QStringLiteral( "software_id" ) ] = QString::fromLocal8Bit( mHeader.generating_software, 32 );
  return metadata;
}

QByteArray QgsLazInfo::vlrData( QString userId, int recordId )
{
  for ( LazVlr vlr : mVlrVector )
  {
    if ( vlr.userId == userId && vlr.recordId == recordId )
    {
      return vlr.data;
    }
  }
  return QByteArray();
}

QVector<QgsLazInfo::ExtraBytesAttributeDetails> QgsLazInfo::extrabytes()
{
  QByteArray ebVlrRaw = vlrData( "LASF_Spec", 4 );
  return QgsLazInfo::parseExtrabytes( ebVlrRaw.data(), ebVlrRaw.size(), mPointRecordLength );
}

void QgsLazInfo::parseAttributes()
{
  // Attributes for COPC format
  // COPC supports only PDRF 6, 7 and 8

  mAttributes.push_back( QgsPointCloudAttribute( "X", QgsPointCloudAttribute::Int32 ) );
  mAttributes.push_back( QgsPointCloudAttribute( "Y", QgsPointCloudAttribute::Int32 ) );
  mAttributes.push_back( QgsPointCloudAttribute( "Z", QgsPointCloudAttribute::Int32 ) );
  mAttributes.push_back( QgsPointCloudAttribute( "Intensity", QgsPointCloudAttribute::UShort ) );
  mAttributes.push_back( QgsPointCloudAttribute( "ReturnNumber", QgsPointCloudAttribute::Char ) );
  mAttributes.push_back( QgsPointCloudAttribute( "NumberOfReturns", QgsPointCloudAttribute::Char ) );
  mAttributes.push_back( QgsPointCloudAttribute( "ScanDirectionFlag", QgsPointCloudAttribute::Char ) );
  mAttributes.push_back( QgsPointCloudAttribute( "EdgeOfFlightLine", QgsPointCloudAttribute::Char ) );
  mAttributes.push_back( QgsPointCloudAttribute( "Classification", QgsPointCloudAttribute::Char ) );
  mAttributes.push_back( QgsPointCloudAttribute( "ScanAngleRank", QgsPointCloudAttribute::Short ) );
  mAttributes.push_back( QgsPointCloudAttribute( "UserData", QgsPointCloudAttribute::Char ) );
  mAttributes.push_back( QgsPointCloudAttribute( "PointSourceId", QgsPointCloudAttribute::UShort ) );
  mAttributes.push_back( QgsPointCloudAttribute( "GpsTime", QgsPointCloudAttribute::Double ) );

  switch ( mPointFormat )
  {
    case 6:
      break;
    case 7:
      mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Red" ), QgsPointCloudAttribute::UShort ) );
      mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Green" ), QgsPointCloudAttribute::UShort ) );
      mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Blue" ), QgsPointCloudAttribute::UShort ) );
      break;
    case 8:
      mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Red" ), QgsPointCloudAttribute::UShort ) );
      mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Green" ), QgsPointCloudAttribute::UShort ) );
      mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Blue" ), QgsPointCloudAttribute::UShort ) );
      mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "NIR" ), QgsPointCloudAttribute::UShort ) );
      break;
    default:
      break;
  }

  QVector<QgsLazInfo::ExtraBytesAttributeDetails> extrabyteAttributes = extrabytes();
  for ( QgsLazInfo::ExtraBytesAttributeDetails attr : extrabyteAttributes )
  {
    mAttributes.push_back( QgsPointCloudAttribute( attr.attribute, attr.type ) );
  }
}

QVector<QgsLazInfo::ExtraBytesAttributeDetails> QgsLazInfo::parseExtrabytes( char *rawData, int length, int pointRecordLength )
{
  QVector<QgsLazInfo::ExtraBytesAttributeDetails> extrabyteAttributes;
  lazperf::eb_vlr ebVlr;
  ebVlr.fill( rawData, length );
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
    int accOffset = ( extrabyteAttributes.empty() ? pointRecordLength : extrabyteAttributes.back().offset ) - ebAtrr.size;
    ebAtrr.offset = accOffset;
    extrabyteAttributes.push_back( ebAtrr );
  }
  return extrabyteAttributes;
}
