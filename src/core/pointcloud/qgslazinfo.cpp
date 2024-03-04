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
#include "qgsnetworkaccessmanager.h"
#include "qgssetrequestinitiator_p.h"

#include "lazperf/readers.hpp"

// QgsLazInfo

QgsLazInfo::QgsLazInfo() { }

uint32_t QgsLazInfo::firstVariableLengthRecord() const
{
  if ( mVersion.first == 1 && mVersion.second == 4 )
    return 375;
  if ( mVersion.first == 1 && mVersion.second == 3 )
    return 235;
  if ( mVersion.first == 1 && mVersion.second <= 2 )
    return 227;
  return 0;
}

void QgsLazInfo::parseRawHeader( char *data, uint64_t length )
{
  mIsValid = true;
  if ( std::string( data, 4 ) != "LASF" )
  {
    mError = QStringLiteral( "Supplied header is not from a LAZ file" );
    mIsValid = false;
    return;
  }
  std::istringstream file( std::string( data, length ) );
  lazperf::header14 header = lazperf::header14::create( file );
  parseHeader( header );
}

void QgsLazInfo::parseRawVlrEntries( char *data, uint64_t length )
{
  if ( !mIsValid )
    return;
  uint64_t currentOffset = 0;
  for ( uint64_t i = 0; i < ( uint64_t )mVlrCount && currentOffset < length; ++i )
  {
    lazperf::vlr_header vlrHeader;
    vlrHeader.fill( data + currentOffset, 54 );

    LazVlr vlr;
    vlr.userId = QString::fromStdString( vlrHeader.user_id );
    vlr.recordId = vlrHeader.record_id;
    vlr.data = QByteArray( data + currentOffset + 54, vlrHeader.data_length );
    mVlrVector.push_back( vlr );
    currentOffset += 54 + vlrHeader.data_length;
  }

  parseCrs();
  parseExtrabyteAttributes();
}


void QgsLazInfo::parseHeader( lazperf::header14 &header )
{
  mHeader = header;

  mScale = QgsVector3D( header.scale.x, header.scale.y, header.scale.z );
  mOffset = QgsVector3D( header.offset.x, header.offset.y, header.offset.z );
  mCreationYearDay = QPair<uint16_t, uint16_t>( header.creation.year, header.creation.day );
  mVersion = QPair<uint8_t, uint8_t>( header.version.major, header.version.minor );
  mPointFormat = header.pointFormat();

  mProjectId = QString( QByteArray( header.guid, 16 ).toHex() );
  mSystemId = QString::fromLocal8Bit( header.system_identifier, 32 );
  while ( !mSystemId.isEmpty() && mSystemId.back() == '\0' )
  {
    mSystemId.remove( mSystemId.size() - 1, 1 );
  }
  mSoftwareId = QString::fromLocal8Bit( header.generating_software, 32 ).trimmed();
  while ( !mSoftwareId.isEmpty() && mSoftwareId.back() == '\0' )
  {
    mSoftwareId.remove( mSoftwareId.size() - 1, 1 );
  }

  mMinCoords = QgsVector3D( header.minx, header.miny, header.minz );
  mMaxCoords = QgsVector3D( header.maxx, header.maxy, header.maxz );

  mVlrCount = header.vlr_count;

  parseLazAttributes();
}


void QgsLazInfo::parseCrs()
{
  // TODO: handle other kind of CRS in the laz spec
  for ( LazVlr &vlr : mVlrVector )
  {
    if ( vlr.userId.trimmed() == QLatin1String( "LASF_Projection" ) && vlr.recordId == 2112 )
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
  metadata[ QStringLiteral( "dataformat_id" ) ] = mHeader.pointFormat();
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

void QgsLazInfo::parseLazAttributes()
{
  if ( mPointFormat < 0 || mPointFormat > 10 )
  {
    QgsDebugMsgLevel( QStringLiteral( "Invalid point record format %1" ).arg( mPointFormat ), 2 );
    return;
  }
  mAttributes.push_back( QgsPointCloudAttribute( "X", QgsPointCloudAttribute::Int32 ) );
  mAttributes.push_back( QgsPointCloudAttribute( "Y", QgsPointCloudAttribute::Int32 ) );
  mAttributes.push_back( QgsPointCloudAttribute( "Z", QgsPointCloudAttribute::Int32 ) );
  mAttributes.push_back( QgsPointCloudAttribute( "Intensity", QgsPointCloudAttribute::UShort ) );
  mAttributes.push_back( QgsPointCloudAttribute( "ReturnNumber", QgsPointCloudAttribute::Char ) );
  mAttributes.push_back( QgsPointCloudAttribute( "NumberOfReturns", QgsPointCloudAttribute::Char ) );
  mAttributes.push_back( QgsPointCloudAttribute( "ScanDirectionFlag", QgsPointCloudAttribute::Char ) );
  mAttributes.push_back( QgsPointCloudAttribute( "EdgeOfFlightLine", QgsPointCloudAttribute::Char ) );
  mAttributes.push_back( QgsPointCloudAttribute( "Classification", QgsPointCloudAttribute::UChar ) );
  mAttributes.push_back( QgsPointCloudAttribute( "ScanAngleRank", QgsPointCloudAttribute::Short ) );
  mAttributes.push_back( QgsPointCloudAttribute( "UserData", QgsPointCloudAttribute::UChar ) );
  mAttributes.push_back( QgsPointCloudAttribute( "PointSourceId", QgsPointCloudAttribute::UShort ) );
  mAttributes.push_back( QgsPointCloudAttribute( "Synthetic", QgsPointCloudAttribute::UChar ) );
  mAttributes.push_back( QgsPointCloudAttribute( "KeyPoint", QgsPointCloudAttribute::UChar ) );
  mAttributes.push_back( QgsPointCloudAttribute( "Withheld", QgsPointCloudAttribute::UChar ) );
  mAttributes.push_back( QgsPointCloudAttribute( "Overlap", QgsPointCloudAttribute::UChar ) );

  if ( mPointFormat == 6 || mPointFormat == 7 || mPointFormat == 8 || mPointFormat == 9 || mPointFormat == 10 )
  {
    mAttributes.push_back( QgsPointCloudAttribute( "ScannerChannel", QgsPointCloudAttribute::Char ) );
  }
  if ( mPointFormat != 0 && mPointFormat != 2 )
  {
    mAttributes.push_back( QgsPointCloudAttribute( "GpsTime", QgsPointCloudAttribute::Double ) );
  }
  if ( mPointFormat == 2 || mPointFormat == 3 || mPointFormat == 5 || mPointFormat == 7 || mPointFormat == 8 || mPointFormat == 10 )
  {
    mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Red" ), QgsPointCloudAttribute::UShort ) );
    mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Green" ), QgsPointCloudAttribute::UShort ) );
    mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Blue" ), QgsPointCloudAttribute::UShort ) );
  }
  if ( mPointFormat == 8 || mPointFormat == 10 )
  {
    mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Infrared" ), QgsPointCloudAttribute::UShort ) );
  }
  // Note: wave packet attributes are not handled and are unreadable
}

void QgsLazInfo::parseExtrabyteAttributes()
{
  QByteArray ebVlrRaw = vlrData( "LASF_Spec", 4 );
  mExtrabyteAttributes = QgsLazInfo::parseExtrabytes( ebVlrRaw.data(), ebVlrRaw.size(), mHeader.point_record_length );

  for ( QgsLazInfo::ExtraBytesAttributeDetails attr : mExtrabyteAttributes )
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

QgsLazInfo QgsLazInfo::fromFile( std::ifstream &file )
{
  QgsLazInfo lazInfo;

  char headerRawData[ 375 ];
  file.seekg( 0 );
  file.read( headerRawData, 375 );
  lazInfo.parseRawHeader( headerRawData, 375 );

  int vlrDataSize = lazInfo.firstPointRecordOffset() - lazInfo.firstVariableLengthRecord();
  std::unique_ptr<char[]> vlrEntriesRawData( new char[ vlrDataSize ] );
  file.seekg( lazInfo.firstVariableLengthRecord() );
  file.read( vlrEntriesRawData.get(), vlrDataSize );
  lazInfo.parseRawVlrEntries( vlrEntriesRawData.get(), vlrDataSize );

  return lazInfo;
}

QgsLazInfo QgsLazInfo::fromUrl( QUrl &url )
{
  QgsLazInfo lazInfo;

  // Fetch header data
  {
    QNetworkRequest nr( url );
    QgsSetRequestInitiatorClass( nr, QStringLiteral( "QgsLazInfo" ) );
    nr.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork );
    nr.setAttribute( QNetworkRequest::CacheSaveControlAttribute, false );
    nr.setRawHeader( "Range", "bytes=0-374" );
    QgsBlockingNetworkRequest req;
    QgsBlockingNetworkRequest::ErrorCode errCode = req.get( nr );
    if ( errCode != QgsBlockingNetworkRequest::NoError )
    {
      QgsDebugError( QStringLiteral( "Request failed: " ) + url.toString() );

      if ( req.reply().attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt() == 200 )
      {
        lazInfo.mError = req.errorMessage();
      }
      else
      {
        lazInfo.mError = QStringLiteral( "Range query 0-374 to \"%1\" failed: \"%2\"" ).arg( url.toString() ).arg( req.errorMessage() );
      }
      return lazInfo;
    }

    const QgsNetworkReplyContent reply = req.reply();

    QByteArray lazHeaderData = reply.content();

    lazInfo.parseRawHeader( lazHeaderData.data(), lazHeaderData.size() );
  }

  // Fetch VLR data
  {
    QNetworkRequest nr( url );
    QgsSetRequestInitiatorClass( nr, QStringLiteral( "QgsLazInfo" ) );
    nr.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork );
    nr.setAttribute( QNetworkRequest::CacheSaveControlAttribute, false );
    uint32_t firstVlrOffset = lazInfo.firstVariableLengthRecord();
    QByteArray vlrRequestRange = QStringLiteral( "bytes=%1-%2" ).arg( firstVlrOffset ).arg( lazInfo.firstPointRecordOffset() - 1 ).toLocal8Bit();
    nr.setRawHeader( "Range", vlrRequestRange );
    QgsBlockingNetworkRequest req;
    QgsBlockingNetworkRequest::ErrorCode errCode = req.get( nr );
    if ( errCode != QgsBlockingNetworkRequest::NoError )
    {
      QgsDebugError( QStringLiteral( "Request failed: " ) + url.toString() );

      lazInfo.mError = QStringLiteral( "Range query %1-%2 to \"%3\" failed: \"%4\"" ).arg( firstVlrOffset ).arg( lazInfo.firstPointRecordOffset() - 1 )
                       .arg( url.toString() ).arg( req.errorMessage() );
      return lazInfo;
    }
    QByteArray vlrDataRaw = req.reply().content();

    lazInfo.parseRawVlrEntries( vlrDataRaw.data(), vlrDataRaw.size() );
  }

  return lazInfo;
}
