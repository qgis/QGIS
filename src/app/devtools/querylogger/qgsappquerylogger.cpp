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
#include "qgsdatabasequeryloggernode.h"
#include "qgsapplication.h"
#include "devtools/qgsdevtoolsmodelnode.h"
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

  connect( QgsApplication::databaseQueryLog(), &QgsDatabaseQueryLog::queryStarted, this, &QgsAppQueryLogger::queryLogged );
  connect( QgsApplication::databaseQueryLog(), &QgsDatabaseQueryLog::queryFinished, this, &QgsAppQueryLogger::queryFinished );
}

QgsAppQueryLogger::~QgsAppQueryLogger() = default;

void QgsAppQueryLogger::clear()
{
  beginResetModel();
  mQueryGroups.clear();
  mRootNode->clear();
  endResetModel();
}

void QgsAppQueryLogger::queryLogged( const QgsDatabaseQueryLogEntry &query )
{
  const int childCount = mRootNode->childCount();

  beginInsertRows( QModelIndex(), childCount, childCount );

  std::unique_ptr< QgsDatabaseQueryLoggerQueryGroup > group = std::make_unique< QgsDatabaseQueryLoggerQueryGroup >( query );
  mQueryGroups.insert( query.queryId, group.get() );
  mRootNode->addChild( std::move( group ) );
  endInsertRows();
}

void QgsAppQueryLogger::queryFinished( const QgsDatabaseQueryLogEntry &query )
{
  QgsDatabaseQueryLoggerQueryGroup *queryGroup = mQueryGroups.value( query.queryId );
  if ( !queryGroup )
    return;

  // find the row: the position of the request in the rootNode
  const QModelIndex requestIndex = node2index( queryGroup );
  if ( !requestIndex.isValid() )
    return;

  if ( query.query != queryGroup->sql() )
  {
    queryGroup->setSql( query.query );
  }

  // Calculate the number of children: if error or not fetched rows 1 row is added else 2 rows are added
  beginInsertRows( requestIndex, queryGroup->childCount(), queryGroup->childCount() + ( query.fetchedRows != -1 ? 1 : 0 ) );
  queryGroup->setFinished( query );
  endInsertRows();

  emit dataChanged( requestIndex, requestIndex );
}

QgsDevToolsModelNode *QgsAppQueryLogger::index2node( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return mRootNode.get();

  return reinterpret_cast<QgsDevToolsModelNode *>( index.internalPointer() );
}

QList<QAction *> QgsAppQueryLogger::actions( const QModelIndex &index, QObject *parent )
{
  QgsDevToolsModelNode *node = index2node( index );
  if ( !node )
    return QList< QAction * >();

  return node->actions( parent );
}

QModelIndex QgsAppQueryLogger::node2index( QgsDevToolsModelNode *node ) const
{
  if ( !node || !node->parent() )
    return QModelIndex(); // this is the only root item -> invalid index

  QModelIndex parentIndex = node2index( node->parent() );

  int row = node->parent()->indexOf( node );
  Q_ASSERT( row >= 0 );
  return index( row, 0, parentIndex );
}

QModelIndex QgsAppQueryLogger::indexOfParentLayerTreeNode( QgsDevToolsModelNode *parentNode ) const
{
  Q_ASSERT( parentNode );

  QgsDevToolsModelGroup *grandParentNode = parentNode->parent();
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
    int popId = data( index( row, 0, QModelIndex() ), QgsDevToolsModelNode::RoleId ).toInt();
    mQueryGroups.remove( popId );

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
  QgsDevToolsModelNode *n = index2node( parent );
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

  QgsDevToolsModelGroup *n = dynamic_cast< QgsDevToolsModelGroup * >( index2node( parent ) );
  if ( !n )
    return QModelIndex(); // have no children

  return createIndex( row, column, n->childAt( row ) );
}

QModelIndex QgsAppQueryLogger::parent( const QModelIndex &child ) const
{
  if ( !child.isValid() )
    return QModelIndex();

  if ( QgsDevToolsModelNode *n = index2node( child ) )
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

  QgsDevToolsModelNode *node = index2node( index );
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
  if ( ! mFilterString.isEmpty() )
  {
    QgsDevToolsModelNode *node = mLogger->index2node( mLogger->index( source_row, 0, source_parent ) );
    if ( QgsDatabaseQueryLoggerQueryGroup *request = dynamic_cast< QgsDatabaseQueryLoggerQueryGroup * >( node ) )
    {
      if ( request->data().toString().contains( mFilterString, Qt::CaseInsensitive ) )
      {
        return true;
      }
      for ( int i = 0; i < request->childCount(); i++ )
      {
        if ( QgsDevToolsModelValueNode *valueNode = static_cast<QgsDevToolsModelValueNode *>( request->childAt( i ) ); valueNode->value().contains( mFilterString, Qt::CaseInsensitive ) )
        {
          return true;
        }
      }
      return false;
    }
  }
  return true;
}
