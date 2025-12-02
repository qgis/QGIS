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

#include "cpl_vsi.h"

#include <QRegularExpression>
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
  QString acceptHeader = QStringLiteral( "application/geo+json, application/json" );
  if ( !mFeatureFormat.isEmpty() )
  {
    acceptHeader = mFeatureFormat;
  }
  const bool isGeoJSON = mFeatureFormat.isEmpty() || mFeatureFormat == QStringLiteral( "application/geo+json" );
  mFakeResponseHasHeaders = !isGeoJSON;
  QgsDebugMsgLevel( QStringLiteral( " QgsOapifItemsRequest::request() start time: %1" ).arg( time( nullptr ) ), 5 );
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
  QgsDebugMsgLevel( QStringLiteral( "processReply start time: %1" ).arg( time( nullptr ) ), 5 );
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

  const bool isGeoJSON = mFeatureFormat.isEmpty() || mFeatureFormat == QStringLiteral( "application/geo+json" );

  if ( isGeoJSON )
  {
    if ( buffer.size() <= 200 )
    {
      QgsDebugMsgLevel( QStringLiteral( "parsing items response: " ) + buffer, 4 );
    }
    else
    {
      QgsDebugMsgLevel( QStringLiteral( "parsing items response: " ) + buffer.left( 100 ) + QStringLiteral( "[... snip ...]" ) + buffer.right( 100 ), 4 );
    }

    // Remove extraneous indentation spaces from the string. This helps a bit
    // improving JSON parsing performance afterwards
    QgsDebugMsgLevel( QStringLiteral( "JSON compaction start time: %1" ).arg( time( nullptr ) ), 5 );
    removeUselessSpacesFromJSONBuffer( buffer );
    QgsDebugMsgLevel( QStringLiteral( "JSON compaction end time: %1" ).arg( time( nullptr ) ), 5 );
  }

  const QString vsimemFilename = mFeatureFormat == QStringLiteral( "application/flatgeobuf" ) ? QStringLiteral( "/vsimem/oaipf_%1.fgb" ).arg( reinterpret_cast<quintptr>( &buffer ), QT_POINTER_SIZE * 2, 16, QLatin1Char( '0' ) ) : QStringLiteral( "/vsimem/oaipf_%1.json" ).arg( reinterpret_cast<quintptr>( &buffer ), QT_POINTER_SIZE * 2, 16, QLatin1Char( '0' ) );
  VSIFCloseL( VSIFileFromMemBuffer( vsimemFilename.toUtf8().constData(), const_cast<GByte *>( reinterpret_cast<const GByte *>( buffer.constData() ) ), buffer.size(), false ) );
  QgsProviderRegistry *pReg = QgsProviderRegistry::instance();
  const QgsDataProvider::ProviderOptions providerOptions;
  QgsDebugMsgLevel( QStringLiteral( "OGR data source open start time: %1" ).arg( time( nullptr ) ), 5 );
  auto vectorProvider = std::unique_ptr<QgsVectorDataProvider>(
    qobject_cast<QgsVectorDataProvider *>( pReg->createProvider( "ogr", vsimemFilename, providerOptions ) )
  );
  QgsDebugMsgLevel( QStringLiteral( "OGR data source open end time: %1" ).arg( time( nullptr ) ), 5 );
  if ( !vectorProvider || !vectorProvider->isValid() )
  {
    VSIUnlink( vsimemFilename.toUtf8().constData() );
    mErrorCode = QgsBaseNetworkRequest::ApplicationLevelError;
    mAppLevelError = ApplicationLevelError::JsonError;
    mErrorMessage = errorMessageWithReason( tr( "Loading of items failed" ) );
    emit gotResponse();
    return;
  }

  mFields = vectorProvider->fields();
  mWKBType = vectorProvider->wkbType();
  if ( mComputeBbox )
  {
    mBbox = vectorProvider->extent();
  }
  QgsDebugMsgLevel( QStringLiteral( "OGR feature iteration start time: %1" ).arg( time( nullptr ) ), 5 );
  auto iter = vectorProvider->getFeatures();

  int idField = -1;
  if ( !isGeoJSON )
  {
    idField = mFields.indexOf( QStringLiteral( "id" ) );
    // If no "id" field, then use the first field if it contains "id" in it.
    if ( idField < 0 && mFields.size() >= 1 && mFields[0].name().indexOf( QLatin1String( "id" ), 0, Qt::CaseInsensitive ) )
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
  QgsDebugMsgLevel( QStringLiteral( "OGR feature iteration end time: %1" ).arg( time( nullptr ) ), 5 );
  vectorProvider.reset();
  VSIUnlink( vsimemFilename.toUtf8().constData() );

  if ( isGeoJSON )
  {
    try
    {
      QgsDebugMsgLevel( QStringLiteral( "json::parse() start time: %1" ).arg( time( nullptr ) ), 5 );
      const json j = json::parse( buffer.constData(), buffer.constData() + buffer.size() );
      QgsDebugMsgLevel( QStringLiteral( "json::parse() end time: %1" ).arg( time( nullptr ) ), 5 );
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
      mNextUrl = QgsOAPIFJson::findLink( links, QStringLiteral( "next" ), { QStringLiteral( "application/geo+json" ) } );

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
      else if ( headerKeyValue.first.compare( QByteArray( "Link" ), Qt::CaseSensitivity::CaseInsensitive ) == 0 )
      {
        // Parse stuff like:
        //  <https://ogc-api.nrw.de/lika/v1/collections/flurstueck/items?f=html>; rel="alternate"; title="This document as HTML"; type="text/html", <https://ogc-api.nrw.de/lika/v1/collections/flurstueck/items?f=fgb&offset=10>; rel="next"; title="Next page"; type="application/flatgeobuf"

        // Split on commas, except when they are in double quotes or between <...>, and skip padding space before/after separator
        const thread_local QRegularExpression splitOnComma( R"(\s*,\s*(?=(?:[^"<]|"[^"]*"|<[^>]*>)*$))" );
        const QStringList links = QString::fromUtf8( headerKeyValue.second ).split( splitOnComma );
        for ( const QString &link : std::as_const( links ) )
        {
          if ( link.isEmpty() || link[0] != QLatin1Char( '<' ) )
            continue;
          const int idxClosingBracket = static_cast<int>( link.indexOf( QLatin1Char( '>' ) ) );
          if ( idxClosingBracket < 0 )
            continue;
          const QString href = link.mid( 1, idxClosingBracket - 1 );
          const int idxSemiColon = static_cast<int>( link.indexOf( QLatin1Char( ';' ), idxClosingBracket ) );
          if ( idxSemiColon < 0 )
            continue;
          // Split on semi-colon, except when they are in double quotes, and skip padding space before/after separator
          const thread_local QRegularExpression splitOnSemiColon( R"(\s*;\s*(?=(?:[^"]*"[^"]*")*[^"]*$))" );
          const QStringList linkParts = link.mid( idxSemiColon + 1 ).split( splitOnSemiColon );
          QString rel, type;
          for ( const QString &linkPart : std::as_const( linkParts ) )
          {
            // Split on equal, except when they are in double quotes, and skip padding space before/after separator
            const thread_local QRegularExpression splitOnEqual( R"(\s*\=\s*(?=(?:[^"]*"[^"]*")*[^"]*$))" );
            const QStringList keyValue = linkPart.split( splitOnEqual );
            if ( keyValue.size() == 2 )
            {
              const QString key = keyValue[0].trimmed();
              QString value = keyValue[1].trimmed();
              if ( !value.isEmpty() && value[0] == QLatin1Char( '"' ) && value.back() == QLatin1Char( '"' ) )
              {
                value = value.mid( 1, value.size() - 2 );
              }
              if ( key == QLatin1String( "rel" ) )
              {
                rel = value;
              }
              else if ( key == QLatin1String( "type" ) )
              {
                type = value;
              }
            }
          }
          if ( rel == QLatin1String( "next" ) && type == mFeatureFormat )
          {
            mNextUrl = href;
          }
        }
      }
    }
  }

  QgsDebugMsgLevel( QStringLiteral( "processReply end time: %1" ).arg( time( nullptr ) ), 5 );
  emit gotResponse();
}
