/***************************************************************************
    qgsmeshdatasetgrouptreeview.cpp
    -------------------------------
    begin                : June 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshdatasetgrouptreeview.h"

#include "qgis.h"
#include "qgsmeshlayer.h"

#include <QList>
#include <QItemSelectionModel>
#include <QMouseEvent>

QgsMeshDatasetGroupTreeItem::QgsMeshDatasetGroupTreeItem( QgsMeshDatasetGroupTreeItem *parent )
  : mParent( parent )
{
}

QgsMeshDatasetGroupTreeItem::QgsMeshDatasetGroupTreeItem( const QString &name,
    bool isVector,
    int index,
    bool isUsed,
    QgsMeshDatasetGroupTreeItem *parent )
  : mParent( parent )
  , mName( name )
  , mIsVector( isVector )
  , mDatasetGroupIndex( index )
  , mIsUsed( isUsed )
{
}

QgsMeshDatasetGroupTreeItem::~QgsMeshDatasetGroupTreeItem()
{
  qDeleteAll( mChildren );
}

void QgsMeshDatasetGroupTreeItem::appendChild( QgsMeshDatasetGroupTreeItem *node )
{
  mChildren.append( node );
}

QgsMeshDatasetGroupTreeItem *QgsMeshDatasetGroupTreeItem::child( int row ) const
{
  if ( row < mChildren.count() )
    return mChildren.at( row );
  else
    return nullptr;
}

int QgsMeshDatasetGroupTreeItem::childCount() const
{
  return mChildren.count();
}

QgsMeshDatasetGroupTreeItem *QgsMeshDatasetGroupTreeItem::parentItem() const
{
  return mParent;
}

int QgsMeshDatasetGroupTreeItem::row() const
{
  if ( mParent )
    return mParent->mChildren.indexOf( const_cast<QgsMeshDatasetGroupTreeItem *>( this ) );

  return 0;
}

QString QgsMeshDatasetGroupTreeItem::name() const
{
  return mName;
}

bool QgsMeshDatasetGroupTreeItem::isVector() const
{
  return mIsVector;
}

int QgsMeshDatasetGroupTreeItem::datasetGroupIndex() const
{
  return mDatasetGroupIndex;
}

bool QgsMeshDatasetGroupTreeItem::isUsed() const
{
  return mIsUsed;
}

void QgsMeshDatasetGroupTreeItem::setIsUsed( bool used )
{
  mIsUsed = used;
}

void QgsMeshDatasetGroupTreeItem::setName( const QString &name )
{
  mName = name;
}

/////////////////////////////////////////////////////////////////////////////////////////

QgsMeshDatasetGroupProvidedTreeModel::QgsMeshDatasetGroupProvidedTreeModel( QObject *parent )
  : QAbstractItemModel( parent )
  ,  mRootItem( new QgsMeshDatasetGroupTreeItem() )
{
}

int QgsMeshDatasetGroupProvidedTreeModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 1;
}

bool QgsMeshDatasetGroupProvidedTreeModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() )
    return false;

  QgsMeshDatasetGroupTreeItem *item = static_cast<QgsMeshDatasetGroupTreeItem *>( index.internalPointer() );

  switch ( role )
  {
    case Qt::EditRole:
      if ( value != QString() )
      {
        item->setName( value.toString() );
        return true;
      }
      break;
    case Qt::CheckStateRole :
      item->setIsUsed( value.toBool() );
      return true;
  }
  return false;
}

QVariant QgsMeshDatasetGroupProvidedTreeModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  QgsMeshDatasetGroupTreeItem *item = static_cast<QgsMeshDatasetGroupTreeItem *>( index.internalPointer() );

  switch ( role )
  {
    case Qt::DisplayRole:
    case Name:
      return item->name();
    case IsVector:
      return item->isVector();
    case DatasetGroupIndex:
      return item->datasetGroupIndex();
    case Qt::CheckStateRole :
      return static_cast< int >( item->isUsed() ? Qt::Checked : Qt::Unchecked );
  }

  return QVariant();
}

Qt::ItemFlags QgsMeshDatasetGroupProvidedTreeModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return Qt::NoItemFlags;

  return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
}

QVariant QgsMeshDatasetGroupProvidedTreeModel::headerData( int section,
    Qt::Orientation orientation,
    int role ) const
{
  Q_UNUSED( section )

  if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
    return tr( "Groups" );

  return QVariant();
}

QModelIndex QgsMeshDatasetGroupProvidedTreeModel::index( int row, int column, const QModelIndex &parent )
const
{
  if ( !hasIndex( row, column, parent ) )
    return QModelIndex();

  QgsMeshDatasetGroupTreeItem *parentItem;

  if ( !parent.isValid() )
    parentItem = mRootItem.get();
  else
    parentItem = static_cast<QgsMeshDatasetGroupTreeItem *>( parent.internalPointer() );

  QgsMeshDatasetGroupTreeItem *childItem = parentItem->child( row );
  if ( childItem )
    return createIndex( row, column, childItem );
  else
    return QModelIndex();
}

QModelIndex QgsMeshDatasetGroupProvidedTreeModel::parent( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return QModelIndex();

  QgsMeshDatasetGroupTreeItem *childItem = static_cast<QgsMeshDatasetGroupTreeItem *>( index.internalPointer() );
  QgsMeshDatasetGroupTreeItem *parentItem = childItem->parentItem();

  if ( parentItem == mRootItem.get() )
    return QModelIndex();

  return createIndex( parentItem->row(), 0, parentItem );
}

int QgsMeshDatasetGroupProvidedTreeModel::rowCount( const QModelIndex &parent ) const
{
  QgsMeshDatasetGroupTreeItem *parentItem;
  if ( parent.column() > 0 )
    return 0;

  if ( !parent.isValid() )
    parentItem = mRootItem.get();
  else
    parentItem = static_cast<QgsMeshDatasetGroupTreeItem *>( parent.internalPointer() );

  return parentItem->childCount();
}

void QgsMeshDatasetGroupProvidedTreeModel::syncToLayer( QgsMeshLayer *layer )
{
  beginResetModel();

  mRootItem.reset( new QgsMeshDatasetGroupTreeItem() );
  mNameToItem.clear();
  mDatasetGroupIndexToItem.clear();

  if ( layer && layer->dataProvider() )
  {
    const QgsMeshDataProvider *dp = layer->dataProvider();
    for ( int groupIndex = 0; groupIndex < dp->datasetGroupCount(); ++groupIndex )
    {
      const QgsMeshDatasetGroupState &state = layer->datasetGroupStates().value( groupIndex, QgsMeshDatasetGroupState() );
      const QgsMeshDatasetGroupMetadata meta = dp->datasetGroupMetadata( groupIndex );
      const QString metaName = meta.name();

      QString name = state.renaming;
      if ( name.isEmpty() )
        name = state.originalName;

      const bool isVector = meta.isVector();
      const QStringList subdatasets = metaName.split( '/' );

      if ( subdatasets.size() == 1 )
      {
        if ( name.isEmpty() )
          name = metaName;
        addTreeItem( metaName, name, isVector, groupIndex, state.used, mRootItem.get() );
      }
      else if ( subdatasets.size() == 2 )
      {
        auto i = mNameToItem.find( subdatasets[0] );
        if ( i == mNameToItem.end() )
        {
          QgsDebugMsg( QStringLiteral( "Unable to find parent group for %1." ).arg( metaName ) );
          addTreeItem( metaName, name, isVector, groupIndex, state.used, mRootItem.get() );
        }
        else
        {
          if ( name.isEmpty() )
            name = subdatasets[1];
          addTreeItem( metaName, name, isVector, groupIndex, state.used, i.value() );
        }
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "Ignoring too deep child group name %1." ).arg( name ) );
        addTreeItem( metaName, name, isVector, groupIndex, state.used, mRootItem.get() );
      }
    }
  }
  endResetModel();
}

bool QgsMeshDatasetGroupProvidedTreeModel::isUsed( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return false;

  QVariant checked = data( index, Qt::CheckStateRole );

  return checked != QVariant() && checked.toInt() == Qt::Checked;
}

QMap<int, QgsMeshDatasetGroupState> QgsMeshDatasetGroupProvidedTreeModel::groupStates() const
{
  QMap<int, QgsMeshDatasetGroupState> ret;

  for ( const QgsMeshDatasetGroupTreeItem *item : mDatasetGroupIndexToItem )
  {
    QgsMeshDatasetGroupState state;
    state.used = item->isUsed();
    state.renaming = item->name();

    ret[item->datasetGroupIndex()] = state;
  }
  return ret;
}

void QgsMeshDatasetGroupProvidedTreeModel::resetToDefaultState( QgsMeshLayer *meshLayer )
{
  if ( !meshLayer )
    return;

  QMap<int, QgsMeshDatasetGroupState> stateGroups = meshLayer->datasetGroupStates();

  for ( int i = 0; i < mRootItem->childCount(); ++i )
  {
    QgsMeshDatasetGroupTreeItem *item = mRootItem->child( i );
    if ( stateGroups.contains( item->datasetGroupIndex() ) )
      item->setName( stateGroups[item->datasetGroupIndex()].originalName );

    for ( int j = 0; j < item->childCount(); ++j )
    {
      QgsMeshDatasetGroupTreeItem *child = item->child( j );
      if ( stateGroups.contains( child->datasetGroupIndex() ) )
        child->setName( stateGroups[child->datasetGroupIndex()].originalName );
    }
  }
  dataChanged( index( 0, 0 ), index( mRootItem->childCount(), 0 ) );
}

void QgsMeshDatasetGroupProvidedTreeModel::setAllGroupsAsUsed( bool isUsed )
{
  for ( int i = 0; i < mRootItem->childCount(); ++i )
  {
    QgsMeshDatasetGroupTreeItem *item = mRootItem->child( i );
    item->setIsUsed( isUsed );
    for ( int j = 0; j < item->childCount(); ++j )
    {
      QgsMeshDatasetGroupTreeItem *child = item->child( j );
      child->setIsUsed( isUsed );
    }
  }
  dataChanged( index( 0, 0 ), index( mRootItem->childCount(), 0 ) );
}

void QgsMeshDatasetGroupProvidedTreeModel::addTreeItem( const QString &groupName,
    const QString &displayName,
    bool isVector,
    int groupIndex,
    bool isUsed,
    QgsMeshDatasetGroupTreeItem *parent )
{
  Q_ASSERT( parent );
  QgsMeshDatasetGroupTreeItem *item = new QgsMeshDatasetGroupTreeItem( displayName, isVector, groupIndex, isUsed, parent );
  parent->appendChild( item );

  if ( mNameToItem.contains( groupName ) )
  {
    QgsDebugMsg( QStringLiteral( "Group %1 is not unique" ).arg( groupName ) );
  }
  mNameToItem[groupName] = item;

  if ( mDatasetGroupIndexToItem.contains( groupIndex ) )
  {
    QgsDebugMsg( QStringLiteral( "Group index %1 is not unique" ).arg( groupIndex ) );
  }
  mDatasetGroupIndexToItem[groupIndex] = item;
}


/////////////////////////////////////////////////////////////////////////////////////////

QgsMeshDatasetGroupFilterUsedModel::QgsMeshDatasetGroupFilterUsedModel( QAbstractItemModel *sourceModel ):
  QSortFilterProxyModel( sourceModel )
{
  setSourceModel( sourceModel );
}

int QgsMeshDatasetGroupFilterUsedModel::activeScalarGroup() const
{
  return mActiveScalarGroupIndex;
}

void QgsMeshDatasetGroupFilterUsedModel::setActiveScalarGroup( int group )
{
  if ( mActiveScalarGroupIndex == group )
    return;

  int oldGroupIndex = mActiveScalarGroupIndex;
  mActiveScalarGroupIndex = group;

  if ( oldGroupIndex > -1  || group > -1 )
    invalidate();
}

int QgsMeshDatasetGroupFilterUsedModel::activeVectorGroup() const
{
  return mActiveVectorGroupIndex;
}

void QgsMeshDatasetGroupFilterUsedModel::setActiveVectorGroup( int group )
{
  if ( mActiveVectorGroupIndex == group )
    return;

  int oldGroupIndex = mActiveVectorGroupIndex;
  mActiveVectorGroupIndex = group;

  if ( oldGroupIndex > -1  || group > -1 )
    invalidate();
}

Qt::ItemFlags QgsMeshDatasetGroupFilterUsedModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return Qt::NoItemFlags;

  return Qt::ItemIsEnabled;
}

QVariant QgsMeshDatasetGroupFilterUsedModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  QModelIndex sourceIndex = mapToSource( index );
  QgsMeshDatasetGroupTreeItem *item = static_cast<QgsMeshDatasetGroupTreeItem *>( sourceIndex.internalPointer() );

  switch ( role )
  {
    case QgsMeshDatasetGroupProvidedTreeModel::IsActiveScalarDatasetGroup:
      return item->datasetGroupIndex() == mActiveScalarGroupIndex;
    case QgsMeshDatasetGroupProvidedTreeModel::IsActiveVectorDatasetGroup:
      return item->datasetGroupIndex() == mActiveVectorGroupIndex;
    case Qt::CheckStateRole :
      return QVariant();
  }

  return sourceModel()->data( sourceIndex, role );
}

void QgsMeshDatasetGroupFilterUsedModel::syncToLayer( QgsMeshLayer *layer )
{
  static_cast<QgsMeshDatasetGroupProvidedTreeModel *>( sourceModel() )->syncToLayer( layer );
  accordActiveGroupToUsedGroup();
}

bool QgsMeshDatasetGroupFilterUsedModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
{
  QModelIndex sourceIndex = sourceModel()->index( source_row, 0, source_parent );

  return static_cast<QgsMeshDatasetGroupProvidedTreeModel *>( sourceModel() )->isUsed( sourceIndex );
}

void QgsMeshDatasetGroupFilterUsedModel::accordActiveGroupToUsedGroup()
{
  //If the active group is not used anymore, change the active group : first used for scalar, inactive for vector

  QMap<int, QgsMeshDatasetGroupState> groupStates = static_cast<QgsMeshDatasetGroupProvidedTreeModel *>( sourceModel() )->groupStates();

  if ( !groupStates.contains( mActiveScalarGroupIndex ) || !groupStates[mActiveScalarGroupIndex].used )
  {
    mActiveScalarGroupIndex = 0;
    while ( groupStates.contains( mActiveScalarGroupIndex ) && !groupStates[mActiveScalarGroupIndex].used )
      mActiveScalarGroupIndex++;

    if ( !groupStates.contains( mActiveScalarGroupIndex ) )
      mActiveScalarGroupIndex = -1;

    invalidate();
  }

  if ( !groupStates.contains( mActiveVectorGroupIndex ) || !groupStates[mActiveVectorGroupIndex].used )
  {
    mActiveVectorGroupIndex = -1;
    invalidate();
  }
}

/////////////////////////////////////////////////////////////////////////////////////////

QgsMeshDatasetGroupTreeItemDelagate::QgsMeshDatasetGroupTreeItemDelagate( QObject *parent )
  : QStyledItemDelegate( parent )
  , mScalarSelectedPixmap( QStringLiteral( ":/images/themes/default/propertyicons/meshcontours.svg" ) )
  , mScalarDeselectedPixmap( QStringLiteral( ":/images/themes/default/propertyicons/meshcontoursoff.svg" ) )
  , mVectorSelectedPixmap( QStringLiteral( ":/images/themes/default/propertyicons/meshvectors.svg" ) )
  , mVectorDeselectedPixmap( QStringLiteral( ":/images/themes/default/propertyicons/meshvectorsoff.svg" ) )
{
}

void QgsMeshDatasetGroupTreeItemDelagate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  if ( !painter )
    return;

  QStyledItemDelegate::paint( painter, option, index );
  bool isVector = index.data( QgsMeshDatasetGroupProvidedTreeModel::IsVector ).toBool();
  if ( isVector )
  {
    bool isActive = index.data( QgsMeshDatasetGroupProvidedTreeModel::IsActiveVectorDatasetGroup ).toBool();
    painter->drawPixmap( iconRect( option.rect, true ), isActive ? mVectorSelectedPixmap : mVectorDeselectedPixmap );
  }
  bool isActive = index.data( QgsMeshDatasetGroupProvidedTreeModel::IsActiveScalarDatasetGroup ).toBool();
  painter->drawPixmap( iconRect( option.rect, false ), isActive ? mScalarSelectedPixmap : mScalarDeselectedPixmap );
}

QRect QgsMeshDatasetGroupTreeItemDelagate::iconRect( const QRect rect, bool isVector ) const
{
  int iw = mScalarSelectedPixmap.width();
  int ih = mScalarSelectedPixmap.height();
  int margin = ( rect.height() - ih ) / 2;
  int i = isVector ? 1 : 2;
  return QRect( rect.right() - i * ( iw + margin ), rect.top() + margin, iw, ih );
}

QSize QgsMeshDatasetGroupTreeItemDelagate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  QSize hint = QStyledItemDelegate::sizeHint( option, index );
  if ( hint.height() < 16 )
    hint.setHeight( 16 );
  return hint;
}

/////////////////////////////////////////////////////////////////////////////////////////

QgsMeshDatasetGroupTreeView::QgsMeshDatasetGroupTreeView( QWidget *parent )
  : QTreeView( parent ),
    mUsedModel( new QgsMeshDatasetGroupFilterUsedModel( new QgsMeshDatasetGroupProvidedTreeModel( this ) ) )
{
  setModel( mUsedModel );
  setItemDelegate( &mDelegate );
  setSelectionMode( QAbstractItemView::SingleSelection );
}

void QgsMeshDatasetGroupTreeView::setLayer( QgsMeshLayer *layer )
{
  if ( layer != mMeshLayer )
  {
    mMeshLayer = layer;
  }
}

int QgsMeshDatasetGroupTreeView::activeScalarGroup() const
{
  return mUsedModel->activeScalarGroup();
}

void QgsMeshDatasetGroupTreeView::setActiveScalarGroup( int group )
{
  if ( mUsedModel->activeScalarGroup() != group )
  {
    mUsedModel->setActiveScalarGroup( group );
    mUsedModel->invalidate();
    emit activeScalarGroupChanged( group );
  }
}

int QgsMeshDatasetGroupTreeView::activeVectorGroup() const
{
  return mUsedModel->activeVectorGroup();
}

void QgsMeshDatasetGroupTreeView::setActiveVectorGroup( int group )
{
  if ( mUsedModel->activeVectorGroup() != group )
  {
    mUsedModel->setActiveVectorGroup( group );
    mUsedModel->invalidate();
    emit activeVectorGroupChanged( group );
  }
}

void QgsMeshDatasetGroupTreeView::syncToLayer()
{
  mUsedModel->syncToLayer( mMeshLayer );
  setActiveGroup();
}

void QgsMeshDatasetGroupTreeView::onActiveGroupChanged()
{
  int activeScalar = activeScalarGroup();
  int activeVector = activeVectorGroup();

  mUsedModel->syncToLayer( mMeshLayer );

  if ( activeScalarGroup() != activeScalar )
    emit activeScalarGroupChanged( activeScalarGroup() );
  if ( activeVectorGroup() != activeVector )
    emit activeVectorGroupChanged( activeVectorGroup() );
}

void QgsMeshDatasetGroupTreeView::mousePressEvent( QMouseEvent *event )
{
  if ( !event )
    return;

  bool processed = false;
  const QModelIndex idx = indexAt( event->pos() ) ;
  if ( idx.isValid() )
  {
    const QRect vr = visualRect( idx );
    if ( mDelegate.iconRect( vr, true ).contains( event->pos() ) )
    {
      bool isVector = idx.data( QgsMeshDatasetGroupProvidedTreeModel::IsVector ).toBool();
      if ( isVector )
      {
        setActiveVectorGroup( idx.data( QgsMeshDatasetGroupProvidedTreeModel::DatasetGroupIndex ).toInt() );
        processed = true;
      }
    }
    else if ( mDelegate.iconRect( vr, false ).contains( event->pos() ) )
    {
      setActiveScalarGroup( idx.data( QgsMeshDatasetGroupProvidedTreeModel::DatasetGroupIndex ).toInt() );
      processed = true;
    }
  }

  // only if the user did not click one of the icons do usual handling
  if ( !processed )
    QTreeView::mousePressEvent( event );
}

void QgsMeshDatasetGroupTreeView::setActiveGroup()
{
  int scalarGroup = -1;
  int vectorGroup = -1;

  // find active dataset
  if ( mMeshLayer )
  {
    const QgsMeshRendererSettings rendererSettings = mMeshLayer->rendererSettings();
    scalarGroup = rendererSettings.activeScalarDatasetGroup();
    vectorGroup = rendererSettings.activeVectorDatasetGroup();
  }

  setActiveScalarGroup( scalarGroup );
  setActiveVectorGroup( vectorGroup );
}

void QgsMeshDatasetGroupProvidedListModel::syncToLayer( QgsMeshLayer *layer )
{
  mLayer = layer;
}

int QgsMeshDatasetGroupProvidedListModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  if ( mLayer )
    return mLayer->dataProvider()->datasetGroupCount();
  else
    return 0;
}

QVariant QgsMeshDatasetGroupProvidedListModel::data( const QModelIndex &index, int role ) const
{
  if ( !mLayer || ! index.isValid() )
    return QVariant();

  QgsMeshDataProvider *dataProvider = mLayer->dataProvider();

  if ( !dataProvider || index.row() >= dataProvider->datasetGroupCount() )
    return QVariant();

  QgsMeshDatasetGroupState state;
  if ( mLayer->datasetGroupStates().contains( index.row() ) )
    state = mLayer->datasetGroupStates()[index.row()];

  if ( role == Qt::DisplayRole )
  {
    QString name = state.renaming;
    if ( name == QString() )
      name = state.originalName;
    return name;
  }

  return QVariant();
}

QgsMeshDatasetGroupProvidedTreeView::QgsMeshDatasetGroupProvidedTreeView( QWidget *parent ):
  QTreeView( parent )
  , mModel( new QgsMeshDatasetGroupProvidedTreeModel( this ) )
{
  setModel( mModel );
  setSelectionMode( QAbstractItemView::SingleSelection );
}

void QgsMeshDatasetGroupProvidedTreeView::syncToLayer( QgsMeshLayer *layer )
{
  if ( mModel )
    mModel->syncToLayer( layer );
}

QMap<int, QgsMeshDatasetGroupState> QgsMeshDatasetGroupProvidedTreeView::groupStates() const
{
  return mModel->groupStates();
}

void QgsMeshDatasetGroupProvidedTreeView::checkAll()
{
  checkAllItem( true );
}

void QgsMeshDatasetGroupProvidedTreeView::uncheckAll()
{
  checkAllItem( false );
}

void QgsMeshDatasetGroupProvidedTreeView::resetDefault( QgsMeshLayer *meshLayer )
{
  mModel->resetToDefaultState( meshLayer );
}

void QgsMeshDatasetGroupProvidedTreeView::checkAllItem( bool isChecked )
{
  mModel->setAllGroupsAsUsed( isChecked );
}
