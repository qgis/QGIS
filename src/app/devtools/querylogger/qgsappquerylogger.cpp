/***************************************************************************
    qgsappquerylogger.cpp
    -------------------------
    begin                : October 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsappquerylogger.h"
#include "qgsqueryloggernode.h"
#include "qgsapplication.h"
#include "qgssettings.h"
#include "qgis.h"
#include <QThread>
#include <QApplication>
#include <QUrlQuery>

QgsAppQueryLogger::QgsAppQueryLogger( QObject *parent )
  : QAbstractItemModel( parent )
  , mRootNode( std::make_unique< QgsDatabaseQueryLoggerRootNode >() )
{
  // logger must be created on the main thread
  Q_ASSERT( QThread::currentThread() == QApplication::instance()->thread() );

  if ( QgsSettings().value( QStringLiteral( "logQueries" ), false, QgsSettings::App ).toBool() )
    enableLogging( true );
}

bool QgsAppQueryLogger::isLogging() const
{
  return mIsLogging;
}

QgsAppQueryLogger::~QgsAppQueryLogger() = default;

void QgsAppQueryLogger::enableLogging( bool enabled )
{
  if ( enabled )
  {
    connect( QgsApplication::databaseQueryLog(), &QgsDatabaseQueryLog::queryStarted, this, &QgsAppQueryLogger::queryLogged, Qt::UniqueConnection );
  }
  else
  {
    disconnect( QgsApplication::databaseQueryLog(), &QgsDatabaseQueryLog::queryStarted, this, &QgsAppQueryLogger::queryLogged );
  }
  mIsLogging = enabled;
}

void QgsAppQueryLogger::clear()
{
  beginResetModel();
  mRequestGroups.clear();
  mRootNode->clear();
  endResetModel();
}

void QgsAppQueryLogger::queryLogged( const QgsDatabaseQueryLogEntry &query )
{
  const int childCount = mRootNode->childCount();

  beginInsertRows( QModelIndex(), childCount, childCount );

  std::unique_ptr< QgsDatabaseQueryLoggerGroup > group = std::make_unique< QgsDatabaseQueryLoggerGroup >( query.query );
//  mRequestGroups.insert( parameters.requestId(), group.get() );
  mRootNode->addChild( std::move( group ) );
  endInsertRows();
}

QgsDatabaseQueryLoggerNode *QgsAppQueryLogger::index2node( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return mRootNode.get();

  return reinterpret_cast<QgsDatabaseQueryLoggerNode *>( index.internalPointer() );
}

QList<QAction *> QgsAppQueryLogger::actions( const QModelIndex &index, QObject *parent )
{
  QgsDatabaseQueryLoggerNode *node = index2node( index );
  if ( !node )
    return QList< QAction * >();

  return node->actions( parent );
}

QModelIndex QgsAppQueryLogger::node2index( QgsDatabaseQueryLoggerNode *node ) const
{
  if ( !node || !node->parent() )
    return QModelIndex(); // this is the only root item -> invalid index

  QModelIndex parentIndex = node2index( node->parent() );

  int row = node->parent()->indexOf( node );
  Q_ASSERT( row >= 0 );
  return index( row, 0, parentIndex );
}

QModelIndex QgsAppQueryLogger::indexOfParentLayerTreeNode( QgsDatabaseQueryLoggerNode *parentNode ) const
{
  Q_ASSERT( parentNode );

  QgsDatabaseQueryLoggerGroup *grandParentNode = parentNode->parent();
  if ( !grandParentNode )
    return QModelIndex();  // root node -> invalid index

  int row = grandParentNode->indexOf( parentNode );
  Q_ASSERT( row >= 0 );

  return createIndex( row, 0, parentNode );
}

void QgsAppQueryLogger::removeRequestRows( const QList<int> &rows )
{
  QList< int > res = rows;
  std::sort( res.begin(), res.end(), std::greater< int >() );

  for ( int row : std::as_const( res ) )
  {
    int popId = data( index( row, 0, QModelIndex() ), QgsDatabaseQueryLoggerNode::RoleId ).toInt();
    mRequestGroups.remove( popId );

    beginRemoveRows( QModelIndex(), row, row );
    mRootNode->removeRow( row );
    endRemoveRows();
  }
}

QgsDatabaseQueryLoggerRootNode *QgsAppQueryLogger::rootGroup()
{
  return mRootNode.get();
}

int QgsAppQueryLogger::rowCount( const QModelIndex &parent ) const
{
  QgsDatabaseQueryLoggerNode *n = index2node( parent );
  if ( !n )
    return 0;

  return n->childCount();
}

int QgsAppQueryLogger::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 1;
}

QModelIndex QgsAppQueryLogger::index( int row, int column, const QModelIndex &parent ) const
{
  if ( column < 0 || column >= columnCount( parent ) ||
       row < 0 || row >= rowCount( parent ) )
    return QModelIndex();

  QgsDatabaseQueryLoggerGroup *n = dynamic_cast< QgsDatabaseQueryLoggerGroup * >( index2node( parent ) );
  if ( !n )
    return QModelIndex(); // have no children

  return createIndex( row, column, n->childAt( row ) );
}

QModelIndex QgsAppQueryLogger::parent( const QModelIndex &child ) const
{
  if ( !child.isValid() )
    return QModelIndex();

  if ( QgsDatabaseQueryLoggerNode *n = index2node( child ) )
  {
    return indexOfParentLayerTreeNode( n->parent() ); // must not be null
  }
  else
  {
    Q_ASSERT( false );
    return QModelIndex();
  }
}

QVariant QgsAppQueryLogger::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || index.column() > 1 )
    return QVariant();

  QgsDatabaseQueryLoggerNode *node = index2node( index );
  if ( !node )
    return QVariant();

  return node->data( role );
}

Qt::ItemFlags QgsAppQueryLogger::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
  {
    Qt::ItemFlags rootFlags = Qt::ItemFlags();
    return rootFlags;
  }

  Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  return f;
}

QVariant QgsAppQueryLogger::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( section == 0 && orientation == Qt::Horizontal && role == Qt::DisplayRole )
    return tr( "Requests" );
  return QVariant();
}


//
// QgsDatabaseQueryLoggerProxyModel
//

QgsDatabaseQueryLoggerProxyModel::QgsDatabaseQueryLoggerProxyModel( QgsAppQueryLogger *logger, QObject *parent )
  : QSortFilterProxyModel( parent )
  , mLogger( logger )
{
  setSourceModel( mLogger );
}

void QgsDatabaseQueryLoggerProxyModel::setFilterString( const QString &string )
{
  mFilterString = string;
  invalidateFilter();
}

bool QgsDatabaseQueryLoggerProxyModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
{
  QgsDatabaseQueryLoggerNode *node = mLogger->index2node( mLogger->index( source_row, 0, source_parent ) );
#if 0
  if ( QgsDatabaseQueryLoggerRequestGroup *request = dynamic_cast< QgsDatabaseQueryLoggerRequestGroup * >( node ) )
  {
    return mFilterString.isEmpty() || request->url().url().contains( mFilterString, Qt::CaseInsensitive );
  }
#endif

  return true;
}
