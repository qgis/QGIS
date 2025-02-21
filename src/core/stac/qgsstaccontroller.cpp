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
#include "qgsstaccollections.h"
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
  qDeleteAll( mFetchedStacObjects );
  qDeleteAll( mFetchedItemCollections );
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

  std::unique_ptr< QgsStacObject > object;
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
  mFetchedStacObjects.insert( requestId, object.release() );
  emit finishedStacObjectRequest( requestId, parser.error() );
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

  std::unique_ptr<QgsStacItemCollection> fc = parser.itemCollection();
  mFetchedItemCollections.insert( requestId, fc.release() );
  emit finishedItemCollectionRequest( requestId, parser.error() );
  reply->deleteLater();
  mReplies.removeOne( reply );
}

template<class T>
std::unique_ptr<T> QgsStacController::takeStacObject( int requestId )
{
  std::unique_ptr< QgsStacObject > obj( mFetchedStacObjects.take( requestId ) );

  if ( T *downCastObj = dynamic_cast< T * >( obj.get() ) )
  {
    ( void )obj.release();
    return std::unique_ptr< T >( downCastObj );
  }

  return nullptr;
}
template CORE_EXPORT std::unique_ptr< QgsStacItem > QgsStacController::takeStacObject<QgsStacItem>( int requestId );
template CORE_EXPORT std::unique_ptr< QgsStacCatalog > QgsStacController::takeStacObject<QgsStacCatalog>( int requestId );
template CORE_EXPORT std::unique_ptr< QgsStacObject > QgsStacController::takeStacObject<QgsStacObject>( int requestId );

std::unique_ptr< QgsStacItemCollection > QgsStacController::takeItemCollection( int requestId )
{
  std::unique_ptr< QgsStacItemCollection > col( mFetchedItemCollections.take( requestId ) );
  return col;
}

std::unique_ptr< QgsStacItemCollection > QgsStacController::fetchItemCollection( const QUrl &url, QString *error )
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
  std::unique_ptr< QgsStacItemCollection > ic( parser.itemCollection() );

  if ( error )
    *error = parser.error();

  return ic;
}

std::unique_ptr< QgsStacCollections > QgsStacController::fetchCollections( const QUrl &url, QString *error )
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
  std::unique_ptr< QgsStacCollections > col( parser.collections() );

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

std::unique_ptr<QgsStacCatalog> QgsStacController::openLocalCatalog( const QString &fileName ) const
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


std::unique_ptr<QgsStacCollection> QgsStacController::openLocalCollection( const QString &fileName ) const
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

std::unique_ptr<QgsStacItem> QgsStacController::openLocalItem( const QString &fileName ) const
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

template<class T>
std::unique_ptr<T> QgsStacController::fetchStacObject( const QUrl &url, QString *error )
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
  std::unique_ptr< QgsStacObject > object;
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
      break;
  }

  std::unique_ptr< T > res;
  if ( T *castObject = dynamic_cast< T * >( object.get() ) )
  {
    ( void )object.release();
    res.reset( castObject );
  }
  else
  {
    QgsDebugError( "Retrieved STAC object could not be cast to expected type" );
  }

  if ( error )
    *error = parser.error();

  return res;
}

template CORE_EXPORT std::unique_ptr< QgsStacItem > QgsStacController::fetchStacObject<QgsStacItem>( const QUrl &url, QString *error );
template CORE_EXPORT std::unique_ptr< QgsStacCollection > QgsStacController::fetchStacObject<QgsStacCollection>( const QUrl &url, QString *error );
template CORE_EXPORT std::unique_ptr< QgsStacCatalog > QgsStacController::fetchStacObject<QgsStacCatalog>( const QUrl &url, QString *error );
