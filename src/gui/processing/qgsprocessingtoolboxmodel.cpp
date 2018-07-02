/***************************************************************************
    QgsProcessingToolboxModel.cpp
    -------------------------------
    begin                : May 2018
    copyright            : (C) 2018 by Nyall Dawso
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingtoolboxmodel.h"
#include "qgsapplication.h"
#include "qgsprocessingregistry.h"
#include <functional>

#ifdef ENABLE_MODELTEST
#include "modeltest.h"
#endif

//
// QgsProcessingToolboxModelNode
//

QgsProcessingToolboxModelNode::~QgsProcessingToolboxModelNode()
{
  deleteChildren();
}

QgsProcessingToolboxModelGroupNode *QgsProcessingToolboxModelNode::getChildGroupNode( const QString &groupId )
{
  for ( QgsProcessingToolboxModelNode *node : qgis::as_const( mChildren ) )
  {
    if ( node->nodeType() == NodeGroup )
    {
      QgsProcessingToolboxModelGroupNode *groupNode = qobject_cast< QgsProcessingToolboxModelGroupNode * >( node );
      if ( groupNode && groupNode->id() == groupId )
        return groupNode;
    }
  }
  return nullptr;
}

void QgsProcessingToolboxModelNode::addChildNode( QgsProcessingToolboxModelNode *node )
{
  if ( !node )
    return;

  Q_ASSERT( !node->mParent );
  node->mParent = this;

  mChildren.append( node );
}

void QgsProcessingToolboxModelNode::deleteChildren()
{
  qDeleteAll( mChildren );
  mChildren.clear();
}

//
// QgsProcessingToolboxModelProviderNode
//

QgsProcessingToolboxModelProviderNode::QgsProcessingToolboxModelProviderNode( QgsProcessingProvider *provider )
  : mProvider( provider )
{}

QgsProcessingProvider *QgsProcessingToolboxModelProviderNode::provider()
{
  return mProvider;
}

//
// QgsProcessingToolboxModelGroupNode
//

QgsProcessingToolboxModelGroupNode::QgsProcessingToolboxModelGroupNode( const QString &id, const QString &name )
  : mId( id )
  , mName( name )
{}

//
// QgsProcessingToolboxModelAlgorithmNode
//

QgsProcessingToolboxModelAlgorithmNode::QgsProcessingToolboxModelAlgorithmNode( const QgsProcessingAlgorithm *algorithm )
  : mAlgorithm( algorithm )
{}

const QgsProcessingAlgorithm *QgsProcessingToolboxModelAlgorithmNode::algorithm() const
{
  return mAlgorithm;
}

//
// QgsProcessingToolboxModel
//

QgsProcessingToolboxModel::QgsProcessingToolboxModel( QObject *parent, QgsProcessingRegistry *registry )
  : QAbstractItemModel( parent )
  , mRegistry( registry ? registry : QgsApplication::processingRegistry() )
  , mRootNode( qgis::make_unique< QgsProcessingToolboxModelGroupNode >( QString(), QString() ) )
{
  rebuild();

  connect( mRegistry, &QgsProcessingRegistry::providerAdded, this, &QgsProcessingToolboxModel::providerAdded );
}

void QgsProcessingToolboxModel::rebuild()
{
  beginResetModel();

  mRootNode->deleteChildren();

  const QList< QgsProcessingProvider * > providers = mRegistry->providers();
  for ( QgsProcessingProvider *provider : providers )
  {
    addProvider( provider );
  }
  endResetModel();
}

void QgsProcessingToolboxModel::providerAdded( const QString &id )
{
  QgsProcessingProvider *provider = mRegistry->providerById( id );
  if ( !provider )
    return;

  beginResetModel();
  addProvider( provider );
  endResetModel();
}

QgsProcessingToolboxModelNode *QgsProcessingToolboxModel::index2node( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return mRootNode.get();

  QObject *obj = reinterpret_cast<QObject *>( index.internalPointer() );
  return qobject_cast<QgsProcessingToolboxModelNode *>( obj );
}

QModelIndex QgsProcessingToolboxModel::node2index( QgsProcessingToolboxModelNode *node ) const
{
  if ( !node || !node->parent() )
    return QModelIndex(); // this is the only root item -> invalid index

  QModelIndex parentIndex = node2index( node->parent() );

  int row = node->parent()->children().indexOf( node );
  Q_ASSERT( row >= 0 );
  return index( row, 0, parentIndex );
}

void QgsProcessingToolboxModel::addProvider( QgsProcessingProvider *provider )
{
  QgsProcessingToolboxModelNode *parentNode = nullptr;
  if ( !isTopLevelProvider( provider ) )
  {
    std::unique_ptr< QgsProcessingToolboxModelProviderNode > node = qgis::make_unique< QgsProcessingToolboxModelProviderNode >( provider );
    parentNode = node.get();
    mRootNode->addChildNode( node.release() );
  }
  else
  {
    parentNode = mRootNode.get();
  }

  const QList< const QgsProcessingAlgorithm * > algorithms = provider->algorithms();
  for ( const QgsProcessingAlgorithm *algorithm : algorithms )
  {
    std::unique_ptr< QgsProcessingToolboxModelAlgorithmNode > algorithmNode = qgis::make_unique< QgsProcessingToolboxModelAlgorithmNode >( algorithm );

    const QString groupId = algorithm->groupId();
    if ( !groupId.isEmpty() )
    {
      QgsProcessingToolboxModelGroupNode *groupNode = parentNode->getChildGroupNode( groupId );
      if ( !groupNode )
      {
        groupNode = new QgsProcessingToolboxModelGroupNode( algorithm->groupId(), algorithm->group() );
        parentNode->addChildNode( groupNode );
      }
      groupNode->addChildNode( algorithmNode.release() );
    }
    else
    {
      // "top level" algorithm - no group
      parentNode->addChildNode( algorithmNode.release() );
    }
  }
}

bool QgsProcessingToolboxModel::isTopLevelProvider( QgsProcessingProvider *provider )
{
  return provider->id() == QLatin1String( "qgis" ) ||
         provider->id() == QLatin1String( "native" ) ||
         provider->id() == QLatin1String( "3d" );
}

Qt::ItemFlags QgsProcessingToolboxModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return nullptr;

  return QAbstractItemModel::flags( index );
}

QVariant QgsProcessingToolboxModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  if ( role == RoleNodeType )
  {
    if ( QgsProcessingToolboxModelNode *node = index2node( index ) )
      return node->nodeType();
    else
      return QVariant();
  }

  QgsProcessingProvider *provider = providerForIndex( index );
  QgsProcessingToolboxModelGroupNode *groupNode = qobject_cast< QgsProcessingToolboxModelGroupNode * >( index2node( index ) );
  const QgsProcessingAlgorithm *algorithm = algorithmForIndex( index );

  switch ( role )
  {
    case Qt::DisplayRole:
    {
      switch ( index.column() )
      {
        case 0:
          if ( provider )
            return provider->name();
          else if ( algorithm )
            return algorithm->displayName();
          else if ( groupNode )
            return groupNode->name();
          else
            return QVariant();

        default:
          return QVariant();
      }
      break;
    }

    case Qt::ToolTipRole:
    {
      if ( provider )
        return provider->longName();
      else if ( algorithm )
        return algorithm->displayName();
      else if ( groupNode )
        return groupNode->name();
      else
        return QVariant();
    }

    case Qt::DecorationRole:
      switch ( index.column() )
      {
        case 0:
        {
          if ( provider )
            return provider->icon();
          else if ( algorithm )
            return algorithm->icon();
          else if ( !index.parent().isValid() )
            // top level groups get the QGIS icon
            return QgsApplication::getThemeIcon( QStringLiteral( "/providerQgis.svg" ) );
          else
            return QVariant();
        }

        default:
          return QVariant();
      }
      break;

    case RoleAlgorithmFlags:
      switch ( index.column() )
      {
        case 0:
        {
          if ( algorithm )
            return static_cast< int >( algorithm->flags() );
          else
            return QVariant();
        }

        default:
          return QVariant();
      }
      break;

    default:
      return QVariant();
  }

  return QVariant();
}

int QgsProcessingToolboxModel::rowCount( const QModelIndex &parent ) const
{
  QgsProcessingToolboxModelNode *n = index2node( parent );
  if ( !n )
    return 0;

  return n->children().count();
}

int QgsProcessingToolboxModel::columnCount( const QModelIndex & ) const
{
  return 1;
}

QModelIndex QgsProcessingToolboxModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( !hasIndex( row, column, parent ) )
    return QModelIndex();

  QgsProcessingToolboxModelNode *n = index2node( parent );
  if ( !n )
    return QModelIndex(); // have no children

  return createIndex( row, column, static_cast<QObject *>( n->children().at( row ) ) );
}

QModelIndex QgsProcessingToolboxModel::parent( const QModelIndex &child ) const
{
  if ( !child.isValid() )
    return QModelIndex();

  if ( QgsProcessingToolboxModelNode *n = index2node( child ) )
  {
    return indexOfParentTreeNode( n->parent() ); // must not be null
  }
  else
  {
    Q_ASSERT( false ); // no other node types!
    return QModelIndex();
  }
}

QgsProcessingProvider *QgsProcessingToolboxModel::providerForIndex( const QModelIndex &index ) const
{
  QgsProcessingToolboxModelNode *n = index2node( index );
  if ( !n || n->nodeType() != QgsProcessingToolboxModelNode::NodeProvider )
    return nullptr;

  return qobject_cast< QgsProcessingToolboxModelProviderNode * >( n )->provider();
}

const QgsProcessingAlgorithm *QgsProcessingToolboxModel::algorithmForIndex( const QModelIndex &index ) const
{
  QgsProcessingToolboxModelNode *n = index2node( index );
  if ( !n || n->nodeType() != QgsProcessingToolboxModelNode::NodeAlgorithm )
    return nullptr;

  return qobject_cast< QgsProcessingToolboxModelAlgorithmNode * >( n )->algorithm();
}

QModelIndex QgsProcessingToolboxModel::indexForProvider( QgsProcessingProvider *provider ) const
{
  if ( !provider )
    return QModelIndex();

  std::function< QModelIndex( const QModelIndex &parent, QgsProcessingProvider *provider ) > findIndex = [&]( const QModelIndex & parent, QgsProcessingProvider * provider )->QModelIndex
  {
    for ( int row = 0; row < rowCount( parent ); ++row )
    {
      QModelIndex current = index( row, 0, parent );
      if ( providerForIndex( current ) == provider )
        return current;

      QModelIndex checkChildren = findIndex( current, provider );
      if ( checkChildren.isValid() )
        return checkChildren;
    }
    return QModelIndex();
  };

  return findIndex( QModelIndex(), provider );
}

QModelIndex QgsProcessingToolboxModel::indexOfParentTreeNode( QgsProcessingToolboxModelNode *parentNode ) const
{
  Q_ASSERT( parentNode );

  QgsProcessingToolboxModelNode *grandParentNode = parentNode->parent();
  if ( !grandParentNode )
    return QModelIndex();  // root node -> invalid index

  int row = grandParentNode->children().indexOf( parentNode );
  Q_ASSERT( row >= 0 );

  return createIndex( row, 0, static_cast<QObject *>( parentNode ) );
}

//
// QgsProcessingToolboxProxyModel
//

QgsProcessingToolboxProxyModel::QgsProcessingToolboxProxyModel( QObject *parent, QgsProcessingRegistry *registry )
  : QSortFilterProxyModel( parent )
  , mModel( new QgsProcessingToolboxModel( parent, registry ) )
{
  setSourceModel( mModel );
  setDynamicSortFilter( true );
  setSortLocaleAware( true );
  setFilterCaseSensitivity( Qt::CaseInsensitive );
  sort( 0 );
}

bool QgsProcessingToolboxProxyModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  QgsProcessingToolboxModelNode::NodeType leftType = static_cast< QgsProcessingToolboxModelNode::NodeType >( sourceModel()->data( left, QgsProcessingToolboxModel::RoleNodeType ).toInt() );
  QgsProcessingToolboxModelNode::NodeType rightType = static_cast< QgsProcessingToolboxModelNode::NodeType >( sourceModel()->data( right, QgsProcessingToolboxModel::RoleNodeType ).toInt() );

  if ( leftType != rightType )
  {
    if ( leftType == QgsProcessingToolboxModelNode::NodeProvider )
      return false;
    else if ( rightType == QgsProcessingToolboxModelNode::NodeProvider )
      return true;
    else if ( leftType == QgsProcessingToolboxModelNode::NodeGroup )
      return false;
    else
      return true;
  }

  // default mode is alphabetical order
  QString leftStr = sourceModel()->data( left ).toString();
  QString rightStr = sourceModel()->data( right ).toString();
  return QString::localeAwareCompare( leftStr, rightStr ) < 0;
}
