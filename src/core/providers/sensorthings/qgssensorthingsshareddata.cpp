/***************************************************************************
      qgssensorthingsshareddata.h
      ----------------
    begin                : November 2023
    copyright            : (C) 2013 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssensorthingsshareddata.h"
#include "qgssensorthingsprovider.h"
#include "qgslogger.h"

#include <QCryptographicHash>
#include <QFile>

///@cond PRIVATE

QgsSensorThingsSharedData::QgsSensorThingsSharedData( const QString &uri )
{
  const QVariantMap uriParts = QgsSensorThingsProviderMetadata().decodeUri( uri );

  mEntityType = qgsEnumKeyToValue( uriParts.value( QStringLiteral( "entity" ) ).toString(), Qgis::SensorThingsEntity::Invalid );

  const QString geometryType = uriParts.value( QStringLiteral( "geometryType" ) ).toString();
  if ( geometryType.compare( QLatin1String( "point" ), Qt::CaseInsensitive ) == 0 )
  {
    mGeometryType = Qgis::WkbType::PointZ;
  }
  else if ( geometryType.compare( QLatin1String( "multipoint" ), Qt::CaseInsensitive ) == 0 )
  {
    mGeometryType = Qgis::WkbType::MultiPointZ;
  }
  else if ( geometryType.compare( QLatin1String( "line" ), Qt::CaseInsensitive ) == 0 )
  {
    mGeometryType = Qgis::WkbType::MultiLineStringZ;
  }
  else if ( geometryType.compare( QLatin1String( "polygon" ), Qt::CaseInsensitive ) == 0 )
  {
    mGeometryType = Qgis::WkbType::MultiPolygonZ;
  }

  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );
  mAuthCfg = dsUri.authConfigId();
  mHeaders = dsUri.httpHeaders();

  mRootUri = uriParts.value( QStringLiteral( "url" ) ).toString();
}

QUrl QgsSensorThingsSharedData::parseUrl( const QUrl &url, bool *isTestEndpoint )
{
  if ( isTestEndpoint )
    *isTestEndpoint = false;

  QUrl modifiedUrl( url );
  if ( modifiedUrl.toString().contains( QLatin1String( "fake_qgis_http_endpoint" ) ) )
  {
    if ( isTestEndpoint )
      *isTestEndpoint = true;

    // Just for testing with local files instead of http:// resources
    QString modifiedUrlString = modifiedUrl.toString();
    // Qt5 does URL encoding from some reason (of the FILTER parameter for example)
    modifiedUrlString = QUrl::fromPercentEncoding( modifiedUrlString.toUtf8() );
    modifiedUrlString.replace( QLatin1String( "fake_qgis_http_endpoint/" ), QLatin1String( "fake_qgis_http_endpoint_" ) );
    QgsDebugMsgLevel( QStringLiteral( "Get %1" ).arg( modifiedUrlString ), 2 );
    modifiedUrlString = modifiedUrlString.mid( QStringLiteral( "http://" ).size() );
    QString args = modifiedUrlString.indexOf( '?' ) >= 0 ? modifiedUrlString.mid( modifiedUrlString.indexOf( '?' ) ) : QString();
    if ( modifiedUrlString.size() > 150 )
    {
      args = QCryptographicHash::hash( args.toUtf8(), QCryptographicHash::Md5 ).toHex();
    }
    else
    {
      args.replace( QLatin1String( "?" ), QLatin1String( "_" ) );
      args.replace( QLatin1String( "&" ), QLatin1String( "_" ) );
      args.replace( QLatin1String( "<" ), QLatin1String( "_" ) );
      args.replace( QLatin1String( ">" ), QLatin1String( "_" ) );
      args.replace( QLatin1String( "'" ), QLatin1String( "_" ) );
      args.replace( QLatin1String( "\"" ), QLatin1String( "_" ) );
      args.replace( QLatin1String( " " ), QLatin1String( "_" ) );
      args.replace( QLatin1String( ":" ), QLatin1String( "_" ) );
      args.replace( QLatin1String( "/" ), QLatin1String( "_" ) );
      args.replace( QLatin1String( "\n" ), QLatin1String( "_" ) );
    }
#ifdef Q_OS_WIN
    // Passing "urls" like "http://c:/path" to QUrl 'eats' the : after c,
    // so we must restore it
    if ( modifiedUrlString[1] == '/' )
    {
      modifiedUrlString = modifiedUrlString[0] + ":/" + modifiedUrlString.mid( 2 );
    }
#endif
    modifiedUrlString = modifiedUrlString.mid( 0, modifiedUrlString.indexOf( '?' ) ) + args;
    QgsDebugMsgLevel( QStringLiteral( "Get %1 (after laundering)" ).arg( modifiedUrlString ), 2 );
    modifiedUrl = QUrl::fromLocalFile( modifiedUrlString );
    if ( !QFile::exists( modifiedUrlString ) )
    {
      QgsDebugError( QStringLiteral( "Local test file %1 for URL %2 does not exist!!!" ).arg( modifiedUrlString, url.toString() ) );
    }
  }

  return modifiedUrl;
}

///@endcond PRIVATE
