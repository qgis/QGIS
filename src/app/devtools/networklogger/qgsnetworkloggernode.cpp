/***************************************************************************
    qgsnetworkloggernode.cpp
    -------------------------
    begin                : March 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsnetworkloggernode.h"
#include "qgis.h"
#include "qgsjsonutils.h"
#include <QUrlQuery>
#include <QColor>
#include <QBrush>
#include <QFont>
#include <QAction>
#include <QDesktopServices>
#include <QApplication>
#include <QClipboard>
#include <nlohmann/json.hpp>

//
// QgsNetworkLoggerNode
//

QgsNetworkLoggerNode::QgsNetworkLoggerNode() = default;
QgsNetworkLoggerNode::~QgsNetworkLoggerNode() = default;


//
// QgsNetworkLoggerGroup
//

QgsNetworkLoggerGroup::QgsNetworkLoggerGroup( const QString &title )
  : mGroupTitle( title )
{
}

void QgsNetworkLoggerGroup::addChild( std::unique_ptr<QgsNetworkLoggerNode> child )
{
  if ( !child )
    return;

  Q_ASSERT( !child->mParent );
  child->mParent = this;

  mChildren.emplace_back( std::move( child ) );
}

int QgsNetworkLoggerGroup::indexOf( QgsNetworkLoggerNode *child ) const
{
  Q_ASSERT( child->mParent == this );
  auto it = std::find_if( mChildren.begin(), mChildren.end(), [&]( const std::unique_ptr<QgsNetworkLoggerNode> &p )
  {
    return p.get() == child;
  } );
  if ( it != mChildren.end() )
    return std::distance( mChildren.begin(), it );
  return -1;
}

QgsNetworkLoggerNode *QgsNetworkLoggerGroup::childAt( int index )
{
  Q_ASSERT( static_cast< std::size_t >( index ) < mChildren.size() );
  return mChildren[ index ].get();
}

void QgsNetworkLoggerGroup::clear()
{
  mChildren.clear();
}

QVariant QgsNetworkLoggerGroup::data( int role ) const
{
  switch ( role )
  {
    case Qt::DisplayRole:
      return mGroupTitle;

    default:
      break;
  }
  return QVariant();
}

QVariant QgsNetworkLoggerGroup::toVariant() const
{
  QVariantMap res;
  for ( const std::unique_ptr< QgsNetworkLoggerNode > &child : mChildren )
  {
    if ( const QgsNetworkLoggerValueNode *valueNode = dynamic_cast< const QgsNetworkLoggerValueNode *>( child.get() ) )
    {
      res.insert( valueNode->key(), valueNode->value() );
    }
  }
  return res;
}

//
// QgsNetworkLoggerRootNode
//

QgsNetworkLoggerRootNode::QgsNetworkLoggerRootNode()
  : QgsNetworkLoggerGroup( QString() )
{

}

QVariant QgsNetworkLoggerRootNode::data( int ) const
{
  return QVariant();
}

void QgsNetworkLoggerRootNode::removeRow( int row )
{
  mChildren.erase( mChildren.begin() + row );
}

QVariant QgsNetworkLoggerRootNode::toVariant() const
{
  QVariantList res;
  for ( const std::unique_ptr< QgsNetworkLoggerNode > &child : mChildren )
    res << child->toVariant();
  return res;
}


//
// QgsNetworkLoggerValueNode
//
QgsNetworkLoggerValueNode::QgsNetworkLoggerValueNode( const QString &key, const QString &value, const QColor &color )
  : mKey( key )
  , mValue( value )
  , mColor( color )
{

}

QVariant QgsNetworkLoggerValueNode::data( int role ) const
{
  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
    {
      return QStringLiteral( "%1: %2" ).arg( mKey.leftJustified( 30, ' ' ), mValue );
    }

    case Qt::ForegroundRole:
    {
      if ( mColor.isValid() )
        return QBrush( mColor );
      break;
    }
    default:
      break;
  }
  return QVariant();
}

QList<QAction *> QgsNetworkLoggerValueNode::actions( QObject *parent )
{
  QList< QAction * > res;

  QAction *copyAction = new QAction( QObject::tr( "Copy" ), parent );
  QObject::connect( copyAction, &QAction::triggered, copyAction, [ = ]
  {
    QApplication::clipboard()->setText( QStringLiteral( "%1: %2" ).arg( mKey, mValue ) );
  } );

  res << copyAction;

  return res;
}

//
// QgsNetworkLoggerGroup
//

void QgsNetworkLoggerGroup::addKeyValueNode( const QString &key, const QString &value, const QColor &color )
{
  addChild( std::make_unique< QgsNetworkLoggerValueNode >( key, value, color ) );
}


//
// QgsNetworkLoggerRequestGroup
//

QgsNetworkLoggerRequestGroup::QgsNetworkLoggerRequestGroup( const QgsNetworkRequestParameters &request )
  : QgsNetworkLoggerGroup( QString() )
  , mUrl( request.request().url() )
  , mRequestId( request.requestId() )
  , mOperation( request.operation() )
  , mData( request.content() )
{
  const QList<QByteArray> headers = request.request().rawHeaderList();
  for ( const QByteArray &header : headers )
  {
    mHeaders.append( qMakePair( QString( header ), QString( request.request().rawHeader( header ) ) ) );
  }

  std::unique_ptr< QgsNetworkLoggerRequestDetailsGroup > detailsGroup = std::make_unique< QgsNetworkLoggerRequestDetailsGroup >( request );
  mDetailsGroup = detailsGroup.get();
  addChild( std::move( detailsGroup ) );

  mTimer.start();
}

QVariant QgsNetworkLoggerRequestGroup::data( int role ) const
{
  switch ( role )
  {
    case Qt::DisplayRole:
      return QStringLiteral( "%1 %2 %3" ).arg( QString::number( mRequestId ),
             operationToString( mOperation ),
             mUrl.url() );

    case Qt::ToolTipRole:
    {
      QString bytes = QObject::tr( "unknown" );
      if ( mBytesTotal != 0 )
      {
        if ( mBytesReceived > 0 && mBytesReceived < mBytesTotal )
          bytes = QStringLiteral( "%1/%2" ).arg( QString::number( mBytesReceived ), QString::number( mBytesTotal ) );
        else if ( mBytesReceived > 0 && mBytesReceived == mBytesTotal )
          bytes = QString::number( mBytesTotal );
      }
      // ?? adding <br/> instead of \n after (very long) url seems to break url up
      // COMPLETE, Status: 200 - text/xml; charset=utf-8 - 2334 bytes - 657 milliseconds
      return QStringLiteral( "%1<br/>%2 - Status: %3 - %4 - %5 bytes - %6 msec - %7 replies" )
             .arg( mUrl.url(),
                   statusToString( mStatus ),
                   QString::number( mHttpStatus ),
                   mContentType,
                   bytes,
                   mStatus == Status::Pending ? QString::number( mTimer.elapsed() / 1000 ) : QString::number( mTotalTime ),
                   QString::number( mReplies ) );
    }

    case RoleStatus:
      return static_cast< int >( mStatus );

    case RoleId:
      return mRequestId;

    case Qt::ForegroundRole:
    {
      if ( mHasSslErrors )
        return QBrush( QColor( 180, 65, 210 ) );
      switch ( mStatus )
      {
        case QgsNetworkLoggerRequestGroup::Status::Pending:
        case QgsNetworkLoggerRequestGroup::Status::Canceled:
          return QBrush( QColor( 0, 0, 0, 100 ) );
        case QgsNetworkLoggerRequestGroup::Status::Error:
          return QBrush( QColor( 235, 10, 10 ) );
        case QgsNetworkLoggerRequestGroup::Status::TimeOut:
          return QBrush( QColor( 235, 10, 10 ) );
        case QgsNetworkLoggerRequestGroup::Status::Complete:
        {
          if ( mReplyFromCache )
            return QBrush( QColor( 10, 40, 85, 150 ) );

          break;
        }
      }
      break;
    }

    case Qt::FontRole:
    {
      if ( mStatus == Status::Canceled )
      {
        QFont f;
        f.setStrikeOut( true );
        return f;
      }
      break;
    }

    default:
      break;
  }
  return QVariant();
}

QList<QAction *> QgsNetworkLoggerRequestGroup::actions( QObject *parent )
{
  QList< QAction * > res;
  QAction *openUrlAction = new QAction( QObject::tr( "Open URL" ), parent );
  QObject::connect( openUrlAction, &QAction::triggered, openUrlAction, [ = ]
  {
    QDesktopServices::openUrl( mUrl );
  } );
  res << openUrlAction;

  QAction *copyUrlAction = new QAction( QObject::tr( "Copy URL" ), parent );
  QObject::connect( copyUrlAction, &QAction::triggered, openUrlAction, [ = ]
  {
    QApplication::clipboard()->setText( mUrl.url() );
  } );
  res << copyUrlAction;

  QAction *copyAsCurlAction = new QAction( QObject::tr( "Copy As cURL" ), parent );
  QObject::connect( copyAsCurlAction, &QAction::triggered, copyAsCurlAction, [ = ]
  {
    QString curlHeaders;
    for ( const QPair< QString, QString > &header : std::as_const( mHeaders ) )
      curlHeaders += QStringLiteral( "-H '%1: %2' " ).arg( header.first, header.second );

    QString curlData;
    if ( mOperation == QNetworkAccessManager::PostOperation || mOperation == QNetworkAccessManager::PutOperation )
      curlData = QStringLiteral( "--data '%1' " ).arg( QString( mData ) );

    QString curlCmd = QStringLiteral( "curl '%1' %2 %3--compressed" ).arg(
      mUrl.url(),
      curlHeaders,
      curlData );
    QApplication::clipboard()->setText( curlCmd );
  } );
  res << copyAsCurlAction;

  QAction *copyJsonAction = new QAction( QObject::tr( "Copy as JSON" ), parent );
  QObject::connect( copyJsonAction, &QAction::triggered, openUrlAction, [ = ]
  {
    const QVariant value = toVariant();
    const QString json = QString::fromStdString( QgsJsonUtils::jsonFromVariant( value ).dump( 2 ) );
    QApplication::clipboard()->setText( json );

  } );
  res << copyJsonAction;

  return res;
}

QVariant QgsNetworkLoggerRequestGroup::toVariant() const
{
  QVariantMap res;
  res.insert( QStringLiteral( "URL" ), mUrl.url() );
  res.insert( QStringLiteral( "Total time (ms)" ), mTotalTime );
  res.insert( QStringLiteral( "Bytes Received" ), mBytesReceived );
  res.insert( QStringLiteral( "Bytes Total" ), mBytesTotal );
  res.insert( QStringLiteral( "Replies" ), mReplies );
  if ( mDetailsGroup )
  {
    const QVariantMap detailsMap = mDetailsGroup->toVariant().toMap();
    for ( auto it = detailsMap.constBegin(); it != detailsMap.constEnd(); ++it )
      res.insert( it.key(), it.value() );
  }
  if ( mReplyGroup )
  {
    res.insert( QObject::tr( "Reply" ), mReplyGroup->toVariant() );
  }
  if ( mSslErrorsGroup )
  {
    res.insert( QObject::tr( "SSL Errors" ), mSslErrorsGroup->toVariant() );
  }
  return res;
}

void QgsNetworkLoggerRequestGroup::setReply( const QgsNetworkReplyContent &reply )
{
  switch ( reply.error() )
  {
    case QNetworkReply::OperationCanceledError:
      mStatus = Status::Canceled;
      break;

    case QNetworkReply::NoError:
      mStatus = Status::Complete;
      break;

    default:
      mStatus = Status::Error;
      break;
  }

  mTotalTime = mTimer.elapsed();
  mHttpStatus = reply.attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  mContentType = reply.rawHeader( "Content - Type" );
  mReplyFromCache = reply.attribute( QNetworkRequest::SourceIsFromCacheAttribute ).toBool();

  std::unique_ptr< QgsNetworkLoggerReplyGroup > replyGroup = std::make_unique< QgsNetworkLoggerReplyGroup >( reply ) ;
  mReplyGroup = replyGroup.get();
  addChild( std::move( replyGroup ) );
}

void QgsNetworkLoggerRequestGroup::setTimedOut()
{
  mStatus = Status::TimeOut;
}

void QgsNetworkLoggerRequestGroup::setProgress( qint64 bytesReceived, qint64 bytesTotal )
{
  mReplies++;
  mBytesReceived = bytesReceived;
  mBytesTotal = bytesTotal;
}

void QgsNetworkLoggerRequestGroup::setSslErrors( const QList<QSslError> &errors )
{
  mHasSslErrors = !errors.empty();
  if ( mHasSslErrors )
  {
    std::unique_ptr< QgsNetworkLoggerSslErrorGroup > errorGroup =  std::make_unique< QgsNetworkLoggerSslErrorGroup >( errors );
    mSslErrorsGroup = errorGroup.get();
    addChild( std::move( errorGroup ) );
  }
}

QString QgsNetworkLoggerRequestGroup::operationToString( QNetworkAccessManager::Operation operation )
{
  switch ( operation )
  {
    case QNetworkAccessManager::HeadOperation:
      return QStringLiteral( "HEAD" );
    case QNetworkAccessManager::GetOperation:
      return QStringLiteral( "GET" );
    case QNetworkAccessManager::PutOperation:
      return QStringLiteral( "PUT" );
    case QNetworkAccessManager::PostOperation:
      return QStringLiteral( "POST" );
    case QNetworkAccessManager::DeleteOperation:
      return QStringLiteral( "DELETE" );
    case QNetworkAccessManager::UnknownOperation:
      return QStringLiteral( "UNKNOWN" );
    case QNetworkAccessManager::CustomOperation:
      return QStringLiteral( "CUSTOM" );
  }
  return QString();
}

QString QgsNetworkLoggerRequestGroup::statusToString( QgsNetworkLoggerRequestGroup::Status status )
{
  switch ( status )
  {
    case QgsNetworkLoggerRequestGroup::Status::Pending:
      return QObject::tr( "Pending" );
    case QgsNetworkLoggerRequestGroup::Status::Complete:
      return QObject::tr( "Complete" );
    case QgsNetworkLoggerRequestGroup::Status::Error:
      return QObject::tr( "Error" );
    case QgsNetworkLoggerRequestGroup::Status::TimeOut:
      return QObject::tr( "Timeout" );
    case QgsNetworkLoggerRequestGroup::Status::Canceled:
      return QObject::tr( "Canceled" );
  }
  return QString();
}

QString QgsNetworkLoggerRequestGroup::cacheControlToString( QNetworkRequest::CacheLoadControl control )
{
  switch ( control )
  {
    case QNetworkRequest::AlwaysNetwork:
      return QObject::tr( "Always load from network, do not check cache" );
    case QNetworkRequest::PreferNetwork:
      return QObject::tr( "Load from the network if the cached entry is older than the network entry" );
    case QNetworkRequest::PreferCache:
      return QObject::tr( "Load from cache if available, otherwise load from network" );
    case QNetworkRequest::AlwaysCache:
      return QObject::tr( "Only load from cache, error if no cached entry available" );
  }
  return QString();
}


//
// QgsNetworkLoggerRequestDetailsGroup
//

QgsNetworkLoggerRequestDetailsGroup::QgsNetworkLoggerRequestDetailsGroup( const QgsNetworkRequestParameters &request )
  : QgsNetworkLoggerGroup( QObject::tr( "Request" ) )
{
  addKeyValueNode( QObject::tr( "Operation" ), QgsNetworkLoggerRequestGroup::operationToString( request.operation() ) );
  addKeyValueNode( QObject::tr( "Thread" ), request.originatingThreadId() );
  addKeyValueNode( QObject::tr( "Initiator" ), request.initiatorClassName().isEmpty() ? QObject::tr( "unknown" ) : request.initiatorClassName() );
  if ( request.initiatorRequestId().isValid() )
    addKeyValueNode( QObject::tr( "ID" ), request.initiatorRequestId().toString() );
  addKeyValueNode( QObject::tr( "Cache (control)" ), QgsNetworkLoggerRequestGroup::cacheControlToString( static_cast< QNetworkRequest::CacheLoadControl >( request.request().attribute( QNetworkRequest::CacheLoadControlAttribute ).toInt() ) ) );
  addKeyValueNode( QObject::tr( "Cache (save)" ), request.request().attribute( QNetworkRequest::CacheSaveControlAttribute ).toBool() ? QObject::tr( "Can store result in cache" ) : QObject::tr( "Result cannot be stored in cache" ) );

  if ( !QUrlQuery( request.request().url() ).queryItems().isEmpty() )
  {
    std::unique_ptr< QgsNetworkLoggerRequestQueryGroup > queryGroup = std::make_unique< QgsNetworkLoggerRequestQueryGroup >( request.request().url() );
    mQueryGroup = queryGroup.get();
    addChild( std::move( queryGroup ) );
  }

  std::unique_ptr< QgsNetworkLoggerRequestHeadersGroup > requestHeadersGroup = std::make_unique< QgsNetworkLoggerRequestHeadersGroup >( request );
  mRequestHeaders = requestHeadersGroup.get();
  addChild( std::move( requestHeadersGroup ) );

  switch ( request.operation() )
  {
    case QNetworkAccessManager::PostOperation:
    case QNetworkAccessManager::PutOperation:
    {
      std::unique_ptr< QgsNetworkLoggerPostContentGroup > postContentGroup = std::make_unique< QgsNetworkLoggerPostContentGroup >( request );
      mPostContent = postContentGroup.get();
      addChild( std::move( postContentGroup ) );
      break;
    }

    case QNetworkAccessManager::GetOperation:
    case QNetworkAccessManager::HeadOperation:
    case QNetworkAccessManager::DeleteOperation:
    case QNetworkAccessManager::UnknownOperation:
    case QNetworkAccessManager::CustomOperation:
      break;
  }
}

QVariant QgsNetworkLoggerRequestDetailsGroup::toVariant() const
{
  QVariantMap res = QgsNetworkLoggerGroup::toVariant().toMap();
  if ( mQueryGroup )
    res.insert( QObject::tr( "Query" ), mQueryGroup->toVariant() );
  if ( mRequestHeaders )
    res.insert( QObject::tr( "Headers" ), mRequestHeaders->toVariant() );
  if ( mPostContent )
    res.insert( QObject::tr( "Content" ), mPostContent->toVariant() );
  return res;
}


//
// QgsNetworkLoggerRequestQueryGroup
//

QgsNetworkLoggerRequestQueryGroup::QgsNetworkLoggerRequestQueryGroup( const QUrl &url )
  : QgsNetworkLoggerGroup( QObject::tr( "Query" ) )
{
  QUrlQuery query( url );
  const QList<QPair<QString, QString> > queryItems = query.queryItems();

  for ( const QPair< QString, QString > &query : queryItems )
  {
    addKeyValueNode( query.first, query.second );
  }
}


//
// QgsNetworkLoggerRequestHeadersGroup
//
QgsNetworkLoggerRequestHeadersGroup::QgsNetworkLoggerRequestHeadersGroup( const QgsNetworkRequestParameters &request )
  : QgsNetworkLoggerGroup( QObject::tr( "Headers" ) )
{
  const QList<QByteArray> headers = request.request().rawHeaderList();
  for ( const QByteArray &header : headers )
  {
    addKeyValueNode( header, request.request().rawHeader( header ) );
  }
}

//
// QgsNetworkLoggerPostContentGroup
//

QgsNetworkLoggerPostContentGroup::QgsNetworkLoggerPostContentGroup( const QgsNetworkRequestParameters &parameters )
  : QgsNetworkLoggerGroup( QObject::tr( "Content" ) )
{
  addKeyValueNode( QObject::tr( "Data" ), parameters.content() );
}


//
// QgsNetworkLoggerReplyGroup
//

QgsNetworkLoggerReplyGroup::QgsNetworkLoggerReplyGroup( const QgsNetworkReplyContent &reply )
  : QgsNetworkLoggerGroup( QObject::tr( "Reply" ) )
{
  addKeyValueNode( QObject::tr( "Status" ), reply.attribute( QNetworkRequest::HttpStatusCodeAttribute ).toString() );
  if ( reply.error() != QNetworkReply::NoError )
  {
    addKeyValueNode( QObject::tr( "Error Code" ), QString::number( static_cast< int >( reply.error() ) ) );
    addKeyValueNode( QObject::tr( "Error" ), reply.errorString() );
  }
  addKeyValueNode( QObject::tr( "Cache (result)" ), reply.attribute( QNetworkRequest::SourceIsFromCacheAttribute ).toBool() ? QObject::tr( "Used entry from cache" ) : QObject::tr( "Read from network" ) );

  std::unique_ptr< QgsNetworkLoggerReplyHeadersGroup > headersGroup = std::make_unique< QgsNetworkLoggerReplyHeadersGroup >( reply );
  mReplyHeaders = headersGroup.get();
  addChild( std::move( headersGroup ) );
}

QVariant QgsNetworkLoggerReplyGroup::toVariant() const
{
  QVariantMap res = QgsNetworkLoggerGroup::toVariant().toMap();
  if ( mReplyHeaders )
  {
    res.insert( QObject::tr( "Headers" ), mReplyHeaders->toVariant() );
  }
  return res;
}


//
// QgsNetworkLoggerReplyHeadersGroup
//
QgsNetworkLoggerReplyHeadersGroup::QgsNetworkLoggerReplyHeadersGroup( const QgsNetworkReplyContent &reply )
  : QgsNetworkLoggerGroup( QObject::tr( "Headers" ) )
{
  const QList<QByteArray> headers = reply.rawHeaderList();
  for ( const QByteArray &header : headers )
  {
    addKeyValueNode( header, reply.rawHeader( header ) );
  }
}

//
// QgsNetworkLoggerSslErrorGroup
//
QgsNetworkLoggerSslErrorGroup::QgsNetworkLoggerSslErrorGroup( const QList<QSslError> &errors )
  : QgsNetworkLoggerGroup( QObject::tr( "SSL errors" ) )
{
  for ( const QSslError &error : errors )
  {
    addKeyValueNode( QObject::tr( "Error" ), error.errorString(), QColor( 180, 65, 210 ) );
  }
}

QVariant QgsNetworkLoggerSslErrorGroup::data( int role ) const
{
  if ( role == Qt::ForegroundRole )
    return QBrush( QColor( 180, 65, 210 ) );

  return QgsNetworkLoggerGroup::data( role );
}

QList<QAction *> QgsNetworkLoggerNode::actions( QObject * )
{
  return QList< QAction * >();
}

QVariant QgsNetworkLoggerNode::toVariant() const
{
  return QVariant();
}
