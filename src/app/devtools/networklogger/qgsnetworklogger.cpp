/***************************************************************************
    qgsnetworklogger.cpp
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

#include "qgsnetworklogger.h"
#include "qgsnetworkloggernode.h"
#include "qgssettings.h"
#include "qgis.h"
#include <QThread>
#include <QApplication>
#include <QUrlQuery>

QgsNetworkLogger::QgsNetworkLogger( QgsNetworkAccessManager *manager, QObject *parent )
  : QAbstractItemModel( parent )
  , mNam( manager )
  , mRootNode( std::make_unique< QgsNetworkLoggerRootNode >() )
{
  // logger must be created on the main thread
  Q_ASSERT( QThread::currentThread() == QApplication::instance()->thread() );
  Q_ASSERT( mNam->thread() == QApplication::instance()->thread() );

  if ( QgsSettings().value( QStringLiteral( "logNetworkRequests" ), false, QgsSettings::App ).toBool() )
    enableLogging( true );
}

bool QgsNetworkLogger::isLogging() const
{
  return mIsLogging;
}

QgsNetworkLogger::~QgsNetworkLogger() = default;

void QgsNetworkLogger::enableLogging( bool enabled )
{
  if ( enabled )
  {
    connect( mNam, qOverload< QgsNetworkRequestParameters >( &QgsNetworkAccessManager::requestAboutToBeCreated ), this, &QgsNetworkLogger::requestAboutToBeCreated, Qt::UniqueConnection );
    connect( mNam, qOverload< QgsNetworkReplyContent >( &QgsNetworkAccessManager::finished ), this, &QgsNetworkLogger::requestFinished, Qt::UniqueConnection );
    connect( mNam, qOverload< QgsNetworkRequestParameters >( &QgsNetworkAccessManager::requestTimedOut ), this, &QgsNetworkLogger::requestTimedOut, Qt::UniqueConnection );
    connect( mNam, &QgsNetworkAccessManager::downloadProgress, this, &QgsNetworkLogger::downloadProgress, Qt::UniqueConnection );
    connect( mNam, &QgsNetworkAccessManager::requestEncounteredSslErrors, this, &QgsNetworkLogger::requestEncounteredSslErrors, Qt::UniqueConnection );
  }
  else
  {
    disconnect( mNam, qOverload< QgsNetworkRequestParameters >( &QgsNetworkAccessManager::requestAboutToBeCreated ), this, &QgsNetworkLogger::requestAboutToBeCreated );
    disconnect( mNam, qOverload< QgsNetworkReplyContent >( &QgsNetworkAccessManager::finished ), this, &QgsNetworkLogger::requestFinished );
    disconnect( mNam, qOverload< QgsNetworkRequestParameters >( &QgsNetworkAccessManager::requestTimedOut ), this, &QgsNetworkLogger::requestTimedOut );
    disconnect( mNam, &QgsNetworkAccessManager::downloadProgress, this, &QgsNetworkLogger::downloadProgress );
    disconnect( mNam, &QgsNetworkAccessManager::requestEncounteredSslErrors, this, &QgsNetworkLogger::requestEncounteredSslErrors );
  }
  mIsLogging = enabled;
}

void QgsNetworkLogger::clear()
{
  beginResetModel();
  mRequestGroups.clear();
  mRootNode->clear();
  endResetModel();
}

void QgsNetworkLogger::requestAboutToBeCreated( QgsNetworkRequestParameters parameters )
{
  const int childCount = mRootNode->childCount();

  beginInsertRows( QModelIndex(), childCount, childCount );

  std::unique_ptr< QgsNetworkLoggerRequestGroup > group = std::make_unique< QgsNetworkLoggerRequestGroup >( parameters );
  mRequestGroups.insert( parameters.requestId(), group.get() );
  mRootNode->addChild( std::move( group ) );
  endInsertRows();
}

void QgsNetworkLogger::requestFinished( QgsNetworkReplyContent content )
{
  QgsNetworkLoggerRequestGroup *requestGroup = mRequestGroups.value( content.requestId() );
  if ( !requestGroup )
    return;

  // find the row: the position of the request in the rootNode
  const QModelIndex requestIndex = node2index( requestGroup );
  if ( !requestIndex.isValid() )
    return;

  beginInsertRows( requestIndex, requestGroup->childCount(), requestGroup->childCount() );
  requestGroup->setReply( content );
  endInsertRows();

  emit dataChanged( requestIndex, requestIndex );
}

void QgsNetworkLogger::requestTimedOut( QgsNetworkRequestParameters parameters )
{
  QgsNetworkLoggerRequestGroup *requestGroup = mRequestGroups.value( parameters.requestId() );
  if ( !requestGroup )
    return;

  const QModelIndex requestIndex = node2index( requestGroup );
  if ( !requestIndex.isValid() )
    return;

  requestGroup->setTimedOut();

  emit dataChanged( requestIndex, requestIndex );
}

void QgsNetworkLogger::downloadProgress( int requestId, qint64 bytesReceived, qint64 bytesTotal )
{
  QgsNetworkLoggerRequestGroup *requestGroup = mRequestGroups.value( requestId );
  if ( !requestGroup )
    return;

  const QModelIndex requestIndex = node2index( requestGroup );
  if ( !requestIndex.isValid() )
    return;

  requestGroup->setProgress( bytesReceived, bytesTotal );

  emit dataChanged( requestIndex, requestIndex, QVector<int >() << Qt::ToolTipRole );
}

void QgsNetworkLogger::requestEncounteredSslErrors( int requestId, const QList<QSslError> &errors )
{
  QgsNetworkLoggerRequestGroup *requestGroup = mRequestGroups.value( requestId );
  if ( !requestGroup )
    return;

  const QModelIndex requestIndex = node2index( requestGroup );
  if ( !requestIndex.isValid() )
    return;

  beginInsertRows( requestIndex, requestGroup->childCount(), requestGroup->childCount() );
  requestGroup->setSslErrors( errors );
  endInsertRows();

  emit dataChanged( requestIndex, requestIndex );
}

QgsNetworkLoggerNode *QgsNetworkLogger::index2node( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return mRootNode.get();

  return reinterpret_cast<QgsNetworkLoggerNode *>( index.internalPointer() );
}

QList<QAction *> QgsNetworkLogger::actions( const QModelIndex &index, QObject *parent )
{
  QgsNetworkLoggerNode *node = index2node( index );
  if ( !node )
    return QList< QAction * >();

  return node->actions( parent );
}

QModelIndex QgsNetworkLogger::node2index( QgsNetworkLoggerNode *node ) const
{
  if ( !node || !node->parent() )
    return QModelIndex(); // this is the only root item -> invalid index

  QModelIndex parentIndex = node2index( node->parent() );

  int row = node->parent()->indexOf( node );
  Q_ASSERT( row >= 0 );
  return index( row, 0, parentIndex );
}

QModelIndex QgsNetworkLogger::indexOfParentLayerTreeNode( QgsNetworkLoggerNode *parentNode ) const
{
  Q_ASSERT( parentNode );

  QgsNetworkLoggerGroup *grandParentNode = parentNode->parent();
  if ( !grandParentNode )
    return QModelIndex();  // root node -> invalid index

  int row = grandParentNode->indexOf( parentNode );
  Q_ASSERT( row >= 0 );

  return createIndex( row, 0, parentNode );
}

void QgsNetworkLogger::removeRequestRows( const QList<int> &rows )
{
  QList< int > res = rows;
  std::sort( res.begin(), res.end(), std::greater< int >() );

  for ( int row : std::as_const( res ) )
  {
    int popId = data( index( row, 0, QModelIndex() ), QgsNetworkLoggerNode::RoleId ).toInt();
    mRequestGroups.remove( popId );

    beginRemoveRows( QModelIndex(), row, row );
    mRootNode->removeRow( row );
    endRemoveRows();
  }
}

QgsNetworkLoggerRootNode *QgsNetworkLogger::rootGroup()
{
  return mRootNode.get();
}

int QgsNetworkLogger::rowCount( const QModelIndex &parent ) const
{
  QgsNetworkLoggerNode *n = index2node( parent );
  if ( !n )
    return 0;

  return n->childCount();
}

int QgsNetworkLogger::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 1;
}

QModelIndex QgsNetworkLogger::index( int row, int column, const QModelIndex &parent ) const
{
  if ( column < 0 || column >= columnCount( parent ) ||
       row < 0 || row >= rowCount( parent ) )
    return QModelIndex();

  QgsNetworkLoggerGroup *n = dynamic_cast< QgsNetworkLoggerGroup * >( index2node( parent ) );
  if ( !n )
    return QModelIndex(); // have no children

  return createIndex( row, column, n->childAt( row ) );
}

QModelIndex QgsNetworkLogger::parent( const QModelIndex &child ) const
{
  if ( !child.isValid() )
    return QModelIndex();

  if ( QgsNetworkLoggerNode *n = index2node( child ) )
  {
    return indexOfParentLayerTreeNode( n->parent() ); // must not be null
  }
  else
  {
    Q_ASSERT( false );
    return QModelIndex();
  }
}

QVariant QgsNetworkLogger::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || index.column() > 1 )
    return QVariant();

  QgsNetworkLoggerNode *node = index2node( index );
  if ( !node )
    return QVariant();

  return node->data( role );
}

Qt::ItemFlags QgsNetworkLogger::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
  {
    Qt::ItemFlags rootFlags = Qt::ItemFlags();
    return rootFlags;
  }

  Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  return f;
}

QVariant QgsNetworkLogger::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( section == 0 && orientation == Qt::Horizontal && role == Qt::DisplayRole )
    return tr( "Requests" );
  return QVariant();
}


//
// QgsNetworkLoggerProxyModel
//

QgsNetworkLoggerProxyModel::QgsNetworkLoggerProxyModel( QgsNetworkLogger *logger, QObject *parent )
  : QSortFilterProxyModel( parent )
  , mLogger( logger )
{
  setSourceModel( mLogger );
}

void QgsNetworkLoggerProxyModel::setFilterString( const QString &string )
{
  mFilterString = string;
  invalidateFilter();
}

void QgsNetworkLoggerProxyModel::setShowSuccessful( bool show )
{
  mShowSuccessful = show;
  invalidateFilter();
}

void QgsNetworkLoggerProxyModel::setShowTimeouts( bool show )
{
  mShowTimeouts = show;
  invalidateFilter();
}

void QgsNetworkLoggerProxyModel::setShowCached( bool show )
{
  mShowCached = show;
  invalidateFilter();
}

bool QgsNetworkLoggerProxyModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
{
  QgsNetworkLoggerNode *node = mLogger->index2node( mLogger->index( source_row, 0, source_parent ) );
  if ( QgsNetworkLoggerRequestGroup *request = dynamic_cast< QgsNetworkLoggerRequestGroup * >( node ) )
  {
    if ( ( request->status() == QgsNetworkLoggerRequestGroup::Status::Complete || request->status() == QgsNetworkLoggerRequestGroup::Status::Canceled )
         & !mShowSuccessful )
      return false;
    else if ( request->status() == QgsNetworkLoggerRequestGroup::Status::TimeOut && !mShowTimeouts )
      return false;
    else if ( request->replyFromCache() && !mShowCached )
      return false;
    return mFilterString.isEmpty() || request->url().url().contains( mFilterString, Qt::CaseInsensitive );
  }

  return true;
}
