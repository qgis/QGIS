/***************************************************************************
    qgsoapiflandingpagerequest.cpp
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
#include "qgsoapiflandingpagerequest.h"
#include "qgsoapifutils.h"
#include "qgswfsconstants.h"

#include <QTextCodec>

QgsOapifLandingPageRequest::QgsOapifLandingPageRequest( const QgsDataSourceUri &uri ):
  QgsBaseNetworkRequest( QgsAuthorizationSettings( uri.username(), uri.password(), uri.authConfigId() ), "OAPIF" ),
  mUri( uri )
{
  // Using Qt::DirectConnection since the download might be running on a different thread.
  // In this case, the request was sent from the main thread and is executed with the main
  // thread being blocked in future.waitForFinished() so we can run code on this object which
  // lives in the main thread without risking havoc.
  connect( this, &QgsBaseNetworkRequest::downloadFinished, this, &QgsOapifLandingPageRequest::processReply, Qt::DirectConnection );
}

bool QgsOapifLandingPageRequest::request( bool synchronous, bool forceRefresh )
{
  if ( !sendGET( QUrl( mUri.param( QgsWFSConstants::URI_PARAM_URL ) ), QString( "application/json" ), synchronous, forceRefresh ) )
  {
    emit gotResponse();
    return false;
  }
  return true;
}

QString QgsOapifLandingPageRequest::errorMessageWithReason( const QString &reason )
{
  return tr( "Download of landing page failed: %1" ).arg( reason );
}

void QgsOapifLandingPageRequest::processReply()
{
  if ( mErrorCode != QgsBaseNetworkRequest::NoError )
  {
    emit gotResponse();
    return;
  }
  const QByteArray &buffer = mResponse;
  if ( buffer.isEmpty() )
  {
    mErrorMessage = tr( "empty response" );
    mErrorCode = QgsBaseNetworkRequest::ServerExceptionError;
    emit gotResponse();
    return;
  }

  QgsDebugMsgLevel( QStringLiteral( "parsing GetLandingPage response: " ) + buffer, 4 );

  QTextCodec::ConverterState state;
  QTextCodec *codec = QTextCodec::codecForName( "UTF-8" );
  Q_ASSERT( codec );

  const QString utf8Text = codec->toUnicode( buffer.constData(), buffer.size(), &state );
  if ( state.invalidChars != 0 )
  {
    mErrorCode = QgsBaseNetworkRequest::ApplicationLevelError;
    mAppLevelError = ApplicationLevelError::JsonError;
    mErrorMessage = errorMessageWithReason( tr( "Invalid UTF-8 content" ) );
    emit gotResponse();
    return;
  }

  try
  {
    const json j = json::parse( utf8Text.toStdString() );

    const auto links = QgsOAPIFJson::parseLinks( j );
    QStringList apiTypes;
    apiTypes <<  QStringLiteral( "application/vnd.oai.openapi+json;version=3.0" );
#ifndef REMOVE_SUPPORT_DRAFT_VERSIONS
    apiTypes <<  QStringLiteral( "application/openapi+json;version=3.0" );
#endif
    mApiUrl = QgsOAPIFJson::findLink( links,
                                      QStringLiteral( "service-desc" ),
                                      apiTypes );
#ifndef REMOVE_SUPPORT_DRAFT_VERSIONS
    if ( mApiUrl.isEmpty() )
    {
      mApiUrl = QgsOAPIFJson::findLink( links,
                                        QStringLiteral( "service" ),
                                        apiTypes );
    }
#endif
#ifndef REMOVE_SUPPORT_QGIS_SERVER_3_10_0_WRONG_SERVICE_DESC
    if ( mApiUrl.isEmpty() )
    {
      mApiUrl = QgsOAPIFJson::findLink( links,
                                        QStringLiteral( "service_desc" ),
                                        apiTypes );
    }
#endif

    QStringList collectionsTypes;
    collectionsTypes << QStringLiteral( "application/json" );
    mCollectionsUrl = QgsOAPIFJson::findLink( links,
                      QStringLiteral( "data" ),
                      collectionsTypes );
#ifndef REMOVE_SUPPORT_DRAFT_VERSIONS
    if ( mCollectionsUrl.isEmpty() )
    {
      mCollectionsUrl = QgsOAPIFJson::findLink( links,
                        QStringLiteral( "collections" ),
                        apiTypes );
    }
#endif
  }
  catch ( const json::parse_error &ex )
  {
    mErrorCode = QgsBaseNetworkRequest::ApplicationLevelError;
    mAppLevelError = ApplicationLevelError::JsonError;
    mErrorMessage = errorMessageWithReason( tr( "Cannot decode JSON document: %1" ).arg( QString::fromStdString( ex.what() ) ) );
    emit gotResponse();
    return;
  }

  // Strip off suffixes like /collections?f=json
  const auto posQuotationMark = mCollectionsUrl.indexOf( '?' );
  if ( posQuotationMark > 0 )
  {
    mCollectionsUrl = mCollectionsUrl.mid( 0, posQuotationMark );
  }

  if ( mApiUrl.isEmpty() || mCollectionsUrl.isEmpty() )
  {
    mErrorCode = QgsBaseNetworkRequest::ApplicationLevelError;
    mAppLevelError = ApplicationLevelError::IncompleteInformation;
    mErrorMessage = errorMessageWithReason( tr( "Missing information in response" ) );
    emit gotResponse();
    return;
  }

  emit gotResponse();
}
