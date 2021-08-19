/***************************************************************************
    qgsruntimeprofiler.cpp
    ---------------------
    begin                : June 2016
    copyright            : (C) 2016 by Nathan Woodrow
    email                : woodrow dot nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsruntimeprofiler.h"
#include "qgslogger.h"
#include "qgis.h"
#include "qgsapplication.h"
#include <QSet>
#include <QThreadStorage>

QgsRuntimeProfiler *QgsRuntimeProfiler::sMainProfiler = nullptr;


//
// QgsRuntimeProfilerNode
//

QgsRuntimeProfilerNode::QgsRuntimeProfilerNode( const QString &group, const QString &name )
  : mName( name )
  , mGroup( group )
{

}

QgsRuntimeProfilerNode::~QgsRuntimeProfilerNode() = default;

QStringList QgsRuntimeProfilerNode::fullParentPath() const
{
  QStringList res;
  if ( mParent )
  {
    res = mParent->fullParentPath();
    const QString parentName = mParent->data( Name ).toString();
    if ( !parentName.isEmpty() )
      res << parentName;
  }
  return res;
}

QVariant QgsRuntimeProfilerNode::data( int role ) const
{
  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
    case Name:
      return mName;

    case Group:
      return mGroup;

    case Elapsed:
      return mElapsed;

    case ParentElapsed:
      return mParent ? ( mParent->elapsed() > 0 ? mParent->elapsed() : mParent->totalElapsedTimeForChildren( mGroup ) ) : 0;
  }
  return QVariant();
}

void QgsRuntimeProfilerNode::addChild( std::unique_ptr<QgsRuntimeProfilerNode> child )
{
  if ( !child )
    return;

  Q_ASSERT( !child->mParent );
  child->mParent = this;

  mChildren.emplace_back( std::move( child ) );
}

int QgsRuntimeProfilerNode::indexOf( QgsRuntimeProfilerNode *child ) const
{
  Q_ASSERT( child->mParent == this );
  const auto it = std::find_if( mChildren.begin(), mChildren.end(), [&]( const std::unique_ptr<QgsRuntimeProfilerNode> &p )
  {
    return p.get() == child;
  } );
  if ( it != mChildren.end() )
    return std::distance( mChildren.begin(), it );
  return -1;
}

QgsRuntimeProfilerNode *QgsRuntimeProfilerNode::child( const QString &group, const QString &name )
{
  for ( auto &it : mChildren )
  {
    if ( it->data( Group ).toString() == group &&  it->data( Name ).toString() == name )
      return it.get();
  }
  return nullptr;
}

QgsRuntimeProfilerNode *QgsRuntimeProfilerNode::childAt( int index )
{
  Q_ASSERT( static_cast< std::size_t >( index ) < mChildren.size() );
  return mChildren[ index ].get();
}

void QgsRuntimeProfilerNode::clear()
{
  mChildren.clear();
}

void QgsRuntimeProfilerNode::removeChildAt( int index )
{
  Q_ASSERT( static_cast< std::size_t >( index ) < mChildren.size() );
  mChildren.erase( mChildren.begin() + index );
}

void QgsRuntimeProfilerNode::start()
{
  mProfileTime.restart();
}

void QgsRuntimeProfilerNode::stop()
{
  mElapsed = mProfileTime.elapsed() / 1000.0;
}

void QgsRuntimeProfilerNode::setElapsed( double time )
{
  mElapsed = time;
}

double QgsRuntimeProfilerNode::elapsed() const
{
  return mElapsed;
}

double QgsRuntimeProfilerNode::totalElapsedTimeForChildren( const QString &group ) const
{
  double total = 0;
  for ( auto &it : mChildren )
  {
    if ( it->data( Group ).toString() == group )
      total += it->elapsed();
  }
  return total;
}

//
// QgsRuntimeProfiler
//

QgsRuntimeProfiler::QgsRuntimeProfiler()
  : mRootNode( std::make_unique< QgsRuntimeProfilerNode >( QString(), QString() ) )
{

}

QgsRuntimeProfiler::~QgsRuntimeProfiler() = default;

QgsRuntimeProfiler *QgsRuntimeProfiler::threadLocalInstance()
{
  static QThreadStorage<QgsRuntimeProfiler> sInstances;
  QgsRuntimeProfiler *profiler = &sInstances.localData();

  if ( !qApp || profiler->thread() == qApp->thread() )
    sMainProfiler = profiler;

  if ( !profiler->mInitialized )
    profiler->setupConnections();

  return profiler;
}

void QgsRuntimeProfiler::beginGroup( const QString &name )
{
  start( name );
}

void QgsRuntimeProfiler::endGroup()
{
  end();
}

QStringList QgsRuntimeProfiler::childGroups( const QString &parent, const QString &group ) const
{
  QgsRuntimeProfilerNode *parentNode = pathToNode( group, parent );
  if ( !parentNode )
    return QStringList();

  QStringList res;
  res.reserve( parentNode->childCount() );
  for ( int i = 0; i < parentNode->childCount(); ++i )
  {
    QgsRuntimeProfilerNode *child = parentNode->childAt( i );
    if ( child->data( QgsRuntimeProfilerNode::Group ).toString() == group )
      res << child->data( QgsRuntimeProfilerNode::Name ).toString();
  }
  return res;
}

void QgsRuntimeProfiler::start( const QString &name, const QString &group )
{
  std::unique_ptr< QgsRuntimeProfilerNode > node = std::make_unique< QgsRuntimeProfilerNode >( group, name );
  node->start();

  QgsRuntimeProfilerNode *child = node.get();
  if ( !mCurrentStack[ group ].empty() )
  {
    QgsRuntimeProfilerNode *parent = mCurrentStack[group ].top();

    const QModelIndex parentIndex = node2index( parent );
    beginInsertRows( parentIndex, parent->childCount(), parent->childCount() );
    parent->addChild( std::move( node ) );
    endInsertRows();
  }
  else
  {
    beginInsertRows( QModelIndex(), mRootNode->childCount(), mRootNode->childCount() );
    mRootNode->addChild( std::move( node ) );
    endInsertRows();
  }

  mCurrentStack[group].push( child );
  emit started( group, child->fullParentPath(), name );

  if ( !mGroups.contains( group ) )
  {
    mGroups.insert( group );
    emit groupAdded( group );
  }
}

void QgsRuntimeProfiler::end( const QString &group )
{
  if ( mCurrentStack[group].empty() )
    return;

  QgsRuntimeProfilerNode *node = mCurrentStack[group].top();
  mCurrentStack[group].pop();
  node->stop();

  const QModelIndex nodeIndex = node2index( node );
  const QModelIndex col2Index = index( nodeIndex.row(), 1, nodeIndex.parent() );
  emit dataChanged( nodeIndex, nodeIndex );
  emit dataChanged( col2Index, col2Index );
  // parent item has data changed too, cos the overall time elapsed will have changed!
  QModelIndex parentIndex = nodeIndex.parent();
  while ( parentIndex.isValid() )
  {
    const QModelIndex parentCol2Index = index( parentIndex.row(), 1, parentIndex.parent() );
    emit dataChanged( parentIndex, parentIndex );
    emit dataChanged( parentCol2Index, parentCol2Index );
    parentIndex = parentIndex.parent();
  }

  emit ended( group, node->fullParentPath(), node->data( QgsRuntimeProfilerNode::Name ).toString(), node->data( QgsRuntimeProfilerNode::Elapsed ).toDouble() );
}

double QgsRuntimeProfiler::profileTime( const QString &name, const QString &group ) const
{
  QgsRuntimeProfilerNode *node = pathToNode( group, name );
  if ( !node )
    return 0;

  return node->data( QgsRuntimeProfilerNode::Elapsed ).toDouble();
}

void QgsRuntimeProfiler::clear( const QString &group )
{
  for ( int row = mRootNode->childCount() - 1; row >= 0; row-- )
  {
    if ( mRootNode->childAt( row )->data( QgsRuntimeProfilerNode::Group ).toString() == group )
    {
      beginRemoveRows( QModelIndex(), row, row );
      mRootNode->removeChildAt( row );
      endRemoveRows();
    }
  }
}

double QgsRuntimeProfiler::totalTime( const QString &group )
{
  if ( QgsRuntimeProfilerNode *node = pathToNode( group, QString() ) )
    return node->elapsed();

  return 0;
}

bool QgsRuntimeProfiler::groupIsActive( const QString &group ) const
{
  return !mCurrentStack.value( group ).empty();
}

QString QgsRuntimeProfiler::translateGroupName( const QString &group )
{
  if ( group == QLatin1String( "startup" ) )
    return tr( "Startup" );
  else if ( group == QLatin1String( "projectload" ) )
    return tr( "Project Load" );
  else if ( group == QLatin1String( "render" ) )
    return tr( "Map Render" );
  return QString();
}

int QgsRuntimeProfiler::rowCount( const QModelIndex &parent ) const
{
  QgsRuntimeProfilerNode *n = index2node( parent );
  if ( !n )
    return 0;

  return n->childCount();
}

int QgsRuntimeProfiler::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 2;
}

QModelIndex QgsRuntimeProfiler::index( int row, int column, const QModelIndex &parent ) const
{
  if ( column < 0 || column >= columnCount( parent ) ||
       row < 0 || row >= rowCount( parent ) )
    return QModelIndex();

  QgsRuntimeProfilerNode *n = index2node( parent );
  if ( !n )
    return QModelIndex(); // have no children

  return createIndex( row, column, n->childAt( row ) );
}

QModelIndex QgsRuntimeProfiler::parent( const QModelIndex &child ) const
{
  if ( !child.isValid() )
    return QModelIndex();

  if ( QgsRuntimeProfilerNode *n = index2node( child ) )
  {
    return indexOfParentNode( n->parent() ); // must not be null
  }
  else
  {
    Q_ASSERT( false );
    return QModelIndex();
  }
}

QVariant QgsRuntimeProfiler::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || index.column() > 2 )
    return QVariant();

  QgsRuntimeProfilerNode *node = index2node( index );
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
        case Qt::InitialSortOrderRole:
          return node->data( QgsRuntimeProfilerNode::Elapsed );

        default:
          break;
      }
      return node->data( role );
    }
  }
  return QVariant();
}

QVariant QgsRuntimeProfiler::headerData( int section, Qt::Orientation orientation, int role ) const
{
  switch ( role )
  {
    case Qt::DisplayRole:
    {
      if ( orientation == Qt::Horizontal )
      {
        switch ( section )
        {
          case 0:
            return tr( "Task" );
          case 1:
            return tr( "Time (seconds)" );
          default:
            return QVariant();
        }
      }
      else
      {
        return QVariant();
      }
    }

    default:
      return QAbstractItemModel::headerData( section, orientation, role );
  }
}

void QgsRuntimeProfiler::otherProfilerStarted( const QString &group, const QStringList &path, const QString &name )
{
  QgsRuntimeProfilerNode *parentNode = mRootNode.get();
  for ( const QString &part : path )
  {
    QgsRuntimeProfilerNode *child = parentNode->child( group, part );
    if ( !child )
    {
      std::unique_ptr< QgsRuntimeProfilerNode > newChild = std::make_unique< QgsRuntimeProfilerNode >( group, part );

      const QModelIndex parentIndex = node2index( parentNode );
      beginInsertRows( parentIndex, parentNode->childCount(), parentNode->childCount() );
      QgsRuntimeProfilerNode *next = newChild.get();
      parentNode->addChild( std::move( newChild ) );
      endInsertRows();
      parentNode = next;
    }
    else
    {
      parentNode = child;
    }
  }

  if ( parentNode->child( group, name ) )
    return;

  const QModelIndex parentIndex = node2index( parentNode );
  beginInsertRows( parentIndex, parentNode->childCount(), parentNode->childCount() );
  parentNode->addChild( std::make_unique< QgsRuntimeProfilerNode >( group, name ) );
  endInsertRows();

  if ( !mGroups.contains( group ) )
  {
    mGroups.insert( group );
    emit groupAdded( group );
  }
}

void QgsRuntimeProfiler::otherProfilerEnded( const QString &group, const QStringList &path, const QString &name, double elapsed )
{
  QgsRuntimeProfilerNode *parentNode = mRootNode.get();
  for ( const QString &part : path )
  {
    QgsRuntimeProfilerNode *child = parentNode->child( group, part );
    if ( !child )
    {
      std::unique_ptr< QgsRuntimeProfilerNode > newChild = std::make_unique< QgsRuntimeProfilerNode >( group, part );

      const QModelIndex parentIndex = node2index( parentNode );
      beginInsertRows( parentIndex, parentNode->childCount(), parentNode->childCount() );
      QgsRuntimeProfilerNode *next = newChild.get();
      parentNode->addChild( std::move( newChild ) );
      endInsertRows();
      parentNode = next;
    }
    else
    {
      parentNode = child;
    }
  }

  QgsRuntimeProfilerNode *destNode = parentNode->child( group, name );
  if ( !destNode )
  {
    std::unique_ptr< QgsRuntimeProfilerNode > node = std::make_unique< QgsRuntimeProfilerNode >( group, name );
    destNode = node.get();
    const QModelIndex parentIndex = node2index( parentNode );
    beginInsertRows( parentIndex, parentNode->childCount(), parentNode->childCount() );
    parentNode->addChild( std::move( node ) );
    endInsertRows();
  }

  destNode->setElapsed( elapsed );

  const QModelIndex nodeIndex = node2index( destNode );
  const QModelIndex col2Index = index( nodeIndex.row(), 1, nodeIndex.parent() );
  emit dataChanged( nodeIndex, nodeIndex );
  emit dataChanged( col2Index, col2Index );
  // parent item has data changed too, cos the overall time elapsed will have changed!
  QModelIndex parentIndex = nodeIndex.parent();
  while ( parentIndex.isValid() )
  {
    const QModelIndex parentCol2Index = index( parentIndex.row(), 1, parentIndex.parent() );
    emit dataChanged( parentIndex, parentIndex );
    emit dataChanged( parentCol2Index, parentCol2Index );
    parentIndex = parentIndex.parent();
  }
}

void QgsRuntimeProfiler::setupConnections()
{
  mInitialized = true;

  Q_ASSERT( sMainProfiler );

  if ( sMainProfiler != this )
  {
    connect( this, &QgsRuntimeProfiler::started, sMainProfiler, &QgsRuntimeProfiler::otherProfilerStarted );
    connect( this, &QgsRuntimeProfiler::ended, sMainProfiler, &QgsRuntimeProfiler::otherProfilerEnded );
  }
}

QgsRuntimeProfilerNode *QgsRuntimeProfiler::pathToNode( const QString &group, const QString &path ) const
{
  const QStringList parts = path.split( '/' );
  QgsRuntimeProfilerNode *res = mRootNode.get();
  for ( const QString &part : parts )
  {
    if ( part.isEmpty() )
      continue;

    res = res->child( group, part );
    if ( !res )
      break;
  }
  return res;
}

QgsRuntimeProfilerNode *QgsRuntimeProfiler::pathToNode( const QString &group, const QStringList &path ) const
{
  QgsRuntimeProfilerNode *res = mRootNode.get();
  for ( const QString &part : path )
  {
    res = res->child( group, part );
    if ( !res )
      break;
  }
  return res;
}

QModelIndex QgsRuntimeProfiler::node2index( QgsRuntimeProfilerNode *node ) const
{
  if ( !node || !node->parent() )
    return QModelIndex(); // this is the only root item -> invalid index

  const QModelIndex parentIndex = node2index( node->parent() );

  const int row = node->parent()->indexOf( node );
  Q_ASSERT( row >= 0 );
  return index( row, 0, parentIndex );
}

QModelIndex QgsRuntimeProfiler::indexOfParentNode( QgsRuntimeProfilerNode *parentNode ) const
{
  Q_ASSERT( parentNode );

  QgsRuntimeProfilerNode *grandParentNode = parentNode->parent();
  if ( !grandParentNode )
    return QModelIndex();  // root node -> invalid index

  const int row = grandParentNode->indexOf( parentNode );
  Q_ASSERT( row >= 0 );

  return createIndex( row, 0, parentNode );
}

QgsRuntimeProfilerNode *QgsRuntimeProfiler::index2node( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return mRootNode.get();

  return reinterpret_cast<QgsRuntimeProfilerNode *>( index.internalPointer() );
}


//
// QgsScopedRuntimeProfile
//

QgsScopedRuntimeProfile::QgsScopedRuntimeProfile( const QString &name, const QString &group )
  : mGroup( group )
{
  QgsApplication::profiler()->start( name, mGroup );
}

QgsScopedRuntimeProfile::~QgsScopedRuntimeProfile()
{
  QgsApplication::profiler()->end( mGroup );
}

void QgsScopedRuntimeProfile::switchTask( const QString &name )
{
  QgsApplication::profiler()->end( mGroup );
  QgsApplication::profiler()->start( name, mGroup );
}
