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
#include "moc_qgsprocessingtoolboxmodel.cpp"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsprocessingregistry.h"
#include "qgsprocessingrecentalgorithmlog.h"
#include "qgsprocessingfavoritealgorithmmanager.h"
#include <functional>
#include <QPalette>
#include <QMimeData>

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
  for ( QgsProcessingToolboxModelNode *node : std::as_const( mChildren ) )
  {
    if ( node->nodeType() == NodeType::Group )
    {
      QgsProcessingToolboxModelGroupNode *groupNode = qobject_cast<QgsProcessingToolboxModelGroupNode *>( node );
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

QgsProcessingToolboxModel::QgsProcessingToolboxModel( QObject *parent, QgsProcessingRegistry *registry, QgsProcessingRecentAlgorithmLog *recentLog, QgsProcessingFavoriteAlgorithmManager *favoriteManager )
  : QAbstractItemModel( parent )
  , mRegistry( registry ? registry : QgsApplication::processingRegistry() )
  , mRecentLog( recentLog )
  , mFavoriteManager( favoriteManager )
  , mRootNode( std::make_unique<QgsProcessingToolboxModelGroupNode>( QString(), QString() ) )
{
  rebuild();

  if ( mRecentLog )
    connect( mRecentLog, &QgsProcessingRecentAlgorithmLog::changed, this, [=] { repopulateRecentAlgorithms(); } );

  if ( mFavoriteManager )
    connect( mFavoriteManager, &QgsProcessingFavoriteAlgorithmManager::changed, this, [=] { repopulateFavoriteAlgorithms(); } );

  connect( mRegistry, &QgsProcessingRegistry::providerAdded, this, &QgsProcessingToolboxModel::rebuild );
  connect( mRegistry, &QgsProcessingRegistry::providerRemoved, this, &QgsProcessingToolboxModel::providerRemoved );
}

void QgsProcessingToolboxModel::rebuild()
{
  beginResetModel();

  mRootNode->deleteChildren();
  mRecentNode = nullptr;
  mFavoriteNode = nullptr;

  if ( mRecentLog )
  {
    std::unique_ptr<QgsProcessingToolboxModelRecentNode> recentNode = std::make_unique<QgsProcessingToolboxModelRecentNode>();
    // cppcheck-suppress danglingLifetime
    mRecentNode = recentNode.get();
    mRootNode->addChildNode( recentNode.release() );
    repopulateRecentAlgorithms( true );
  }

  if ( mFavoriteManager )
  {
    std::unique_ptr<QgsProcessingToolboxModelFavoriteNode> favoriteNode = std::make_unique<QgsProcessingToolboxModelFavoriteNode>();
    // cppcheck-suppress danglingLifetime
    mFavoriteNode = favoriteNode.get();
    mRootNode->addChildNode( favoriteNode.release() );
    repopulateFavoriteAlgorithms( true );
  }

  if ( mRegistry )
  {
    const QList<QgsProcessingProvider *> providers = mRegistry->providers();
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
  QList<const QgsProcessingAlgorithm *> recentAlgorithms;
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

  for ( const QgsProcessingAlgorithm *algorithm : std::as_const( recentAlgorithms ) )
  {
    std::unique_ptr<QgsProcessingToolboxModelAlgorithmNode> algorithmNode = std::make_unique<QgsProcessingToolboxModelAlgorithmNode>( algorithm );
    mRecentNode->addChildNode( algorithmNode.release() );
  }

  if ( !resetting )
  {
    endInsertRows();
    emit recentAlgorithmAdded();
  }
}

void QgsProcessingToolboxModel::repopulateFavoriteAlgorithms( bool resetting )
{
  if ( !mFavoriteNode || !mFavoriteManager )
    return;

  // favorite node should be under the Recent node if it is present or
  // the first top-level item in the toolbox if Recent node is not present
  int idx = ( mRecentNode && mRecentLog ) ? 1 : 0;

  QModelIndex favoriteIndex = index( idx, 0 );
  const int prevCount = rowCount( favoriteIndex );
  if ( !resetting && prevCount > 0 )
  {
    beginRemoveRows( favoriteIndex, 0, prevCount - 1 );
    mFavoriteNode->deleteChildren();
    endRemoveRows();
  }

  if ( !mRegistry )
  {
    if ( !resetting )
      emit favoriteAlgorithmAdded();
    return;
  }

  const QStringList favoriteAlgIds = mFavoriteManager->favoriteAlgorithmIds();
  QList<const QgsProcessingAlgorithm *> favoriteAlgorithms;
  favoriteAlgorithms.reserve( favoriteAlgIds.count() );
  for ( const QString &id : favoriteAlgIds )
  {
    const QgsProcessingAlgorithm *algorithm = mRegistry->algorithmById( id );
    if ( algorithm )
      favoriteAlgorithms << algorithm;
  }

  if ( favoriteAlgorithms.empty() )
  {
    if ( !resetting )
      emit favoriteAlgorithmAdded();
    return;
  }

  if ( !resetting )
  {
    beginInsertRows( favoriteIndex, 0, favoriteAlgorithms.count() - 1 );
  }

  for ( const QgsProcessingAlgorithm *algorithm : std::as_const( favoriteAlgorithms ) )
  {
    std::unique_ptr<QgsProcessingToolboxModelAlgorithmNode> algorithmNode = std::make_unique<QgsProcessingToolboxModelAlgorithmNode>( algorithm );
    mFavoriteNode->addChildNode( algorithmNode.release() );
  }

  if ( !resetting )
  {
    endInsertRows();
    emit favoriteAlgorithmAdded();
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
    std::unique_ptr<QgsProcessingToolboxModelProviderNode> node = std::make_unique<QgsProcessingToolboxModelProviderNode>( provider );
    parentNode = node.get();
    mRootNode->addChildNode( node.release() );
  }
  else
  {
    parentNode = mRootNode.get();
  }

  const QList<const QgsProcessingAlgorithm *> algorithms = provider->algorithms();
  for ( const QgsProcessingAlgorithm *algorithm : algorithms )
  {
    std::unique_ptr<QgsProcessingToolboxModelAlgorithmNode> algorithmNode = std::make_unique<QgsProcessingToolboxModelAlgorithmNode>( algorithm );

    const QString groupId = algorithm->groupId();
    if ( !groupId.isEmpty() )
    {
      // cppcheck-suppress invalidLifetime
      QgsProcessingToolboxModelGroupNode *groupNode = parentNode->getChildGroupNode( groupId );
      if ( !groupNode )
      {
        groupNode = new QgsProcessingToolboxModelGroupNode( algorithm->groupId(), algorithm->group() );
        // cppcheck-suppress invalidLifetime
        parentNode->addChildNode( groupNode );
      }
      groupNode->addChildNode( algorithmNode.release() );
    }
    else
    {
      // "top level" algorithm - no group
      // cppcheck-suppress invalidLifetime
      parentNode->addChildNode( algorithmNode.release() );
    }
  }
}

bool QgsProcessingToolboxModel::isTopLevelProvider( const QString &providerId )
{
  return providerId == QLatin1String( "qgis" ) || providerId == QLatin1String( "native" ) || providerId == QLatin1String( "3d" ) || providerId == QLatin1String( "pdal" );
}

QString QgsProcessingToolboxModel::toolTipForAlgorithm( const QgsProcessingAlgorithm *algorithm )
{
  return QStringLiteral( "<p><b>%1</b></p>%2<p>%3</p>%4" ).arg( algorithm->displayName(), !algorithm->shortDescription().isEmpty() ? QStringLiteral( "<p>%1</p>" ).arg( algorithm->shortDescription() ) : QString(), QObject::tr( "Algorithm ID: ‘%1’" ).arg( QStringLiteral( "<i>%1</i>" ).arg( algorithm->id() ) ), ( algorithm->flags() & Qgis::ProcessingAlgorithmFlag::KnownIssues ) ? QStringLiteral( "<b style=\"color:red\">%1</b>" ).arg( QObject::tr( "Warning: Algorithm has known issues" ) ) : QString() );
}

Qt::ItemFlags QgsProcessingToolboxModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return Qt::ItemFlags();

  return QAbstractItemModel::flags( index );
}

QVariant QgsProcessingToolboxModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  if ( role == static_cast<int>( CustomRole::NodeType ) )
  {
    if ( QgsProcessingToolboxModelNode *node = index2node( index ) )
      return static_cast<int>( node->nodeType() );
    else
      return QVariant();
  }

  bool isRecentNode = false;
  if ( QgsProcessingToolboxModelNode *node = index2node( index ) )
    isRecentNode = node->nodeType() == QgsProcessingToolboxModelNode::NodeType::Recent;

  bool isFavoriteNode = false;
  if ( QgsProcessingToolboxModelNode *node = index2node( index ) )
    isFavoriteNode = node->nodeType() == QgsProcessingToolboxModelNode::NodeType::Favorite;

  QgsProcessingProvider *provider = providerForIndex( index );
  QgsProcessingToolboxModelGroupNode *groupNode = qobject_cast<QgsProcessingToolboxModelGroupNode *>( index2node( index ) );
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
          else if ( isFavoriteNode )
            return tr( "Favorites" );
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

    case Qt::ForegroundRole:
    {
      if ( algorithm && algorithm->flags() & Qgis::ProcessingAlgorithmFlag::KnownIssues )
        return QBrush( QColor( Qt::red ) );
      else
        return QVariant();
    }

    case Qt::DecorationRole:
    {
      switch ( index.column() )
      {
        case 0:
        {
          if ( provider )
            return provider->icon();
          else if ( algorithm )
          {
            if ( algorithm->flags() & Qgis::ProcessingAlgorithmFlag::KnownIssues )
              return QgsApplication::getThemeIcon( QStringLiteral( "mIconWarning.svg" ) );
            return algorithm->icon();
          }
          else if ( isRecentNode )
            return QgsApplication::getThemeIcon( QStringLiteral( "/mIconHistory.svg" ) );
          else if ( isFavoriteNode )
            return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFavorites.svg" ) );
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
    }

    case static_cast<int>( CustomRole::AlgorithmFlags ):
      switch ( index.column() )
      {
        case 0:
        {
          if ( algorithm )
            return static_cast<int>( algorithm->flags() );
          else
            return QVariant();
        }

        default:
          return QVariant();
      }
      break;

    case static_cast<int>( CustomRole::ProviderFlags ):
      switch ( index.column() )
      {
        case 0:
        {
          if ( provider )
            return static_cast<int>( provider->flags() );
          else if ( algorithm && algorithm->provider() )
            return static_cast<int>( algorithm->provider()->flags() );
          else if ( index.parent().data( static_cast<int>( CustomRole::ProviderFlags ) ).isValid() ) // group node
            return static_cast<int>( index.parent().data( static_cast<int>( CustomRole::ProviderFlags ) ).toInt() );
          else
            return QVariant();
        }

        default:
          return QVariant();
      }
      break;

    case static_cast<int>( CustomRole::AlgorithmId ):
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

    case static_cast<int>( CustomRole::AlgorithmName ):
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

    case static_cast<int>( CustomRole::AlgorithmTags ):
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

    case static_cast<int>( CustomRole::AlgorithmShortDescription ):
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
#ifndef _MSC_VER // avoid warning
  return QVariant();
#endif
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

    std::unique_ptr<QMimeData> mimeData = std::make_unique<QMimeData>();
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
  if ( !n || n->nodeType() != QgsProcessingToolboxModelNode::NodeType::Provider )
    return nullptr;

  return qobject_cast<QgsProcessingToolboxModelProviderNode *>( n )->provider();
}

QString QgsProcessingToolboxModel::providerIdForIndex( const QModelIndex &index ) const
{
  QgsProcessingToolboxModelNode *n = index2node( index );
  if ( !n || n->nodeType() != QgsProcessingToolboxModelNode::NodeType::Provider )
    return nullptr;

  return qobject_cast<QgsProcessingToolboxModelProviderNode *>( n )->providerId();
}

const QgsProcessingAlgorithm *QgsProcessingToolboxModel::algorithmForIndex( const QModelIndex &index ) const
{
  QgsProcessingToolboxModelNode *n = index2node( index );
  if ( !n || n->nodeType() != QgsProcessingToolboxModelNode::NodeType::Algorithm )
    return nullptr;

  return qobject_cast<QgsProcessingToolboxModelAlgorithmNode *>( n )->algorithm();
}

bool QgsProcessingToolboxModel::isAlgorithm( const QModelIndex &index ) const
{
  QgsProcessingToolboxModelNode *n = index2node( index );
  return ( n && n->nodeType() == QgsProcessingToolboxModelNode::NodeType::Algorithm );
}

QModelIndex QgsProcessingToolboxModel::indexForProvider( const QString &providerId ) const
{
  std::function<QModelIndex( const QModelIndex &parent, const QString &providerId )> findIndex = [&]( const QModelIndex &parent, const QString &providerId ) -> QModelIndex {
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
    return QModelIndex(); // root node -> invalid index

  int row = grandParentNode->children().indexOf( parentNode );
  Q_ASSERT( row >= 0 );

  return createIndex( row, 0, static_cast<QObject *>( parentNode ) );
}

//
// QgsProcessingToolboxProxyModel
//

QgsProcessingToolboxProxyModel::QgsProcessingToolboxProxyModel( QObject *parent, QgsProcessingRegistry *registry, QgsProcessingRecentAlgorithmLog *recentLog, QgsProcessingFavoriteAlgorithmManager *favoriteManager )
  : QSortFilterProxyModel( parent )
  , mModel( new QgsProcessingToolboxModel( this, registry, recentLog, favoriteManager ) )
{
  setSourceModel( mModel );
  setDynamicSortFilter( true );
  setSortLocaleAware( true );
  setFilterCaseSensitivity( Qt::CaseInsensitive );
  sort( 0 );

  connect( mModel, &QgsProcessingToolboxModel::recentAlgorithmAdded, this, [=] { invalidateFilter(); } );
  connect( mModel, &QgsProcessingToolboxModel::favoriteAlgorithmAdded, this, [=] { invalidateFilter(); } );
}

QgsProcessingToolboxModel *QgsProcessingToolboxProxyModel::toolboxModel()
{
  return mModel;
}

const QgsProcessingToolboxModel *QgsProcessingToolboxProxyModel::toolboxModel() const
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
    const bool hasKnownIssues = sourceModel()->data( sourceIndex, static_cast<int>( QgsProcessingToolboxModel::CustomRole::AlgorithmFlags ) ).toInt() & static_cast<int>( Qgis::ProcessingAlgorithmFlag::KnownIssues );
    if ( hasKnownIssues && !( mFilters & Filter::ShowKnownIssues ) )
      return false;

    if ( !mFilterString.trimmed().isEmpty() )
    {
      const QString algId = sourceModel()->data( sourceIndex, static_cast<int>( QgsProcessingToolboxModel::CustomRole::AlgorithmId ) ).toString();
      const QString algName = sourceModel()->data( sourceIndex, static_cast<int>( QgsProcessingToolboxModel::CustomRole::AlgorithmName ) ).toString();
      const QStringList algTags = sourceModel()->data( sourceIndex, static_cast<int>( QgsProcessingToolboxModel::CustomRole::AlgorithmTags ) ).toStringList();
      const QString shortDesc = sourceModel()->data( sourceIndex, static_cast<int>( QgsProcessingToolboxModel::CustomRole::AlgorithmShortDescription ) ).toString();

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
        for ( const QString &partToSearch : std::as_const( partsToSearch ) )
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

    if ( mFilters & Filter::InPlace )
    {
      const bool supportsInPlace = sourceModel()->data( sourceIndex, static_cast<int>( QgsProcessingToolboxModel::CustomRole::AlgorithmFlags ) ).toInt() & static_cast<int>( Qgis::ProcessingAlgorithmFlag::SupportsInPlaceEdits );
      if ( !supportsInPlace )
        return false;

      const QgsProcessingAlgorithm *alg = mModel->algorithmForIndex( sourceIndex );
      if ( !( mInPlaceLayer && alg && alg->supportInPlaceEdit( mInPlaceLayer ) ) )
      {
        return false;
      }
    }
    if ( mFilters & Filter::Modeler )
    {
      bool isHiddenFromModeler = sourceModel()->data( sourceIndex, static_cast<int>( QgsProcessingToolboxModel::CustomRole::AlgorithmFlags ) ).toInt() & static_cast<int>( Qgis::ProcessingAlgorithmFlag::HideFromModeler );
      return !isHiddenFromModeler;
    }
    if ( mFilters & Filter::Toolbox )
    {
      bool isHiddenFromToolbox = sourceModel()->data( sourceIndex, static_cast<int>( QgsProcessingToolboxModel::CustomRole::AlgorithmFlags ) ).toInt() & static_cast<int>( Qgis::ProcessingAlgorithmFlag::HideFromToolbox );
      return !isHiddenFromToolbox;
    }
    return true;
  }

  bool hasChildren = false;
  // groups/providers are shown only if they have visible children
  int count = sourceModel()->rowCount( sourceIndex );
  for ( int i = 0; i < count; ++i )
  {
    if ( filterAcceptsRow( i, sourceIndex ) )
    {
      hasChildren = true;
      break;
    }
  }

  return hasChildren;
}

bool QgsProcessingToolboxProxyModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  QgsProcessingToolboxModelNode::NodeType leftType = static_cast<QgsProcessingToolboxModelNode::NodeType>( sourceModel()->data( left, static_cast<int>( QgsProcessingToolboxModel::CustomRole::NodeType ) ).toInt() );
  QgsProcessingToolboxModelNode::NodeType rightType = static_cast<QgsProcessingToolboxModelNode::NodeType>( sourceModel()->data( right, static_cast<int>( QgsProcessingToolboxModel::CustomRole::NodeType ) ).toInt() );

  if ( leftType == QgsProcessingToolboxModelNode::NodeType::Recent )
    return true;
  else if ( rightType == QgsProcessingToolboxModelNode::NodeType::Recent )
    return false;
  else if ( leftType == QgsProcessingToolboxModelNode::NodeType::Favorite )
    return true;
  else if ( rightType == QgsProcessingToolboxModelNode::NodeType::Favorite )
    return false;
  else if ( leftType != rightType )
  {
    if ( leftType == QgsProcessingToolboxModelNode::NodeType::Provider )
      return false;
    else if ( rightType == QgsProcessingToolboxModelNode::NodeType::Provider )
      return true;
    else if ( leftType == QgsProcessingToolboxModelNode::NodeType::Group )
      return false;
    else
      return true;
  }

  // if node represents a recent algorithm, it's not sorted at all
  bool isRecentNode = false;
  QModelIndex parent = left.parent();
  while ( parent.isValid() )
  {
    if ( mModel->data( parent, static_cast<int>( QgsProcessingToolboxModel::CustomRole::NodeType ) ).toInt() == static_cast<int>( QgsProcessingToolboxModelNode::NodeType::Recent ) )
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

QVariant QgsProcessingToolboxProxyModel::data( const QModelIndex &index, int role ) const
{
  if ( role == Qt::ForegroundRole && !mFilterString.isEmpty() )
  {
    QModelIndex sourceIndex = mapToSource( index );
    const QVariant flags = sourceModel()->data( sourceIndex, static_cast<int>( QgsProcessingToolboxModel::CustomRole::ProviderFlags ) );
    if ( flags.isValid() && flags.toInt() & static_cast<int>( Qgis::ProcessingProviderFlag::DeemphasiseSearchResults ) )
    {
      QBrush brush( qApp->palette().color( QPalette::Text ), Qt::SolidPattern );
      QColor fadedTextColor = brush.color();
      fadedTextColor.setAlpha( 100 );
      brush.setColor( fadedTextColor );
      return brush;
    }
  }
  return QSortFilterProxyModel::data( index, role );
}
