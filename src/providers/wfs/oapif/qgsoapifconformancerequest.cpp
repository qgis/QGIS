/***************************************************************************
    qgsoapifconformancerequest.cpp
    ------------------------------
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
#include "qgsoapifconformancerequest.h"
#include "moc_qgsoapifconformancerequest.cpp"
#include "qgsoapifutils.h"
#include "qgswfsconstants.h"

#include <QTextCodec>

QgsOapifConformanceRequest::QgsOapifConformanceRequest( const QgsDataSourceUri &uri )
  : QgsBaseNetworkRequest( QgsAuthorizationSettings( uri.username(), uri.password(), uri.authConfigId() ), "OAPIF" )
{
  // Using Qt::DirectConnection since the download might be running on a different thread.
  // In this case, the request was sent from the main thread and is executed with the main
  // thread being blocked in future.waitForFinished() so we can run code on this object which
  // lives in the main thread without risking havoc.
  connect( this, &QgsBaseNetworkRequest::downloadFinished, this, &QgsOapifConformanceRequest::processReply, Qt::DirectConnection );
}

QStringList QgsOapifConformanceRequest::conformanceClasses( const QUrl &conformanceUrl )
{
  sendGET( conformanceUrl, QString( "application/json" ), /*synchronous=*/true, /*forceRefresh=*/false );
  return mConformanceClasses;
}

QString QgsOapifConformanceRequest::errorMessageWithReason( const QString &reason )
{
  return tr( "Download of conformance classes failed: %1" ).arg( reason );
}

void QgsOapifConformanceRequest::processReply()
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

  QgsDebugMsgLevel( QStringLiteral( "parsing Conformance response: " ) + buffer, 4 );

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

    if ( j.is_object() && j.contains( "conformsTo" ) )
    {
      const json jConformsTo = j["conformsTo"];
      if ( jConformsTo.is_array() )
      {
        for ( const auto &subj : jConformsTo )
        {
          if ( subj.is_string() )
          {
            mConformanceClasses.append( QString::fromStdString( subj.get<std::string>() ) );
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
