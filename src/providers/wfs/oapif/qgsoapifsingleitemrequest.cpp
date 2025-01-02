/***************************************************************************
    qgsoapifsingleitemrequest.cpp
    -----------------------------
    begin                : May 2024
    copyright            : (C) 2024 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslogger.h"
#include "qgsoapifsingleitemrequest.h"
#include "moc_qgsoapifsingleitemrequest.cpp"
#include "qgsoapifutils.h"
#include "qgsproviderregistry.h"
#include "qgsvectordataprovider.h"

#include "cpl_vsi.h"

#include <QTextCodec>

QgsOapifSingleItemRequest::QgsOapifSingleItemRequest( const QgsDataSourceUri &baseUri, const QString &url )
  : QgsBaseNetworkRequest( QgsAuthorizationSettings( baseUri.username(), baseUri.password(), baseUri.authConfigId() ), tr( "OAPIF" ) ), mUrl( url )
{
  // Using Qt::DirectConnection since the download might be running on a different thread.
  // In this case, the request was sent from the main thread and is executed with the main
  // thread being blocked in future.waitForFinished() so we can run code on this object which
  // lives in the main thread without risking havoc.
  connect( this, &QgsBaseNetworkRequest::downloadFinished, this, &QgsOapifSingleItemRequest::processReply, Qt::DirectConnection );
}

bool QgsOapifSingleItemRequest::request( bool synchronous, bool forceRefresh )
{
  QgsDebugMsgLevel( QStringLiteral( " QgsOapifSingleItemRequest::request() start time: %1" ).arg( time( nullptr ) ), 5 );
  if ( !sendGET( QUrl::fromEncoded( mUrl.toLatin1() ), QString( "application/geo+json, application/json" ), synchronous, forceRefresh ) )
  {
    emit gotResponse();
    return false;
  }
  return true;
}

QString QgsOapifSingleItemRequest::errorMessageWithReason( const QString &reason )
{
  return tr( "Download of item failed: %1" ).arg( reason );
}

void QgsOapifSingleItemRequest::processReply()
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
    QgsDebugMsgLevel( QStringLiteral( "parsing item response: " ) + buffer, 4 );
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "parsing item response: " ) + buffer.left( 100 ) + QStringLiteral( "[... snip ...]" ) + buffer.right( 100 ), 4 );
  }

  const QString vsimemFilename = QStringLiteral( "/vsimem/oaipf_%1.json" ).arg( reinterpret_cast<quintptr>( &buffer ), QT_POINTER_SIZE * 2, 16, QLatin1Char( '0' ) );
  VSIFCloseL( VSIFileFromMemBuffer( vsimemFilename.toUtf8().constData(), const_cast<GByte *>( reinterpret_cast<const GByte *>( buffer.constData() ) ), buffer.size(), false ) );
  QgsProviderRegistry *pReg = QgsProviderRegistry::instance();
  const QgsDataProvider::ProviderOptions providerOptions;
  auto vectorProvider = std::unique_ptr<QgsVectorDataProvider>(
    qobject_cast<QgsVectorDataProvider *>( pReg->createProvider( "ogr", vsimemFilename, providerOptions ) )
  );
  if ( !vectorProvider || !vectorProvider->isValid() )
  {
    VSIUnlink( vsimemFilename.toUtf8().constData() );
    mErrorCode = QgsBaseNetworkRequest::ApplicationLevelError;
    mAppLevelError = ApplicationLevelError::JsonError;
    mErrorMessage = errorMessageWithReason( tr( "Loading of item failed" ) );
    emit gotResponse();
    return;
  }

  mFields = vectorProvider->fields();
  auto iter = vectorProvider->getFeatures();
  iter.nextFeature( mFeature );
  vectorProvider.reset();
  VSIUnlink( vsimemFilename.toUtf8().constData() );

  QgsDebugMsgLevel( QStringLiteral( "processReply end time: %1" ).arg( time( nullptr ) ), 5 );
  emit gotResponse();
}
