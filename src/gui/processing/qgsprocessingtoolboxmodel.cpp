/***************************************************************************
    qgsprocessingtoolboxmodel.cpp
    -------------------------------
    begin                : May 2018
    copyright            : (C) 2018 by Nyall Dawson
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
#include "qgsvectorlayer.h"
#include "qgsprocessingregistry.h"
#include "qgsprocessingrecentalgorithmlog.h"
#include <functional>

#ifdef ENABLE_MODELTEST
#include "modeltest.h"
#endif

///@cond PRIVATE

//
// QgsProcessingToolboxModelNode
//

QgsProcessingToolboxModelNode::~QgsProcessingToolboxModelNode()
{
  deleteChildren();
}

QgsProcessingToolboxModelNode *QgsProcessingToolboxModelNode::takeChild( QgsProcessingToolboxModelNode *node )
{
  return mChildren.takeAt( mChildren.indexOf( node ) );
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
  : mProviderId( provider->id() )
  , mProvider( provider )
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

///@endcond

//
// QgsProcessingToolboxModel
//

QgsProcessingToolboxModel::QgsProcessingToolboxModel( QObject *parent, QgsProcessingRegistry *registry, QgsProcessingRecentAlgorithmLog *recentLog )
  : QAbstractItemModel( parent )
  , mRegistry( registry ? registry : QgsApplication::processingRegistry() )
  , mRecentLog( recentLog )
  , mRootNode( qgis::make_unique< QgsProcessingToolboxModelGroupNode >( QString(), QString() ) )
{
  rebuild();

  if ( mRecentLog )
    connect( mRecentLog, &QgsProcessingRecentAlgorithmLog::changed, this, [ = ] { repopulateRecentAlgorithms(); } );

  connect( mRegistry, &QgsProcessingRegistry::providerAdded, this, &QgsProcessingToolboxModel::rebuild );
  connect( mRegistry, &QgsProcessingRegistry::providerRemoved, this, &QgsProcessingToolboxModel::providerRemoved );
}

void QgsProcessingToolboxModel::rebuild()
{
  beginResetModel();

  mRootNode->deleteChildren();
  mRecentNode = nullptr;

  if ( mRecentLog )
  {
    std::unique_ptr< QgsProcessingToolboxModelRecentNode > recentNode = qgis::make_unique< QgsProcessingToolboxModelRecentNode >();
    mRecentNode = recentNode.get();
    mRootNode->addChildNode( recentNode.release() );
    repopulateRecentAlgorithms( true );
  }

  if ( mRegistry )
  {
    const QList< QgsProcessingProvider * > providers = mRegistry->providers();
    for ( QgsProcessingProvider *provider : providers )
    {
      addProvider( provider );
    }
  }
  endResetModel();
}

void QgsProcessingToolboxModel::repopulateRecentAlgorithms( bool resetting )
{
  if ( !mRecentNode || !mRecentLog )
    return;

  QModelIndex recentIndex = index( 0, 0 );
  const int prevCount = rowCount( recentIndex );
  if ( !resetting && prevCount > 0 )
  {
    beginRemoveRows( recentIndex, 0, prevCount - 1 );
    mRecentNode->deleteChildren();
    endRemoveRows();
  }

  if ( !mRegistry )
  {
    if ( !resetting )
      emit recentAlgorithmAdded();
    return;
  }

  const QStringList recentAlgIds = mRecentLog->recentAlgorithmIds();
  QList< const QgsProcessingAlgorithm * > recentAlgorithms;
  recentAlgorithms.reserve( recentAlgIds.count() );
  for ( const QString &id : recentAlgIds )
  {
    const QgsProcessingAlgorithm *algorithm = mRegistry->algorithmById( id );
    if ( algorithm )
      recentAlgorithms << algorithm;
  }

  if ( recentAlgorithms.empty() )
  {
    if ( !resetting )
      emit recentAlgorithmAdded();
    return;
  }

  if ( !resetting )
  {
    beginInsertRows( recentIndex, 0, recentAlgorithms.count() - 1 );
  }

  for ( const QgsProcessingAlgorithm *algorithm : qgis::as_const( recentAlgorithms ) )
  {
    std::unique_ptr< QgsProcessingToolboxModelAlgorithmNode > algorithmNode = qgis::make_unique< QgsProcessingToolboxModelAlgorithmNode >( algorithm );
    mRecentNode->addChildNode( algorithmNode.release() );
  }

  if ( !resetting )
  {
    endInsertRows();
    emit recentAlgorithmAdded();
  }
}

void QgsProcessingToolboxModel::providerAdded( const QString &id )
{
  if ( !mRegistry )
    return;

  QgsProcessingProvider *provider = mRegistry->providerById( id );
  if ( !provider )
    return;

  if ( !isTopLevelProvider( id ) )
  {
    int previousRowCount = rowCount();
    beginInsertRows( QModelIndex(), previousRowCount, previousRowCount );
    addProvider( provider );
    endInsertRows();
  }
  else
  {
    // native providers use top level groups - that's too hard for us to
    // work out exactly what's going to change, so just reset the model
    beginResetModel();
    addProvider( provider );
    endResetModel();
  }
}

void QgsProcessingToolboxModel::providerRemoved( const QString & )
{
  // native providers use top level groups - so we can't
  // work out what to remove. Just rebuild the whole model instead.
  rebuild();
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
  if ( !provider )
    return;

  connect( provider, &QgsProcessingProvider::algorithmsLoaded, this, &QgsProcessingToolboxModel::rebuild, Qt::UniqueConnection );

  QgsProcessingToolboxModelNode *parentNode = nullptr;
  if ( !isTopLevelProvider( provider->id() ) )
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

bool QgsProcessingToolboxModel::isTopLevelProvider( const QString &providerId )
{
  return providerId == QLatin1String( "qgis" ) ||
         providerId == QLatin1String( "native" ) ||
         providerId == QLatin1String( "3d" );
}

QString QgsProcessingToolboxModel::toolTipForAlgorithm( const QgsProcessingAlgorithm *algorithm )
{
  return QStringLiteral( "<p><b>%1</b></p>%2<p>%3</p>" ).arg(
           algorithm->displayName(),
           !algorithm->shortDescription().isEmpty() ? QStringLiteral( "<p>%1</p>" ).arg( algorithm->shortDescription() ) : QString(),
           QObject::tr( "Algorithm ID: ‘%1’" ).arg( QStringLiteral( "<i>%1</i>" ).arg( algorithm->id() ) )
         );
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

  bool isRecentNode = false;
  if ( QgsProcessingToolboxModelNode *node = index2node( index ) )
    isRecentNode = node->nodeType() == QgsProcessingToolboxModelNode::NodeRecent;

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
          else if ( isRecentNode )
            return tr( "Recently used" );
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
        return toolTipForAlgorithm( algorithm );
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
          else if ( isRecentNode )
            return QgsApplication::getThemeIcon( QStringLiteral( "/mIconHistory.svg" ) );
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

    case RoleAlgorithmId:
      switch ( index.column() )
      {
        case 0:
        {
          if ( algorithm )
            return algorithm->id();
          else
            return QVariant();
        }

        default:
          return QVariant();
      }
      break;

    case RoleAlgorithmName:
      switch ( index.column() )
      {
        case 0:
        {
          if ( algorithm )
            return algorithm->name();
          else
            return QVariant();
        }

        default:
          return QVariant();
      }
      break;

    case RoleAlgorithmTags:
      switch ( index.column() )
      {
        case 0:
        {
          if ( algorithm )
            return algorithm->tags();
          else
            return QVariant();
        }

        default:
          return QVariant();
      }
      break;

    case RoleAlgorithmShortDescription:
      switch ( index.column() )
      {
        case 0:
        {
          if ( algorithm )
            return algorithm->shortDescription();
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

QMimeData *QgsProcessingToolboxModel::mimeData( const QModelIndexList &indexes ) const
{
  if ( !indexes.isEmpty() && isAlgorithm( indexes.at( 0 ) ) )
  {
    QByteArray encodedData;
    QDataStream stream( &encodedData, QIODevice::WriteOnly | QIODevice::Truncate );

    std::unique_ptr< QMimeData > mimeData = qgis::make_unique< QMimeData >();
    const QgsProcessingAlgorithm *algorithm = algorithmForIndex( indexes.at( 0 ) );
    if ( algorithm )
    {
      stream << algorithm->id();
    }
    mimeData->setData( QStringLiteral( "application/x-vnd.qgis.qgis.algorithmid" ), encodedData );
    return mimeData.release();
  }
  return nullptr;
}

QgsProcessingProvider *QgsProcessingToolboxModel::providerForIndex( const QModelIndex &index ) const
{
  QgsProcessingToolboxModelNode *n = index2node( index );
  if ( !n || n->nodeType() != QgsProcessingToolboxModelNode::NodeProvider )
    return nullptr;

  return qobject_cast< QgsProcessingToolboxModelProviderNode * >( n )->provider();
}

QString QgsProcessingToolboxModel::providerIdForIndex( const QModelIndex &index ) const
{
  QgsProcessingToolboxModelNode *n = index2node( index );
  if ( !n || n->nodeType() != QgsProcessingToolboxModelNode::NodeProvider )
    return nullptr;

  return qobject_cast< QgsProcessingToolboxModelProviderNode * >( n )->providerId();
}

const QgsProcessingAlgorithm *QgsProcessingToolboxModel::algorithmForIndex( const QModelIndex &index ) const
{
  QgsProcessingToolboxModelNode *n = index2node( index );
  if ( !n || n->nodeType() != QgsProcessingToolboxModelNode::NodeAlgorithm )
    return nullptr;

  return qobject_cast< QgsProcessingToolboxModelAlgorithmNode * >( n )->algorithm();
}

bool QgsProcessingToolboxModel::isAlgorithm( const QModelIndex &index ) const
{
  QgsProcessingToolboxModelNode *n = index2node( index );
  return ( n && n->nodeType() == QgsProcessingToolboxModelNode::NodeAlgorithm );
}

QModelIndex QgsProcessingToolboxModel::indexForProvider( const QString &providerId ) const
{
  std::function< QModelIndex( const QModelIndex &parent, const QString &providerId ) > findIndex = [&]( const QModelIndex & parent, const QString & providerId )->QModelIndex
  {
    for ( int row = 0; row < rowCount( parent ); ++row )
    {
      QModelIndex current = index( row, 0, parent );
      const QString currentProviderId = providerIdForIndex( current );
      if ( !currentProviderId.isEmpty() && currentProviderId == providerId )
        return current;

      QModelIndex checkChildren = findIndex( current, providerId );
      if ( checkChildren.isValid() )
        return checkChildren;
    }
    return QModelIndex();
  };

  return findIndex( QModelIndex(), providerId );
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

QgsProcessingToolboxProxyModel::QgsProcessingToolboxProxyModel( QObject *parent, QgsProcessingRegistry *registry,
    QgsProcessingRecentAlgorithmLog *recentLog )
  : QSortFilterProxyModel( parent )
  , mModel( new QgsProcessingToolboxModel( this, registry, recentLog ) )
{
  setSourceModel( mModel );
  setDynamicSortFilter( true );
  setSortLocaleAware( true );
  setFilterCaseSensitivity( Qt::CaseInsensitive );
  sort( 0 );

  connect( mModel, &QgsProcessingToolboxModel::recentAlgorithmAdded, this, [ = ] { invalidateFilter(); } );
}

QgsProcessingToolboxModel *QgsProcessingToolboxProxyModel::toolboxModel()
{
  return mModel;
}

void QgsProcessingToolboxProxyModel::setFilters( QgsProcessingToolboxProxyModel::Filters filters )
{
  mFilters = filters;
  invalidateFilter();
}

void QgsProcessingToolboxProxyModel::setInPlaceLayer( QgsVectorLayer *layer )
{
  mInPlaceLayer = layer;
  invalidateFilter();
}


void QgsProcessingToolboxProxyModel::setFilterString( const QString &filter )
{
  mFilterString = filter;
  invalidateFilter();
}

bool QgsProcessingToolboxProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
  QModelIndex sourceIndex = mModel->index( sourceRow, 0, sourceParent );
  if ( mModel->isAlgorithm( sourceIndex ) )
  {
    if ( !mFilterString.trimmed().isEmpty() )
    {
      const QString algId = sourceModel()->data( sourceIndex, QgsProcessingToolboxModel::RoleAlgorithmId ).toString();
      const QString algName = sourceModel()->data( sourceIndex, QgsProcessingToolboxModel::RoleAlgorithmName ).toString();
      const QStringList algTags = sourceModel()->data( sourceIndex, QgsProcessingToolboxModel::RoleAlgorithmTags ).toStringList();
      const QString shortDesc = sourceModel()->data( sourceIndex, QgsProcessingToolboxModel::RoleAlgorithmShortDescription ).toString();

      QStringList parentText;
      QModelIndex parent = sourceIndex.parent();
      while ( parent.isValid() )
      {
        const QStringList parentParts = sourceModel()->data( parent, Qt::DisplayRole ).toString().split( ' ' );
        if ( !parentParts.empty() )
          parentText.append( parentParts );
        parent = parent.parent();
      }

      const QStringList partsToMatch = mFilterString.trimmed().split( ' ' );

      QStringList partsToSearch = sourceModel()->data( sourceIndex, Qt::DisplayRole ).toString().split( ' ' );
      partsToSearch << algId << algName;
      partsToSearch.append( algTags );
      if ( !shortDesc.isEmpty() )
        partsToSearch.append( shortDesc.split( ' ' ) );
      partsToSearch.append( parentText );

      for ( const QString &part : partsToMatch )
      {
        bool found = false;
        for ( const QString &partToSearch : qgis::as_const( partsToSearch ) )
        {
          if ( partToSearch.contains( part, Qt::CaseInsensitive ) )
          {
            found = true;
            break;
          }
        }
        if ( !found )
          return false; // couldn't find a match for this word, so hide algorithm
      }
    }

    if ( mFilters & FilterInPlace )
    {
      const bool supportsInPlace = sourceModel()->data( sourceIndex, QgsProcessingToolboxModel::RoleAlgorithmFlags ).toInt() & QgsProcessingAlgorithm::FlagSupportsInPlaceEdits;
      if ( !supportsInPlace )
        return false;

      const QgsProcessingAlgorithm *alg = dynamic_cast< const QgsProcessingAlgorithm * >( mModel->algorithmForIndex( sourceIndex ) );
      if ( !( mInPlaceLayer && alg && alg->supportInPlaceEdit( mInPlaceLayer ) ) )
      {
        return false;
      }
    }
    if ( mFilters & FilterModeler )
    {
      bool isHiddenFromModeler = sourceModel()->data( sourceIndex, QgsProcessingToolboxModel::RoleAlgorithmFlags ).toInt() & QgsProcessingAlgorithm::FlagHideFromModeler;
      return !isHiddenFromModeler;
    }
    if ( mFilters & FilterToolbox )
    {
      bool isHiddenFromToolbox = sourceModel()->data( sourceIndex, QgsProcessingToolboxModel::RoleAlgorithmFlags ).toInt() & QgsProcessingAlgorithm::FlagHideFromToolbox;
      return !isHiddenFromToolbox;
    }
    return true;
  }

  bool hasChildren = false;
  // groups are shown only if they have visible children
  // but providers are shown if they have visible children, OR the filter string is empty
  int count = sourceModel()->rowCount( sourceIndex );
  for ( int i = 0; i < count; ++i )
  {
    if ( filterAcceptsRow( i, sourceIndex ) )
    {
      hasChildren = true;
      break;
    }
  }

  if ( QgsProcessingProvider *provider = mModel->providerForIndex( sourceIndex ) )
  {
    return ( hasChildren || mFilterString.trimmed().isEmpty() ) && provider->isActive();
  }
  else
  {
    // group
    return hasChildren; // || isRecentNode;
  }
}

bool QgsProcessingToolboxProxyModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  QgsProcessingToolboxModelNode::NodeType leftType = static_cast< QgsProcessingToolboxModelNode::NodeType >( sourceModel()->data( left, QgsProcessingToolboxModel::RoleNodeType ).toInt() );
  QgsProcessingToolboxModelNode::NodeType rightType = static_cast< QgsProcessingToolboxModelNode::NodeType >( sourceModel()->data( right, QgsProcessingToolboxModel::RoleNodeType ).toInt() );

  if ( leftType == QgsProcessingToolboxModelNode::NodeRecent )
    return true;
  else if ( rightType == QgsProcessingToolboxModelNode::NodeRecent )
    return false;
  else if ( leftType != rightType )
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

  // if node represents a recent algorithm, it's not sorted at all
  bool isRecentNode = false;
  QModelIndex parent = left.parent();
  while ( parent.isValid() )
  {
    if ( mModel->data( parent, QgsProcessingToolboxModel::RoleNodeType ).toInt() == QgsProcessingToolboxModelNode::NodeRecent )
    {
      isRecentNode = true;
      break;
    }
    parent = parent.parent();
  }
  if ( isRecentNode )
  {
    return left.row() < right.row();
  }


  // default mode is alphabetical order
  QString leftStr = sourceModel()->data( left ).toString();
  QString rightStr = sourceModel()->data( right ).toString();
  return QString::localeAwareCompare( leftStr, rightStr ) < 0;
}
