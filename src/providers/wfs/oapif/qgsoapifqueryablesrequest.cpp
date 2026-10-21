/***************************************************************************
    qgsoapifqueryablesrequest.cpp
    -----------------------------
    begin                : April 2023
    copyright            : (C) 2023 by Even Rouault
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

#include <QString>

using namespace Qt::StringLiterals;

using namespace nlohmann;

#include "qgslogger.h"
#include "qgsoapifqueryablesrequest.h"
#include "moc_qgsoapifqueryablesrequest.cpp"
#include "qgsoapifutils.h"
#include "qgswfsconstants.h"

#include <QTextCodec>

QgsOapifQueryablesRequest::QgsOapifQueryablesRequest( const QgsDataSourceUri &uri )
  : QgsBaseNetworkRequest( QgsAuthorizationSettings( uri.username(), uri.password(), QgsHttpHeaders(), uri.authConfigId() ), "OAPIF" )
{
  // Using Qt::DirectConnection since the download might be running on a different thread.
  // In this case, the request was sent from the main thread and is executed with the main
  // thread being blocked in future.waitForFinished() so we can run code on this object which
  // lives in the main thread without risking havoc.
  connect( this, &QgsBaseNetworkRequest::downloadFinished, this, &QgsOapifQueryablesRequest::processReply, Qt::DirectConnection );
}

const QMap<QString, QgsOapifQueryablesRequest::Queryable> &QgsOapifQueryablesRequest::queryables( const QUrl &queryablesUrl )
{
  sendGET( queryablesUrl, QString( "application/schema+json" ), /*synchronous=*/true, /*forceRefresh=*/false );
  return mQueryables;
}

QString QgsOapifQueryablesRequest::errorMessageWithReason( const QString &reason )
{
  return tr( "Download of queryables failed: %1" ).arg( reason );
}

void QgsOapifQueryablesRequest::processReply()
{
  if ( mErrorCode != QgsBaseNetworkRequest::NoError )
  {
    return;
  }
  const QByteArray &buffer = mResponse;
  if ( buffer.isEmpty() )
  {
    mErrorMessage = tr( "empty response" );
    mErrorCode = QgsBaseNetworkRequest::ServerExceptionError;
    return;
  }

  QgsDebugMsgLevel( u"parsing Queryables response: "_s + buffer, 4 );

  QTextCodec::ConverterState state;
  QTextCodec *codec = QTextCodec::codecForName( "UTF-8" );
  Q_ASSERT( codec );

  const QString utf8Text = codec->toUnicode( buffer.constData(), buffer.size(), &state );
  if ( state.invalidChars != 0 )
  {
    mErrorCode = QgsBaseNetworkRequest::ApplicationLevelError;
    mErrorMessage = errorMessageWithReason( tr( "Invalid UTF-8 content" ) );
    return;
  }

  try
  {
    const json j = json::parse( utf8Text.toStdString() );

    if ( j.is_object() && j.contains( "properties" ) )
    {
      const json jProperties = j["properties"];
      if ( jProperties.is_object() )
      {
        for ( const auto &[key, val] : jProperties.items() )
        {
          if ( !val.is_object() )
            continue;

          Queryable queryable;
          if ( val.contains( "format" ) )
          {
            const json jFormat = val["format"];
            if ( jFormat.is_string() )
            {
              queryable.mFormat = QString::fromStdString( jFormat.get<std::string>() );
            }
          }

          if ( val.contains( "x-ogc-role" ) )
          {
            const json jOgcRole = val["x-ogc-role"];
            queryable.mIsPrimaryGeometry = jOgcRole.is_string() && jOgcRole.get<std::string>() == "primary-geometry";
          }

          bool hasGeoJsonRef = false;
          if ( val.contains( "$ref" ) )
          {
            const json jRef = val["$ref"];
            if ( jRef.is_string() )
            {
              const auto ref = jRef.get<std::string>();
              const char *prefix = "https://geojson.org/schema/";
              hasGeoJsonRef = ref.size() > strlen( prefix ) && ref.compare( 0, strlen( prefix ), prefix ) == 0;
            }
          }

          // Part 3 and Part 5 describe a geometry with a "geometry-*" format and/or a "primary-geometry" role,
          // older servers with a $ref to a GeoJSON schema. Any of these wins over a "type".
          queryable.mIsGeometry = hasGeoJsonRef || queryable.mFormat.startsWith( "geometry-"_L1 ) || queryable.mIsPrimaryGeometry;
          if ( !queryable.mIsGeometry )
          {
            if ( !val.contains( "type" ) )
              continue;
            const json jType = val["type"];
            if ( !jType.is_string() )
              continue;
            queryable.mType = QString::fromStdString( jType.get<std::string>() );
          }
          mQueryables[QString::fromStdString( key )] = queryable;
        }
      }
    }
  }
  catch ( const json::parse_error &ex )
  {
    mErrorCode = QgsBaseNetworkRequest::ApplicationLevelError;
    mErrorMessage = errorMessageWithReason( tr( "Cannot decode JSON document: %1" ).arg( QString::fromStdString( ex.what() ) ) );
    return;
  }
}
