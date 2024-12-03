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
#include "moc_qgsappquerylogger.cpp"
#include "qgsdatabasequeryloggernode.h"
#include "qgsapplication.h"
#include "devtools/qgsdevtoolsmodelnode.h"
#include "qgssettings.h"
#include "qgis.h"
#include <QThread>
#include <QApplication>
#include <QUrlQuery>
#include <QPainter>

QgsAppQueryLogger::QgsAppQueryLogger( QObject *parent )
  : QAbstractItemModel( parent )
  , mRootNode( std::make_unique<QgsDatabaseQueryLoggerRootNode>() )
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
  mMaxCost = 0;
  mRootNode->clear();
  endResetModel();
}

void QgsAppQueryLogger::queryLogged( const QgsDatabaseQueryLogEntry &query )
{
  const int childCount = mRootNode->childCount();

  beginInsertRows( QModelIndex(), childCount, childCount );

  std::unique_ptr<QgsDatabaseQueryLoggerQueryGroup> group = std::make_unique<QgsDatabaseQueryLoggerQueryGroup>( query );
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

  const long long newMaxCost = std::max<long long>( static_cast<long long>( query.finishedTime - query.startedTime ), mMaxCost );

  // Calculate the number of children: if error or not fetched rows 1 row is added else 2 rows are added
  beginInsertRows( requestIndex, queryGroup->childCount(), queryGroup->childCount() + ( query.fetchedRows != -1 ? 1 : 0 ) );
  queryGroup->setFinished( query );
  endInsertRows();

  emit dataChanged( requestIndex, requestIndex );

  if ( newMaxCost > mMaxCost )
  {
    mMaxCost = newMaxCost;
    emit dataChanged( index( 0, 1 ), index( rowCount(), 1 ) );
  }
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
    return QList<QAction *>();

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
    return QModelIndex(); // root node -> invalid index

  int row = grandParentNode->indexOf( parentNode );
  Q_ASSERT( row >= 0 );

  return createIndex( row, 0, parentNode );
}

void QgsAppQueryLogger::removeRequestRows( const QList<int> &rows )
{
  QList<int> res = rows;
  std::sort( res.begin(), res.end(), std::greater<int>() );

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
  return 2;
}

QModelIndex QgsAppQueryLogger::index( int row, int column, const QModelIndex &parent ) const
{
  if ( column < 0 || column >= columnCount( parent ) || row < 0 || row >= rowCount( parent ) )
    return QModelIndex();

  QgsDevToolsModelGroup *n = dynamic_cast<QgsDevToolsModelGroup *>( index2node( parent ) );
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
  if ( !index.isValid() || index.column() > columnCount() )
    return QVariant();

  QgsDevToolsModelNode *node = index2node( index );
  if ( !node )
    return QVariant();

  switch ( index.column() )
  {
    case 0:
      return node->data( role );

    case 1:
    {
      switch ( role )
      {
        case Qt::DisplayRole:
        case QgsDevToolsModelNode::RoleElapsedTime:
        case QgsDevToolsModelNode::RoleSort:
          return node->data( QgsDevToolsModelNode::RoleElapsedTime );

        case QgsDevToolsModelNode::RoleMaximumTime:
          return mMaxCost;

        default:
          break;
      }
      return node->data( role );
    }
  }
  return QVariant();
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
  if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
  {
    switch ( section )
    {
      case 0:
        return tr( "Query" );
      case 1:
        return tr( "Time (ms)" );
    }
  }
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
  if ( !mFilterString.isEmpty() )
  {
    QgsDevToolsModelNode *node = mLogger->index2node( mLogger->index( source_row, 0, source_parent ) );
    if ( QgsDatabaseQueryLoggerQueryGroup *request = dynamic_cast<QgsDatabaseQueryLoggerQueryGroup *>( node ) )
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

//
// QueryCostDelegate
//

QueryCostDelegate::QueryCostDelegate( int sortRole, int totalCostRole, QObject *parent )
  : QStyledItemDelegate( parent )
  , mSortRole( sortRole )
  , mTotalCostRole( totalCostRole )
{
}

QueryCostDelegate::~QueryCostDelegate() = default;

void QueryCostDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  const auto cost = index.data( mSortRole ).toDouble();
  if ( cost <= 0 )
  {
    QStyledItemDelegate::paint( painter, option, index );
    return;
  }

  const auto totalCost = index.data( mTotalCostRole ).toDouble();
  const auto fraction = std::abs( float( cost ) / totalCost );

  auto rect = option.rect;
  rect.setWidth( static_cast<int>( rect.width() * fraction ) );

  const auto &brush = painter->brush();
  const auto &pen = painter->pen();

  painter->setPen( Qt::NoPen );

  if ( option.features & QStyleOptionViewItem::Alternate )
  {
    // we must handle this ourselves as otherwise the custom background
    // would get painted over with the alternate background color
    painter->setBrush( option.palette.alternateBase() );
    painter->drawRect( option.rect );
  }

  const auto color = QColor::fromHsv( static_cast<int>( 120 - fraction * 120 ), 255, 255, static_cast<int>( ( -( ( fraction - 1 ) * ( fraction - 1 ) ) ) * 120 + 120 ) );
  painter->setBrush( color );
  painter->drawRect( rect );

  painter->setBrush( brush );
  painter->setPen( pen );

  if ( option.features & QStyleOptionViewItem::Alternate )
  {
    auto o = option;
    o.features &= ~QStyleOptionViewItem::Alternate;
    QStyledItemDelegate::paint( painter, o, index );
  }
  else
  {
    QStyledItemDelegate::paint( painter, option, index );
  }
}
