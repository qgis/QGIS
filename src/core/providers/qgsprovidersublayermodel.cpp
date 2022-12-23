/***************************************************************************
                             qgsprovidersublayermodel.cpp
                             ----------------------
    begin                : June 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprovidersublayermodel.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsiconutils.h"
#include "qgsapplication.h"
#include <QLocale>

//
// QgsProviderSublayerModelNode
//

///@cond PRIVATE
QgsProviderSublayerModelNode::~QgsProviderSublayerModelNode() = default;

void QgsProviderSublayerModelGroup::populateFromSublayers( const QList<QgsProviderSublayerDetails> &sublayers )
{
  for ( const QgsProviderSublayerDetails &sublayer : sublayers )
  {
    if ( !sublayer.path().isEmpty() )
    {
      QStringList currentPath;
      QStringList remainingPaths = sublayer.path();
      QgsProviderSublayerModelGroup *groupNode = this;

      while ( !remainingPaths.empty() )
      {
        currentPath << remainingPaths.takeAt( 0 );

        QgsProviderSublayerModelGroup *nextChild = groupNode->findGroup( currentPath.constLast() );
        if ( !nextChild )
        {
          std::unique_ptr< QgsProviderSublayerModelGroup > newNode = std::make_unique< QgsProviderSublayerModelGroup >( currentPath.constLast() );
          groupNode = qgis::down_cast< QgsProviderSublayerModelGroup * >( groupNode->addChild( std::move( newNode ) ) );
        }
        else
        {
          groupNode = nextChild;
        }
      }

      groupNode->addChild( std::make_unique< QgsProviderSublayerModelSublayerNode >( sublayer ) );
    }
    else
    {
      addChild( std::make_unique< QgsProviderSublayerModelSublayerNode >( sublayer ) );
    }
  }
}

QgsProviderSublayerModelGroup::QgsProviderSublayerModelGroup( const QString &title )
  : mGroupTitle( title )
{

}

QgsProviderSublayerModelNode *QgsProviderSublayerModelGroup::addChild( std::unique_ptr<QgsProviderSublayerModelNode> child )
{
  if ( !child )
    return nullptr;

  Q_ASSERT( !child->mParent );
  child->mParent = this;

  QgsProviderSublayerModelNode *res = child.get();
  mChildren.emplace_back( std::move( child ) );
  return res;
}

int QgsProviderSublayerModelGroup::indexOf( QgsProviderSublayerModelNode *child ) const
{
  Q_ASSERT( child->mParent == this );
  auto it = std::find_if( mChildren.begin(), mChildren.end(), [&]( const std::unique_ptr<QgsProviderSublayerModelNode> &p )
  {
    return p.get() == child;
  } );
  if ( it != mChildren.end() )
    return std::distance( mChildren.begin(), it );
  return -1;
}

QgsProviderSublayerModelNode *QgsProviderSublayerModelGroup::childAt( int index )
{
  if ( static_cast< std::size_t >( index ) < mChildren.size() )
    return mChildren[ index ].get();

  return nullptr;
}

void QgsProviderSublayerModelGroup::removeChildAt( int index )
{
  mChildren.erase( mChildren.begin() + index );
}

QgsProviderSublayerModelGroup *QgsProviderSublayerModelGroup::findGroup( const QString &name ) const
{
  for ( const auto &node : mChildren )
  {
    if ( QgsProviderSublayerModelGroup *group = dynamic_cast< QgsProviderSublayerModelGroup * >( node.get() ) )
    {
      if ( group->name() == name )
        return group;
    }
  }
  return nullptr;
}

QgsProviderSublayerModelGroup *QgsProviderSublayerModelGroup::findGroupForPath( const QStringList &path ) const
{
  const QgsProviderSublayerModelGroup *currentGroup = this;
  for ( const QString &part : path )
  {
    currentGroup = currentGroup->findGroup( part );
  }
  return const_cast< QgsProviderSublayerModelGroup * >( currentGroup );
}

QgsProviderSublayerModelSublayerNode *QgsProviderSublayerModelGroup::findSublayer( const QgsProviderSublayerDetails &sublayer )
{
  for ( const auto &node : mChildren )
  {
    if ( QgsProviderSublayerModelGroup *group = dynamic_cast< QgsProviderSublayerModelGroup * >( node.get() ) )
    {
      if ( QgsProviderSublayerModelSublayerNode *node = group->findSublayer( sublayer ) )
        return node;
    }
    else if ( QgsProviderSublayerModelSublayerNode *sublayerNode = dynamic_cast< QgsProviderSublayerModelSublayerNode * >( node.get() ) )
    {
      if ( sublayerNode->sublayer() == sublayer )
        return sublayerNode;
    }
  }
  return nullptr;
}

QVariant QgsProviderSublayerModelGroup::data( int role, int column ) const
{
  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
    case Qt::EditRole:
    {
      switch ( static_cast< QgsProviderSublayerModel::Column >( column ) )
      {
        case QgsProviderSublayerModel::Column::Name:
          return mGroupTitle;

        case QgsProviderSublayerModel::Column::Description:
          return QVariant();
      }
      return QVariant();
    }

    case Qt::DecorationRole:
    {
      if ( column == 0 )
        return QgsApplication::getThemeIcon( QStringLiteral( "/mIconDbSchema.svg" ) );
      else
        return QVariant();
    }

    default:
      return QVariant();
  }
}

QgsProviderSublayerModelSublayerNode::QgsProviderSublayerModelSublayerNode( const QgsProviderSublayerDetails &sublayer )
  : mSublayer( sublayer )
{
}

QVariant QgsProviderSublayerModelSublayerNode::data( int role, int column ) const
{
  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
    case Qt::EditRole:
    {
      switch ( static_cast< QgsProviderSublayerModel::Column >( column ) )
      {
        case QgsProviderSublayerModel::Column::Name:
          return mSublayer.name();
        case QgsProviderSublayerModel::Column::Description:
        {
          switch ( mSublayer.type() )
          {
            case QgsMapLayerType::VectorLayer:
            {
              QString count;
              if ( mSublayer.featureCount() == static_cast< long long >( Qgis::FeatureCountState::Uncounted )
                   || mSublayer.featureCount() == static_cast< long long >( Qgis::FeatureCountState::UnknownCount ) )
                count = QObject::tr( "Uncounted" );
              else
                count = QLocale().toString( mSublayer.featureCount() );

              if ( !mSublayer.description().isEmpty() )
                return QStringLiteral( "%1 - %2 (%3)" ).arg( mSublayer.description(),
                       QgsWkbTypes::displayString( mSublayer.wkbType() ),
                       count );
              else
                return QStringLiteral( "%2 (%3)" ).arg(
                         QgsWkbTypes::displayString( mSublayer.wkbType() ),
                         count );
            }

            case QgsMapLayerType::RasterLayer:
            case QgsMapLayerType::PluginLayer:
            case QgsMapLayerType::MeshLayer:
            case QgsMapLayerType::VectorTileLayer:
            case QgsMapLayerType::AnnotationLayer:
            case QgsMapLayerType::PointCloudLayer:
            case QgsMapLayerType::GroupLayer:
              return mSublayer.description();
          }
          break;

        }
      }
      return mSublayer.name();

    }

    case Qt::DecorationRole:
    {
      if ( column == 0 )
        return mSublayer.type() == QgsMapLayerType::VectorLayer
               ? ( mSublayer.wkbType() != QgsWkbTypes::Unknown ? QgsIconUtils::iconForWkbType( mSublayer.wkbType() ) : QVariant() )
               : QgsIconUtils::iconForLayerType( mSublayer.type() );
      else
        return QVariant();
    }

    case static_cast< int >( QgsProviderSublayerModel::Role::IsNonLayerItem ):
      return false;

    case static_cast< int >( QgsProviderSublayerModel::Role::ProviderKey ):
      return mSublayer.providerKey();

    case static_cast< int >( QgsProviderSublayerModel::Role::LayerType ):
      return static_cast< int >( mSublayer.type() );

    case static_cast< int >( QgsProviderSublayerModel::Role::Uri ):
      return mSublayer.uri();

    case static_cast< int >( QgsProviderSublayerModel::Role::Name ):
      return mSublayer.name();

    case static_cast< int >( QgsProviderSublayerModel::Role::Description ):
      return mSublayer.description();

    case static_cast< int >( QgsProviderSublayerModel::Role::Path ):
      return mSublayer.path();

    case static_cast< int >( QgsProviderSublayerModel::Role::FeatureCount ):
      return mSublayer.featureCount();

    case static_cast< int >( QgsProviderSublayerModel::Role::WkbType ):
      return mSublayer.wkbType();

    case static_cast< int >( QgsProviderSublayerModel::Role::GeometryColumnName ):
      return mSublayer.geometryColumnName();

    case static_cast< int >( QgsProviderSublayerModel::Role::LayerNumber ):
      return mSublayer.layerNumber();

    case static_cast< int >( QgsProviderSublayerModel::Role::Flags ):
      return static_cast< int >( mSublayer.flags() );

    default:
      return QVariant();
  }
}

QgsProviderSublayerModelNonLayerItemNode::QgsProviderSublayerModelNonLayerItemNode( const QgsProviderSublayerModel::NonLayerItem &item )
  : mItem( item )
{
}

QVariant QgsProviderSublayerModelNonLayerItemNode::data( int role, int column ) const
{
  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
    case Qt::EditRole:
    {
      switch ( static_cast< QgsProviderSublayerModel::Column >( column ) )
      {
        case QgsProviderSublayerModel::Column::Name:
          return mItem.name();
        case QgsProviderSublayerModel::Column::Description:
          return mItem.description();
      }
      return QVariant();
    }

    case Qt::DecorationRole:
    {
      if ( column == 0 )
        return mItem.icon();
      else
        return QVariant();
    }

    case static_cast< int >( QgsProviderSublayerModel::Role::IsNonLayerItem ):
      return true;

    case static_cast< int >( QgsProviderSublayerModel::Role::Uri ):
      return mItem.uri();

    case static_cast< int >( QgsProviderSublayerModel::Role::Name ):
      return mItem.name();

    case static_cast< int >( QgsProviderSublayerModel::Role::Description ):
      return mItem.description();

    case static_cast< int >( QgsProviderSublayerModel::Role::NonLayerItemType ):
      return mItem.type();

    default:
      return QVariant();
  }
}

///@endcond

//
// QgsProviderSublayerModel
//

QgsProviderSublayerModel::QgsProviderSublayerModel( QObject *parent )
  : QAbstractItemModel( parent )
  , mRootNode( std::make_unique< QgsProviderSublayerModelGroup >( QString() ) )
{

}

void QgsProviderSublayerModel::setSublayerDetails( const QList<QgsProviderSublayerDetails> &details )
{
  if ( mSublayers.isEmpty() )
  {
    // initial population, just keep things simple and reset the model
    beginResetModel();
    mRootNode->populateFromSublayers( details );
    mSublayers = details;
    endResetModel();
  }
  else
  {
    // gracefully go item by item...

    // remove layers which don't exist in new list
    for ( int i = mSublayers.count() - 1; i >= 0; --i )
    {
      if ( !details.contains( mSublayers.at( i ) ) )
      {
        QgsProviderSublayerModelSublayerNode *sublayerNode = mRootNode->findSublayer( mSublayers.at( i ) );
        Q_ASSERT( sublayerNode );
        Q_ASSERT( sublayerNode->parent() );
        const int row = sublayerNode->parent()->indexOf( sublayerNode );

        beginRemoveRows( node2index( sublayerNode->parent() ), row, row );
        sublayerNode->parent()->removeChildAt( row );
        mSublayers.removeAt( i );
        endRemoveRows();
      }
    }

    // and add new layers which exist only in new list
    for ( const QgsProviderSublayerDetails &sublayer : details )
    {
      if ( !mSublayers.contains( sublayer ) )
      {
        // need to add new layer
        QgsProviderSublayerModelGroup *group = mRootNode->findGroupForPath( sublayer.path() );
        beginInsertRows( node2index( group ), group->childCount(), group->childCount() );
        group->addChild( std::make_unique< QgsProviderSublayerModelSublayerNode >( sublayer ) );
        mSublayers.append( sublayer );
        endInsertRows();
      }
    }
  }
}

QList<QgsProviderSublayerDetails> QgsProviderSublayerModel::sublayerDetails() const
{
  return mSublayers;
}

QgsProviderSublayerDetails QgsProviderSublayerModel::indexToSublayer( const QModelIndex &index ) const
{
  if ( QgsProviderSublayerModelSublayerNode *n = dynamic_cast< QgsProviderSublayerModelSublayerNode *>( index2node( index ) ) )
    return n->sublayer();
  else
    return QgsProviderSublayerDetails();
}

QgsProviderSublayerModel::NonLayerItem QgsProviderSublayerModel::indexToNonLayerItem( const QModelIndex &index ) const
{
  if ( QgsProviderSublayerModelNonLayerItemNode *n = dynamic_cast< QgsProviderSublayerModelNonLayerItemNode *>( index2node( index ) ) )
    return n->item();
  else
    return QgsProviderSublayerModel::NonLayerItem();
}

void QgsProviderSublayerModel::addNonLayerItem( const QgsProviderSublayerModel::NonLayerItem &item )
{
  beginInsertRows( QModelIndex(), mRootNode->childCount(), mRootNode->childCount() );
  mRootNode->addChild( std::make_unique< QgsProviderSublayerModelNonLayerItemNode >( item ) );
  endInsertRows();
}

QModelIndex QgsProviderSublayerModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( column < 0 || column >= columnCount()
       || row < 0 || row >= rowCount( parent ) )
  {
    // out of bounds
    return QModelIndex();
  }

  QgsProviderSublayerModelGroup *n = dynamic_cast< QgsProviderSublayerModelGroup *>( index2node( parent ) );
  if ( !n )
    return QModelIndex(); // have no children

  return createIndex( row, column, n->childAt( row ) );
}

QModelIndex QgsProviderSublayerModel::parent( const QModelIndex &child ) const
{
  if ( !child.isValid() )
    return QModelIndex();

  if ( QgsProviderSublayerModelNode *n = index2node( child ) )
  {
    return indexOfParentNode( n->parent() ); // must not be null
  }
  else
  {
    Q_ASSERT( false );
    return QModelIndex();
  }
}

int QgsProviderSublayerModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return static_cast< int >( Column::Description ) + 1;
}

int QgsProviderSublayerModel::rowCount( const QModelIndex &parent ) const
{
  QgsProviderSublayerModelNode *n = index2node( parent );
  if ( !n )
    return 0;

  return n->childCount();
}

Qt::ItemFlags QgsProviderSublayerModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
  {
    Qt::ItemFlags rootFlags = Qt::ItemFlags();
    return rootFlags;
  }

  Qt::ItemFlags f = Qt::ItemIsEnabled;

  // if index is a group, it is not selectable ...
  if ( !dynamic_cast< QgsProviderSublayerModelGroup * >( index2node( index ) ) )
  {
    f |= Qt::ItemIsSelectable;
  }
  return f;
}

QVariant QgsProviderSublayerModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();


  QgsProviderSublayerModelNode *node = index2node( index );
  if ( !node )
    return QVariant();

  return node->data( role, index.column() );
}

QVariant QgsProviderSublayerModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  switch ( orientation )
  {
    case Qt::Vertical:
      break;
    case Qt::Horizontal:
    {
      switch ( role )
      {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
        {
          switch ( static_cast< Column>( section ) )
          {
            case QgsProviderSublayerModel::Column::Name:
              return tr( "Item" );
            case QgsProviderSublayerModel::Column::Description:
              return tr( "Description" );
          }
          break;
        }
      }
      break;
    }
  }
  return QVariant();
}

///@cond PRIVATE
QgsProviderSublayerModelNode *QgsProviderSublayerModel::index2node( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return mRootNode.get();

  return reinterpret_cast<QgsProviderSublayerModelNode *>( index.internalPointer() );
}

QModelIndex QgsProviderSublayerModel::indexOfParentNode( QgsProviderSublayerModelNode *parentNode ) const
{
  Q_ASSERT( parentNode );

  QgsProviderSublayerModelGroup *grandParentNode = parentNode->parent();
  if ( !grandParentNode )
    return QModelIndex();  // root node -> invalid index

  int row = grandParentNode->indexOf( parentNode );
  Q_ASSERT( row >= 0 );

  return createIndex( row, 0, parentNode );
}

QModelIndex QgsProviderSublayerModel::node2index( QgsProviderSublayerModelNode *node ) const
{
  if ( !node || !node->parent() )
    return QModelIndex(); // this is the only root item -> invalid index

  QModelIndex parentIndex = node2index( node->parent() );

  int row = node->parent()->indexOf( node );
  Q_ASSERT( row >= 0 );
  return index( row, 0, parentIndex );
}
///@endcond

//
// QgsProviderSublayerModel::NonLayerItem
//

QString QgsProviderSublayerModel::NonLayerItem::type() const
{
  return mType;
}

void QgsProviderSublayerModel::NonLayerItem::setType( const QString &type )
{
  mType = type;
}

QString QgsProviderSublayerModel::NonLayerItem::name() const
{
  return mName;
}

void QgsProviderSublayerModel::NonLayerItem::setName( const QString &name )
{
  mName = name;
}

QString QgsProviderSublayerModel::NonLayerItem::description() const
{
  return mDescription;
}

void QgsProviderSublayerModel::NonLayerItem::setDescription( const QString &description )
{
  mDescription = description;
}

QString QgsProviderSublayerModel::NonLayerItem::uri() const
{
  return mUri;
}

void QgsProviderSublayerModel::NonLayerItem::setUri( const QString &uri )
{
  mUri = uri;
}

QIcon QgsProviderSublayerModel::NonLayerItem::icon() const
{
  return mIcon;
}

void QgsProviderSublayerModel::NonLayerItem::setIcon( const QIcon &icon )
{
  mIcon = icon;
}

bool QgsProviderSublayerModel::NonLayerItem::operator==( const QgsProviderSublayerModel::NonLayerItem &other ) const
{
  return mType == other.mType
         && mName == other.mName
         && mDescription == other.mDescription
         && mUri == other.mUri;
}

bool QgsProviderSublayerModel::NonLayerItem::operator!=( const QgsProviderSublayerModel::NonLayerItem &other ) const
{
  return !( *this == other );
}

//
// QgsProviderSublayerProxyModel
//

QgsProviderSublayerProxyModel::QgsProviderSublayerProxyModel( QObject *parent )
  : QSortFilterProxyModel( parent )
{
  setRecursiveFilteringEnabled( true );
  setDynamicSortFilter( true );
  sort( 0 );
}

bool QgsProviderSublayerProxyModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
{
  const QModelIndex sourceIndex = sourceModel()->index( source_row, 0, source_parent );

  if ( !mIncludeSystemTables && static_cast< Qgis::SublayerFlags >( sourceModel()->data( sourceIndex, static_cast< int >( QgsProviderSublayerModel::Role::Flags ) ).toInt() ) & Qgis::SublayerFlag::SystemTable )
    return false;

  if ( !mIncludeEmptyLayers && sourceModel()->data( sourceIndex, static_cast< int >( QgsProviderSublayerModel::Role::FeatureCount ) ) == 0 )
    return false;

  if ( mFilterString.trimmed().isEmpty() )
    return true;

  if ( sourceModel()->data( sourceIndex, static_cast< int >( QgsProviderSublayerModel::Role::Name ) ).toString().contains( mFilterString, Qt::CaseInsensitive ) )
    return true;

  // check against the Description column's display role as it might be different from QgsProviderSublayerModel::Role::Description
  const QModelIndex descriptionColumnIndex = sourceModel()->index( source_row, 1, source_parent );
  if ( sourceModel()->data( descriptionColumnIndex, static_cast< int >( Qt::DisplayRole ) ).toString().contains( mFilterString, Qt::CaseInsensitive ) )
    return true;

  const QVariant wkbTypeVariant =  sourceModel()->data( sourceIndex, static_cast< int >( QgsProviderSublayerModel::Role::WkbType ) );
  if ( wkbTypeVariant.isValid() )
  {
    const QgsWkbTypes::Type wkbType = static_cast< QgsWkbTypes::Type >( wkbTypeVariant.toInt() );
    if ( QgsWkbTypes::displayString( wkbType ).contains( mFilterString, Qt::CaseInsensitive ) )
      return true;
  }

  return false;
}

bool QgsProviderSublayerProxyModel::lessThan( const QModelIndex &source_left, const QModelIndex &source_right ) const
{
  const bool leftIsNonLayer = sourceModel()->data( source_left, static_cast< int >( QgsProviderSublayerModel::Role::IsNonLayerItem ) ).toBool();
  const bool rightIsNonLayer = sourceModel()->data( source_right, static_cast< int >( QgsProviderSublayerModel::Role::IsNonLayerItem ) ).toBool();

  if ( leftIsNonLayer && !rightIsNonLayer )
    return true;
  else if ( rightIsNonLayer && !leftIsNonLayer )
    return false;

  const QString leftName = sourceModel()->data( source_left, static_cast< int >( QgsProviderSublayerModel::Role::Name ) ).toString();
  const QString rightName = sourceModel()->data( source_right, static_cast< int >( QgsProviderSublayerModel::Role::Name ) ).toString();

  return QString::localeAwareCompare( leftName, rightName ) < 0;
}

bool QgsProviderSublayerProxyModel::includeSystemTables() const
{
  return mIncludeSystemTables;
}

void QgsProviderSublayerProxyModel::setIncludeSystemTables( bool include )
{
  mIncludeSystemTables = include;
  invalidateFilter();
}

bool QgsProviderSublayerProxyModel::includeEmptyLayers() const
{
  return mIncludeEmptyLayers;
}

void QgsProviderSublayerProxyModel::setIncludeEmptyLayers( bool include )
{
  mIncludeEmptyLayers = include;
  invalidateFilter();
}

QString QgsProviderSublayerProxyModel::filterString() const
{
  return mFilterString;
}

void QgsProviderSublayerProxyModel::setFilterString( const QString &filter )
{
  mFilterString = filter;
  invalidateFilter();
}
