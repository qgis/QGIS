/***************************************************************************
  qgsxyzvectortiledataprovider.cpp
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsxyzvectortiledataprovider.h"
#include "qgsthreadingutils.h"
#include "qgstiles.h"
#include "qgsvectortileloader.h"
#include "qgsvectortileutils.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgsmessagelog.h"
#include "qgsblockingnetworkrequest.h"
#include "qgslogger.h"

#include <QNetworkRequest>

///@cond PRIVATE

QgsXyzVectorTileDataProvider::QgsXyzVectorTileDataProvider( const QString &uri, const ProviderOptions &providerOptions, ReadFlags flags )
  : QgsVectorTileDataProvider( uri, providerOptions, flags )
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );

  mAuthCfg = dsUri.authConfigId();
  mHeaders = dsUri.httpHeaders();
}

QgsVectorTileDataProvider *QgsXyzVectorTileDataProvider::clone() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  ProviderOptions options;
  options.transformContext = transformContext();
  return new QgsXyzVectorTileDataProvider( dataSourceUri(), options, mReadFlags );
}

QString QgsXyzVectorTileDataProvider::sourcePath() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( dataSourceUri() );
  return dsUri.param( QStringLiteral( "url" ) );
}

bool QgsXyzVectorTileDataProvider::isValid() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return true;
}

QgsCoordinateReferenceSystem QgsXyzVectorTileDataProvider::crs() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) );
}

bool QgsXyzVectorTileDataProvider::supportsAsync() const
{
  return true;
}

QByteArray QgsXyzVectorTileDataProvider::readTile( const QgsTileMatrix &tileMatrix, const QgsTileXYZ &id, QgsFeedback *feedback ) const
{
  return loadFromNetwork( id, tileMatrix, sourcePath(), mAuthCfg, mHeaders, feedback );
}

QList<QgsVectorTileRawData> QgsXyzVectorTileDataProvider::readTiles( const QgsTileMatrix &tileMatrix, const QVector<QgsTileXYZ> &tiles, QgsFeedback *feedback ) const
{
  QList<QgsVectorTileRawData> rawTiles;
  rawTiles.reserve( tiles.size() );
  const QString source = sourcePath();
  for ( QgsTileXYZ id : std::as_const( tiles ) )
  {
    if ( feedback && feedback->isCanceled() )
      break;

    const QByteArray rawData = loadFromNetwork( id, tileMatrix, source, mAuthCfg, mHeaders, feedback );
    if ( !rawData.isEmpty() )
    {
      rawTiles.append( QgsVectorTileRawData( id, rawData ) );
    }
  }
  return rawTiles;
}

QNetworkRequest QgsXyzVectorTileDataProvider::tileRequest( const QgsTileMatrix &tileMatrix, const QgsTileXYZ &id, Qgis::RendererUsage usage ) const
{
  QString urlTemplate = sourcePath();

  if ( urlTemplate.contains( QLatin1String( "{usage}" ) ) )
  {
    switch ( usage )
    {
      case Qgis::RendererUsage::View:
        urlTemplate.replace( QLatin1String( "{usage}" ), QLatin1String( "view" ) );
        break;
      case Qgis::RendererUsage::Export:
        urlTemplate.replace( QLatin1String( "{usage}" ), QLatin1String( "export" ) );
        break;
      case Qgis::RendererUsage::Unknown:
        urlTemplate.replace( QLatin1String( "{usage}" ), QString() );
        break;
    }
  }

  const QString url = QgsVectorTileUtils::formatXYZUrlTemplate( urlTemplate, id, tileMatrix );

  QNetworkRequest request( url );
  QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsXyzVectorTileDataProvider" ) );
  QgsSetRequestInitiatorId( request, id.toString() );

  request.setAttribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 1 ), id.column() );
  request.setAttribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 2 ), id.row() );
  request.setAttribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 3 ), id.zoomLevel() );

  request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
  request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

  mHeaders.updateNetworkRequest( request );

  if ( !mAuthCfg.isEmpty() &&  !QgsApplication::authManager()->updateNetworkRequest( request, mAuthCfg ) )
  {
    QgsMessageLog::logMessage( tr( "network request update failed for authentication config" ), tr( "Network" ) );
  }

  return request;
}

QByteArray QgsXyzVectorTileDataProvider::loadFromNetwork( const QgsTileXYZ &id, const QgsTileMatrix &tileMatrix, const QString &requestUrl, const QString &authid, const QgsHttpHeaders &headers, QgsFeedback *feedback )
{
  QString url = QgsVectorTileUtils::formatXYZUrlTemplate( requestUrl, id, tileMatrix );
  QNetworkRequest nr;
  nr.setUrl( QUrl( url ) );

  headers.updateNetworkRequest( nr );

  QgsBlockingNetworkRequest req;
  req.setAuthCfg( authid );
  QgsDebugMsgLevel( QStringLiteral( "Blocking request: " ) + url, 2 );
  QgsBlockingNetworkRequest::ErrorCode errCode = req.get( nr, false, feedback );
  if ( errCode != QgsBlockingNetworkRequest::NoError )
  {
    QgsDebugMsg( QStringLiteral( "Request failed: " ) + url );
    return QByteArray();
  }
  QgsNetworkReplyContent reply = req.reply();
  QgsDebugMsgLevel( QStringLiteral( "Request successful, content size %1" ).arg( reply.content().size() ), 2 );
  return reply.content();
}

///@endcond


