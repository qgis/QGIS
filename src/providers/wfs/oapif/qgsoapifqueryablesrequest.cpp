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
using namespace nlohmann;

#include "qgslogger.h"
#include "qgsoapifqueryablesrequest.h"
#include "moc_qgsoapifqueryablesrequest.cpp"
#include "qgsoapifutils.h"
#include "qgswfsconstants.h"

#include <QTextCodec>

QgsOapifQueryablesRequest::QgsOapifQueryablesRequest( const QgsDataSourceUri &uri )
  : QgsBaseNetworkRequest( QgsAuthorizationSettings( uri.username(), uri.password(), uri.authConfigId() ), "OAPIF" )
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

  QgsDebugMsgLevel( QStringLiteral( "parsing Queryables response: " ) + buffer, 4 );

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
          if ( val.is_object() && val.contains( "type" ) )
          {
            const json jType = val["type"];
            if ( jType.is_string() )
            {
              Queryable queryable;
              queryable.mType = QString::fromStdString( jType.get<std::string>() );
              if ( val.contains( "format" ) )
              {
                const json jFormat = val["format"];
                if ( jFormat.is_string() )
                {
                  queryable.mFormat = QString::fromStdString( jFormat.get<std::string>() );
                }
              }
              mQueryables[QString::fromStdString( key )] = queryable;
            }
          }
          else if ( val.is_object() && val.contains( "$ref" ) )
          {
            const json jRef = val["$ref"];
            if ( jRef.is_string() )
            {
              const auto ref = jRef.get<std::string>();
              const char *prefix = "https://geojson.org/schema/";
              if ( ref.size() > strlen( prefix ) && ref.compare( 0, strlen( prefix ), prefix ) == 0 )
              {
                Queryable queryable;
                queryable.mIsGeometry = true;
                mQueryables[QString::fromStdString( key )] = queryable;
              }
            }
          }
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
