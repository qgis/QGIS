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
#include "qgsoapifutils.h"
#include "qgsproviderregistry.h"

#include "cpl_vsi.h"

#include <QTextCodec>

QgsOapifItemsRequest::QgsOapifItemsRequest( const QgsDataSourceUri &baseUri, const QString &url )
  : QgsBaseNetworkRequest( QgsAuthorizationSettings( baseUri.username(), baseUri.password(), baseUri.authConfigId() ), tr( "OAPIF" ) ), mUrl( url )
{
  // Using Qt::DirectConnection since the download might be running on a different thread.
  // In this case, the request was sent from the main thread and is executed with the main
  // thread being blocked in future.waitForFinished() so we can run code on this object which
  // lives in the main thread without risking havoc.
  connect( this, &QgsBaseNetworkRequest::downloadFinished, this, &QgsOapifItemsRequest::processReply, Qt::DirectConnection );
}

bool QgsOapifItemsRequest::request( bool synchronous, bool forceRefresh )
{
  QgsDebugMsgLevel( QStringLiteral( " QgsOapifItemsRequest::request() start time: %1" ).arg( time( nullptr ) ), 5 );
  if ( !sendGET( QUrl::fromEncoded( mUrl.toLatin1() ), QString( "application/geo+json, application/json" ), synchronous, forceRefresh ) )
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

  const QString vsimemFilename = QStringLiteral( "/vsimem/oaipf_%1.json" ).arg( reinterpret_cast<quintptr>( &buffer ), QT_POINTER_SIZE * 2, 16, QLatin1Char( '0' ) );
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
  while ( true )
  {
    QgsFeature f;
    if ( !iter.nextFeature( f ) )
      break;
    mFeatures.push_back( QgsFeatureUniqueIdPair( f, QString() ) );
  }
  QgsDebugMsgLevel( QStringLiteral( "OGR feature iteration end time: %1" ).arg( time( nullptr ) ), 5 );
  vectorProvider.reset();
  VSIUnlink( vsimemFilename.toUtf8().constData() );

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

  QgsDebugMsgLevel( QStringLiteral( "processReply end time: %1" ).arg( time( nullptr ) ), 5 );
  emit gotResponse();
}
