/***************************************************************************
    qgsoapifitemsrequest.cpp
    ---------------------
    begin                : October 2019
    copyright            : (C) 2019 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <nlohmann/json.hpp>

using namespace nlohmann;

#include "qgslogger.h"
#include "qgsoapifitemsrequest.h"
#include "moc_qgsoapifitemsrequest.cpp"
#include "qgsoapifshareddata.h"
#include "qgsoapifutils.h"
#include "qgsproviderregistry.h"
#include "qgsvectordataprovider.h"

#include "cpl_conv.h"
#include "cpl_vsi.h"

#include <QTextCodec>

QgsOapifItemsRequest::QgsOapifItemsRequest( const QgsDataSourceUri &baseUri, const QString &url, const QString &featureFormat )
  : QgsBaseNetworkRequest( QgsAuthorizationSettings( baseUri.username(), baseUri.password(), QgsHttpHeaders(), baseUri.authConfigId() ), tr( "OAPIF" ) ), mUrl( url ), mFeatureFormat( featureFormat )
{
  // Using Qt::DirectConnection since the download might be running on a different thread.
  // In this case, the request was sent from the main thread and is executed with the main
  // thread being blocked in future.waitForFinished() so we can run code on this object which
  // lives in the main thread without risking havoc.
  connect( this, &QgsBaseNetworkRequest::downloadFinished, this, &QgsOapifItemsRequest::processReply, Qt::DirectConnection );
}

bool QgsOapifItemsRequest::request( bool synchronous, bool forceRefresh )
{
  QString acceptHeader = u"application/geo+json, application/json"_s;
  if ( !mFeatureFormat.isEmpty() )
  {
    acceptHeader = mFeatureFormat;
  }
  const bool isGeoJSON = mFeatureFormat.isEmpty() || mFeatureFormat == "application/geo+json"_L1;
  mFakeResponseHasHeaders = !isGeoJSON;
  QgsDebugMsgLevel( u" QgsOapifItemsRequest::request() start time: %1"_s.arg( time( nullptr ) ), 5 );
  if ( !sendGET( QUrl::fromEncoded( mUrl.toLatin1() ), acceptHeader, synchronous, forceRefresh ) )
  {
    emit gotResponse();
    return false;
  }
  return true;
}

QString QgsOapifItemsRequest::errorMessageWithReason( const QString &reason )
{
  return tr( "Download of items failed: %1" ).arg( reason );
}

// Remove extraneous indentation spaces from a JSON buffer
static void removeUselessSpacesFromJSONBuffer( QByteArray &buffer )
{
  int j = 0;
  bool inString = false;
  const int bufferInitialSize = buffer.size();
  char *ptr = buffer.data();
  for ( int i = 0; i < bufferInitialSize; ++i )
  {
    const char ch = ptr[i];
    if ( inString )
    {
      if ( ch == '"' )
      {
        inString = false;
      }
      else if ( ch == '\\' && i + 1 < bufferInitialSize && ptr[i + 1] == '"' )
      {
        ptr[j++] = ch;
        ++i;
      }
    }
    else
    {
      if ( ch == '"' )
      {
        inString = true;
      }
      else if ( ch == ' ' )
      {
        // strip spaces outside strings
        continue;
      }
    }
    ptr[j++] = ch;
  }
  buffer.resize( j );
}

void QgsOapifItemsRequest::processReply()
{
  QgsDebugMsgLevel( u"processReply start time: %1"_s.arg( time( nullptr ) ), 5 );
  if ( mErrorCode != QgsBaseNetworkRequest::NoError )
  {
    emit gotResponse();
    return;
  }
  QByteArray &buffer = mResponse;
  if ( buffer.isEmpty() )
  {
    mErrorMessage = tr( "empty response" );
    mErrorCode = QgsBaseNetworkRequest::ServerExceptionError;
    emit gotResponse();
    return;
  }

  const bool isGeoJSON = mFeatureFormat.isEmpty() || mFeatureFormat == "application/geo+json"_L1;
  const bool isGML = mFeatureFormat.startsWith( "application/gml+xml"_L1 );

  if ( isGeoJSON )
  {
    if ( buffer.size() <= 200 )
    {
      QgsDebugMsgLevel( u"parsing items response: "_s + buffer, 4 );
    }
    else
    {
      QgsDebugMsgLevel( u"parsing items response: "_s + buffer.left( 100 ) + u"[... snip ...]"_s + buffer.right( 100 ), 4 );
    }

    // Remove extraneous indentation spaces from the string. This helps a bit
    // improving JSON parsing performance afterwards
    QgsDebugMsgLevel( u"JSON compaction start time: %1"_s.arg( time( nullptr ) ), 5 );
    removeUselessSpacesFromJSONBuffer( buffer );
    QgsDebugMsgLevel( u"JSON compaction end time: %1"_s.arg( time( nullptr ) ), 5 );
  }

  QString extension;
  if ( mFeatureFormat == "application/flatgeobuf"_L1 )
    extension = u"fgb"_s;
  else if ( isGML )
    extension = u"gml"_s;
  else
    extension = u"json"_s;
  const QString vsimemFilename = u"/vsimem/oaipf_%1.%2"_s.arg( reinterpret_cast<quintptr>( &buffer ), QT_POINTER_SIZE * 2, 16, '0'_L1 ).arg( extension );

  VSIFCloseL( VSIFileFromMemBuffer( vsimemFilename.toUtf8().constData(), const_cast<GByte *>( reinterpret_cast<const GByte *>( buffer.constData() ) ), buffer.size(), false ) );
  QgsProviderRegistry *pReg = QgsProviderRegistry::instance();
  const QgsDataProvider::ProviderOptions providerOptions;
  QgsDebugMsgLevel( u"OGR data source open start time: %1"_s.arg( time( nullptr ) ), 5 );
  auto vectorProvider = std::unique_ptr<QgsVectorDataProvider>(
    qobject_cast<QgsVectorDataProvider *>( pReg->createProvider( "ogr", vsimemFilename, providerOptions ) )
  );
  QgsDebugMsgLevel( u"OGR data source open end time: %1"_s.arg( time( nullptr ) ), 5 );
  if ( !vectorProvider || !vectorProvider->isValid() )
  {
    VSIUnlink( vsimemFilename.toUtf8().constData() );
    VSIUnlink( CPLResetExtension( vsimemFilename.toUtf8().constData(), "gfs" ) );
    mErrorCode = QgsBaseNetworkRequest::ApplicationLevelError;
    mAppLevelError = ApplicationLevelError::JsonError;
    mErrorMessage = errorMessageWithReason( tr( "Loading of items failed" ) );
    emit gotResponse();
    return;
  }

  mGeometryAttribute = vectorProvider->geometryColumnName();
  mFields = vectorProvider->fields();
  if ( isGML )
  {
    if ( mGeometryAttribute.isEmpty() && buffer.contains( QByteArray( "bml:boreholePath" ) ) )
    {
      // Hack needed before https://github.com/OSGeo/gdal/commit/5f5f34b60a208bfe4b7c4b1ada0c0a702ddb2d28 (GDAL 3.12.1)
      mGeometryAttribute = u"boreholePath"_s;
    }

    // Field length guessed from a GML sample is not reliable
    for ( QgsField &field : mFields )
      field.setLength( 0 );
  }
  mWKBType = vectorProvider->wkbType();
  if ( mComputeBbox )
  {
    mBbox = vectorProvider->extent();
  }
  QgsDebugMsgLevel( u"OGR feature iteration start time: %1"_s.arg( time( nullptr ) ), 5 );
  auto iter = vectorProvider->getFeatures();

  int idField = -1;
  if ( !isGeoJSON )
  {
    idField = mFields.indexOf( "id"_L1 );
    // If no "id" field, then use the first field if it contains "id" in it.
    if ( idField < 0 && mFields.size() >= 1 && mFields[0].name().indexOf( "id"_L1, 0, Qt::CaseInsensitive ) )
    {
      idField = 0;
    }
  }

  while ( true )
  {
    QgsFeature f;
    if ( !iter.nextFeature( f ) )
      break;
    QString id;
    if ( idField >= 0 )
    {
      id = f.attribute( idField ).toString();
    }
    mFeatures.push_back( QgsFeatureUniqueIdPair( f, id ) );
  }
  QgsDebugMsgLevel( u"OGR feature iteration end time: %1"_s.arg( time( nullptr ) ), 5 );
  vectorProvider.reset();
  VSIUnlink( vsimemFilename.toUtf8().constData() );
  VSIUnlink( CPLResetExtension( vsimemFilename.toUtf8().constData(), "gfs" ) );

  if ( isGeoJSON )
  {
    try
    {
      QgsDebugMsgLevel( u"json::parse() start time: %1"_s.arg( time( nullptr ) ), 5 );
      const json j = json::parse( buffer.constData(), buffer.constData() + buffer.size() );
      QgsDebugMsgLevel( u"json::parse() end time: %1"_s.arg( time( nullptr ) ), 5 );
      if ( j.is_object() && j.contains( "features" ) )
      {
        const json &features = j["features"];
        if ( features.is_array() && features.size() == mFeatures.size() )
        {
          for ( size_t i = 0; i < features.size(); i++ )
          {
            const json &jFeature = features[i];
            if ( jFeature.is_object() && jFeature.contains( "id" ) )
            {
              const json &id = jFeature["id"];
              mFoundIdTopLevel = true;
              if ( id.is_string() )
              {
                mFeatures[i].second = QString::fromStdString( id.get<std::string>() );
              }
              else if ( id.is_number_integer() )
              {
                mFeatures[i].second = QString::number( id.get<qint64>() );
              }
            }
            if ( jFeature.is_object() && jFeature.contains( "properties" ) )
            {
              const json &properties = jFeature["properties"];
              if ( properties.is_object() && properties.contains( "id" ) )
              {
                mFoundIdInProperties = true;
              }
            }
          }
        }
      }

      const auto links = QgsOAPIFJson::parseLinks( j );
      mNextUrl = QgsOAPIFJson::findLink( links, u"next"_s, { u"application/geo+json"_s } );

      if ( j.is_object() && j.contains( "numberMatched" ) )
      {
        const auto numberMatched = j["numberMatched"];
        if ( numberMatched.is_number_integer() )
        {
          mNumberMatched = numberMatched.get<int>();
        }
      }
    }
    catch ( const json::parse_error &ex )
    {
      mErrorCode = QgsBaseNetworkRequest::ApplicationLevelError;
      mAppLevelError = ApplicationLevelError::JsonError;
      mErrorMessage = errorMessageWithReason( tr( "Cannot decode JSON document: %1" ).arg( QString::fromStdString( ex.what() ) ) );
      emit gotResponse();
      return;
    }
  }
  else
  {
    // Get numberMatched and next link from HTTP response headers
    for ( const auto &headerKeyValue : mResponseHeaders )
    {
      if ( headerKeyValue.first.compare( QByteArray( "OGC-NumberMatched" ), Qt::CaseSensitivity::CaseInsensitive ) == 0 )
      {
        bool ok = false;
        const int val = QString::fromUtf8( headerKeyValue.second ).toInt( &ok );
        if ( ok )
          mNumberMatched = val;
      }
    }
    mNextUrl = QgsOAPIFGetNextLinkFromResponseHeader( mResponseHeaders, mFeatureFormat );
  }

  QgsDebugMsgLevel( u"processReply end time: %1"_s.arg( time( nullptr ) ), 5 );
  emit gotResponse();
}
