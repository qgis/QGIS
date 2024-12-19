/***************************************************************************
    qgsstaccontroller.cpp
    ---------------------
    begin                : August 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstaccontroller.h"
#include "moc_qgsstaccontroller.cpp"
#include "qgsstaccatalog.h"
#include "qgsstaccollection.h"
#include "qgsstacitem.h"
#include "qgsstacitemcollection.h"
#include "qgsstacparser.h"
#include "qgslogger.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgssetrequestinitiator_p.h"

#include <QFile>


QgsStacController::~QgsStacController()
{
  qDeleteAll( mReplies );
}


int QgsStacController::fetchStacObjectAsync( const QUrl &url )
{
  QNetworkReply *reply = fetchAsync( url );
  connect( reply, &QNetworkReply::finished, this, &QgsStacController::handleStacObjectReply );

  return reply->property( "requestId" ).toInt();
}

int QgsStacController::fetchItemCollectionAsync( const QUrl &url )
{
  QNetworkReply *reply = fetchAsync( url );
  connect( reply, &QNetworkReply::finished, this, &QgsStacController::handleItemCollectionReply );

  return reply->property( "requestId" ).toInt();
}

int QgsStacController::fetchCollectionsAsync( const QUrl &url )
{
  QNetworkReply *reply = fetchAsync( url );
  connect( reply, &QNetworkReply::finished, this, &QgsStacController::handleCollectionsReply );

  return reply->property( "requestId" ).toInt();
}

void QgsStacController::cancelPendingAsyncRequests()
{
  for ( QNetworkReply *reply : std::as_const( mReplies ) )
  {
    reply->abort();
    reply->deleteLater();
  }
  mReplies.clear();
}

QNetworkReply *QgsStacController::fetchAsync( const QUrl &url )
{
  QNetworkRequest req( url );
  mHeaders.updateNetworkRequest( req );
  QgsSetRequestInitiatorClass( req, QStringLiteral( "QgsStacController" ) );
  req.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
  req.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

  if ( !mAuthCfg.isEmpty() )
  {
    QgsApplication::authManager()->updateNetworkRequest( req, mAuthCfg );
  }

  QgsNetworkAccessManager *nam = QgsNetworkAccessManager::instance();

  QNetworkReply *reply = nam->get( req );

  if ( !mAuthCfg.isEmpty() )
  {
    QgsApplication::authManager()->updateNetworkReply( reply, mAuthCfg );
  }

  mReplies.append( reply );

  QgsDebugMsgLevel( QStringLiteral( "Fired STAC request with id %1" ).arg( reply->property( "requestId" ).toInt() ), 2 );

  return reply;
}

void QgsStacController::handleStacObjectReply()
{
  QNetworkReply *reply = qobject_cast<QNetworkReply *>( QObject::sender() );
  if ( !reply )
    return;

  const int requestId = reply->property( "requestId" ).toInt();
  QgsDebugMsgLevel( QStringLiteral( "Finished STAC request with id %1" ).arg( requestId ), 2 );

  if ( reply->error() != QNetworkReply::NoError )
  {
    emit finishedStacObjectRequest( requestId, reply->errorString() );
    reply->deleteLater();
    mReplies.removeOne( reply );
    return;
  }

  const QByteArray data = reply->readAll();
  QgsStacParser parser;
  parser.setData( data );
  parser.setBaseUrl( reply->url() );

  QString error;
  QgsStacObject *object = nullptr;
  switch ( parser.type() )
  {
    case QgsStacObject::Type::Catalog:
      object = parser.catalog();
      break;
    case QgsStacObject::Type::Collection:
      object = parser.collection();
      break;
    case QgsStacObject::Type::Item:
      object = parser.item();
      break;
    case QgsStacObject::Type::Unknown:
      object = nullptr;
      error = parser.error().isEmpty() ? QStringLiteral( "Parsed STAC data is not a Catalog, Collection or Item" ) : parser.error();
      break;
  }
  mFetchedStacObjects.insert( requestId, object );
  emit finishedStacObjectRequest( requestId, error.isEmpty() ? parser.error() : error );
  reply->deleteLater();
  mReplies.removeOne( reply );
}

void QgsStacController::handleItemCollectionReply()
{
  QNetworkReply *reply = qobject_cast<QNetworkReply *>( QObject::sender() );
  if ( !reply )
    return;

  const int requestId = reply->property( "requestId" ).toInt();
  QgsDebugMsgLevel( QStringLiteral( "Finished STAC request with id %1" ).arg( requestId ), 2 );

  if ( reply->error() != QNetworkReply::NoError )
  {
    emit finishedItemCollectionRequest( requestId, reply->errorString() );
    reply->deleteLater();
    mReplies.removeOne( reply );
    return;
  }

  const QByteArray data = reply->readAll();
  QgsStacParser parser;
  parser.setData( data );
  parser.setBaseUrl( reply->url() );

  QgsStacItemCollection *fc = parser.itemCollection();
  mFetchedItemCollections.insert( requestId, fc );
  emit finishedItemCollectionRequest( requestId, parser.error() );
  reply->deleteLater();
  mReplies.removeOne( reply );
}

void QgsStacController::handleCollectionsReply()
{
  QNetworkReply *reply = qobject_cast<QNetworkReply *>( QObject::sender() );
  if ( !reply )
    return;

  const int requestId = reply->property( "requestId" ).toInt();
  QgsDebugMsgLevel( QStringLiteral( "Finished STAC request with id %1" ).arg( requestId ), 2 );

  if ( reply->error() != QNetworkReply::NoError )
  {
    emit finishedCollectionsRequest( requestId, reply->errorString() );
    reply->deleteLater();
    mReplies.removeOne( reply );
    return;
  }

  const QByteArray data = reply->readAll();
  QgsStacParser parser;
  parser.setData( data );
  parser.setBaseUrl( reply->url() );

  QgsStacCollections *cols = parser.collections();
  mFetchedCollections.insert( requestId, cols );
  emit finishedCollectionsRequest( requestId, parser.error() );
  reply->deleteLater();
  mReplies.removeOne( reply );
}

QgsStacObject *QgsStacController::takeStacObject( int requestId )
{
  return mFetchedStacObjects.take( requestId );
}

QgsStacItemCollection *QgsStacController::takeItemCollection( int requestId )
{
  return mFetchedItemCollections.take( requestId );
}

QgsStacCollections *QgsStacController::takeCollections( int requestId )
{
  return mFetchedCollections.take( requestId );
}

QgsStacObject *QgsStacController::fetchStacObject( const QUrl &url, QString *error )
{
  QgsNetworkReplyContent content = fetchBlocking( url );

  if ( content.error() != QNetworkReply::NoError )
  {
    if ( error )
      *error = content.errorString();

    return nullptr;
  }

  const QByteArray data = content.content();

  QgsStacParser parser;
  parser.setData( data );
  parser.setBaseUrl( url );
  QgsStacObject *object = nullptr;
  switch ( parser.type() )
  {
    case QgsStacObject::Type::Catalog:
      object = parser.catalog();
      break;
    case QgsStacObject::Type::Collection:
      object = parser.collection();
      break;
    case QgsStacObject::Type::Item:
      object = parser.item();
      break;
    case QgsStacObject::Type::Unknown:
      object = nullptr;
      break;
  }

  if ( error )
    *error = parser.error();

  return object;
}

QgsStacItemCollection *QgsStacController::fetchItemCollection( const QUrl &url, QString *error )
{
  QgsNetworkReplyContent content = fetchBlocking( url );

  if ( content.error() != QNetworkReply::NoError )
  {
    if ( error )
      *error = content.errorString();

    return nullptr;
  }

  const QByteArray data = content.content();

  QgsStacParser parser;
  parser.setData( data );
  parser.setBaseUrl( url );
  QgsStacItemCollection *ic = parser.itemCollection();

  if ( error )
    *error = parser.error();

  return ic;
}

QgsStacCollections *QgsStacController::fetchCollections( const QUrl &url, QString *error )
{
  QgsNetworkReplyContent content = fetchBlocking( url );

  if ( content.error() != QNetworkReply::NoError )
  {
    if ( error )
      *error = content.errorString();

    return nullptr;
  }

  const QByteArray data = content.content();

  QgsStacParser parser;
  parser.setData( data );
  QgsStacCollections *col = parser.collections();

  if ( error )
    *error = parser.error();

  return col;
}

QgsNetworkReplyContent QgsStacController::fetchBlocking( const QUrl &url )
{
  QNetworkRequest req( url );
  mHeaders.updateNetworkRequest( req );
  QgsSetRequestInitiatorClass( req, QStringLiteral( "QgsStacController" ) );
  req.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
  req.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

  if ( !mAuthCfg.isEmpty() )
  {
    QgsApplication::authManager()->updateNetworkRequest( req, mAuthCfg );
  }

  QgsNetworkAccessManager *nam = QgsNetworkAccessManager::instance();

  return nam->blockingGet( req );
}

QString QgsStacController::authCfg() const
{
  return mAuthCfg;
}

void QgsStacController::setAuthCfg( const QString &authCfg )
{
  mAuthCfg = authCfg;
}

QgsStacCatalog *QgsStacController::openLocalCatalog( const QString &fileName ) const
{
  QFile file( fileName );
  const bool ok = file.open( QIODevice::ReadOnly );
  if ( !ok )
  {
    QgsDebugError( QStringLiteral( "Could not open file: " ).arg( fileName ) );
    return nullptr;
  }

  QgsStacParser parser;
  parser.setData( file.readAll() );
  parser.setBaseUrl( fileName );
  return parser.catalog();
}


QgsStacCollection *QgsStacController::openLocalCollection( const QString &fileName ) const
{
  QFile file( fileName );
  const bool ok = file.open( QIODevice::ReadOnly );
  if ( !ok )
  {
    QgsDebugError( QStringLiteral( "Could not open file: " ).arg( fileName ) );
    return nullptr;
  }

  QgsStacParser parser;
  parser.setData( file.readAll() );
  parser.setBaseUrl( fileName );
  return parser.collection();
}

QgsStacItem *QgsStacController::openLocalItem( const QString &fileName ) const
{
  QFile file( fileName );
  const bool ok = file.open( QIODevice::ReadOnly );
  if ( !ok )
  {
    QgsDebugError( QStringLiteral( "Could not open file: " ).arg( fileName ) );
    return nullptr;
  }

  QgsStacParser parser;
  parser.setData( file.readAll() );
  parser.setBaseUrl( fileName );
  return parser.item();
}





