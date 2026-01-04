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

#include "lazperf/vlr.hpp"
#include "qgsblockingnetworkrequest.h"
#include "qgslogger.h"
#include "qgssetrequestinitiator_p.h"

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
    mError = u"Supplied header is not from a LAZ file"_s;
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
    if ( vlr.userId.trimmed() == "LASF_Projection"_L1 && vlr.recordId == 2112 )
    {
      mCrs = QgsCoordinateReferenceSystem::fromWkt( QString::fromStdString( vlr.data.toStdString() ) );
      break;
    }
  }
}

QVariantMap QgsLazInfo::toMetadata() const
{
  QVariantMap metadata;
  metadata[ u"creation_year"_s ] = mHeader.creation.year;
  metadata[ u"creation_day"_s ] = mHeader.creation.day;
  metadata[ u"major_version"_s ] = mHeader.version.major;
  metadata[ u"minor_version"_s ] = mHeader.version.minor;
  metadata[ u"dataformat_id"_s ] = mHeader.pointFormat();
  metadata[ u"scale_x"_s ] = mScale.x();
  metadata[ u"scale_y"_s ] = mScale.y();
  metadata[ u"scale_z"_s ] = mScale.z();
  metadata[ u"offset_x"_s ] = mOffset.x();
  metadata[ u"offset_y"_s ] = mOffset.y();
  metadata[ u"offset_z"_s ] = mOffset.z();
  metadata[ u"project_id"_s ] = QString( QByteArray( mHeader.guid, 16 ).toHex() );
  metadata[ u"system_id"_s ] = QString::fromLocal8Bit( mHeader.system_identifier, 32 );
  metadata[ u"software_id"_s ] = QString::fromLocal8Bit( mHeader.generating_software, 32 );
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
    QgsDebugMsgLevel( u"Invalid point record format %1"_s.arg( mPointFormat ), 2 );
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
  mAttributes.push_back( QgsPointCloudAttribute( "ScanAngleRank", QgsPointCloudAttribute::Float ) );
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
    mAttributes.push_back( QgsPointCloudAttribute( u"Red"_s, QgsPointCloudAttribute::UShort ) );
    mAttributes.push_back( QgsPointCloudAttribute( u"Green"_s, QgsPointCloudAttribute::UShort ) );
    mAttributes.push_back( QgsPointCloudAttribute( u"Blue"_s, QgsPointCloudAttribute::UShort ) );
  }
  if ( mPointFormat == 8 || mPointFormat == 10 )
  {
    mAttributes.push_back( QgsPointCloudAttribute( u"Infrared"_s, QgsPointCloudAttribute::UShort ) );
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

QgsLazInfo QgsLazInfo::fromUrl( QUrl &url, const QString &authcfg )
{
  QgsLazInfo lazInfo;

  // Fetch header data
  {
    QNetworkRequest nr( url );
    QgsSetRequestInitiatorClass( nr, u"QgsLazInfo"_s );
    nr.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork );
    nr.setAttribute( QNetworkRequest::CacheSaveControlAttribute, false );
    nr.setRawHeader( "Range", "bytes=0-374" );
    QgsBlockingNetworkRequest req;
    req.setAuthCfg( authcfg );
    QgsBlockingNetworkRequest::ErrorCode errCode = req.get( nr );
    if ( errCode != QgsBlockingNetworkRequest::NoError )
    {
      QgsDebugError( u"Request failed: "_s + url.toString() );

      if ( req.reply().attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt() == 200 )
      {
        lazInfo.mError = req.errorMessage();
      }
      else
      {
        lazInfo.mError = u"Range query 0-374 to \"%1\" failed: \"%2\""_s.arg( url.toString() ).arg( req.errorMessage() );
      }
      return lazInfo;
    }

    const QgsNetworkReplyContent reply = req.reply();

    QByteArray lazHeaderData = reply.content();

    lazInfo.parseRawHeader( lazHeaderData.data(), lazHeaderData.size() );

    // If request was redirected, let's update our url for all next calls
    const QUrl requestedUrl = reply.request().url();
    if ( requestedUrl != url && authcfg.isEmpty() )
    {
      url.setUrl( requestedUrl.toString() );
    }
  }

  // Fetch VLR data
  {
    QNetworkRequest nr( url );
    QgsSetRequestInitiatorClass( nr, u"QgsLazInfo"_s );
    nr.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork );
    nr.setAttribute( QNetworkRequest::CacheSaveControlAttribute, false );
    uint32_t firstVlrOffset = lazInfo.firstVariableLengthRecord();
    QByteArray vlrRequestRange = u"bytes=%1-%2"_s.arg( firstVlrOffset ).arg( lazInfo.firstPointRecordOffset() - 1 ).toLocal8Bit();
    nr.setRawHeader( "Range", vlrRequestRange );
    QgsBlockingNetworkRequest req;
    req.setAuthCfg( authcfg );
    QgsBlockingNetworkRequest::ErrorCode errCode = req.get( nr );
    if ( errCode != QgsBlockingNetworkRequest::NoError )
    {
      QgsDebugError( u"Request failed: "_s + url.toString() );

      lazInfo.mError = u"Range query %1-%2 to \"%3\" failed: \"%4\""_s.arg( firstVlrOffset ).arg( lazInfo.firstPointRecordOffset() - 1 )
                       .arg( url.toString() ).arg( req.errorMessage() );
      return lazInfo;
    }
    QByteArray vlrDataRaw = req.reply().content();

    lazInfo.parseRawVlrEntries( vlrDataRaw.data(), vlrDataRaw.size() );
  }

  return lazInfo;
}
