/***************************************************************************
                         qgsmeshdatasetgroupstore.cpp
                         ---------------------
    begin                : June 2020
    copyright            : (C) 2020 by Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshdatasetgroupstore.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayerutils.h"
#include "qgsapplication.h"
#include "qgsmeshvirtualdatasetgroup.h"
#include "qgslogger.h"

QList<int> QgsMeshDatasetGroupStore::datasetGroupIndexes() const
{
  return mRegistry.keys();
}

QList<int> QgsMeshDatasetGroupStore::enabledDatasetGroupIndexes() const
{
  return mDatasetGroupTreeRootItem->enabledDatasetGroupIndexes();
}

int QgsMeshDatasetGroupStore::datasetGroupCount() const
{
  return mRegistry.count();
}

int QgsMeshDatasetGroupStore::extraDatasetGroupCount() const
{
  return mExtraDatasets.datasetGroupCount();
}

QgsMeshDatasetGroupStore::QgsMeshDatasetGroupStore( QgsMeshLayer *layer ):
  mLayer( layer ),
  mDatasetGroupTreeRootItem( new QgsMeshDatasetGroupTreeItem )
{}

void QgsMeshDatasetGroupStore::setPersistentProvider( QgsMeshDataProvider *provider, const QStringList &extraDatasetUri )
{
  removePersistentProvider();
  mPersistentProvider = provider;
  if ( !mPersistentProvider )
    return;
  for ( const QString &uri : extraDatasetUri )
    mPersistentProvider->addDataset( uri );

  onPersistentDatasetAdded( mPersistentProvider->datasetGroupCount() );

  checkDatasetConsistency( mPersistentProvider );
  removeUnregisteredItemFromTree();

  //Once everything is in place, initialize the extra dataset groups
  const int groupCount = mExtraDatasets.datasetGroupCount();
  for ( int i = 0; i < groupCount; ++i )
    if ( mExtraDatasets.datasetGroup( i ) )
      mExtraDatasets.datasetGroup( i )->initialize();

  mExtraDatasets.updateTemporalCapabilities();

  connect( mPersistentProvider, &QgsMeshDataProvider::datasetGroupsAdded, this, &QgsMeshDatasetGroupStore::onPersistentDatasetAdded );
}

QgsMeshDatasetGroupStore::DatasetGroup QgsMeshDatasetGroupStore::datasetGroup( int index ) const
{
  return mRegistry.value( index, DatasetGroup{nullptr, -1} );
}

bool QgsMeshDatasetGroupStore::addPersistentDatasets( const QString &path )
{
  if ( !mPersistentProvider )
    return false;
  return mPersistentProvider->addDataset( path ) ;
}

bool QgsMeshDatasetGroupStore::addDatasetGroup( QgsMeshDatasetGroup *group )
{
  if ( !mLayer )
    return false;

  switch ( group->dataType() )
  {
    case QgsMeshDatasetGroupMetadata::DataOnFaces:
      if ( ! group->checkValueCountPerDataset( mLayer->meshFaceCount() ) )
        return false;
      break;
    case QgsMeshDatasetGroupMetadata::DataOnVertices:
      if ( ! group->checkValueCountPerDataset( mLayer->meshVertexCount() ) )
        return false;
      break;
    case QgsMeshDatasetGroupMetadata::DataOnVolumes:
      return false; // volume not supported for extra dataset
      break;
    case QgsMeshDatasetGroupMetadata::DataOnEdges:
      if ( ! group->checkValueCountPerDataset( mLayer->meshEdgeCount() ) )
        return false;
      break;
  }

  int nativeIndex = mExtraDatasets.addDatasetGroup( group );
  int groupIndex = registerDatasetGroup( DatasetGroup{&mExtraDatasets, nativeIndex} );

  if ( groupIndex == -1 )
    return false;

  QList<int> groupIndexes;
  groupIndexes.append( groupIndex );
  createDatasetGroupTreeItems( groupIndexes );
  syncItemToDatasetGroup( groupIndex );

  emit datasetGroupsAdded( groupIndexes );

  return true;
}

void QgsMeshDatasetGroupStore::resetDatasetGroupTreeItem()
{
  mDatasetGroupTreeRootItem.reset( new QgsMeshDatasetGroupTreeItem );
  createDatasetGroupTreeItems( datasetGroupIndexes() );
  QList<int> groupIndexes = datasetGroupIndexes();
  for ( int groupIndex : groupIndexes )
    syncItemToDatasetGroup( groupIndex );
}

QgsMeshDatasetGroupTreeItem *QgsMeshDatasetGroupStore::datasetGroupTreeItem() const
{
  return mDatasetGroupTreeRootItem.get();
}

void QgsMeshDatasetGroupStore::setDatasetGroupTreeItem( const QgsMeshDatasetGroupTreeItem *rootItem )
{
  if ( rootItem )
    mDatasetGroupTreeRootItem.reset( rootItem->clone() );
  else
    mDatasetGroupTreeRootItem.reset();

  unregisterGroupNotPresentInTree();
}

QgsMeshDatasetGroupMetadata QgsMeshDatasetGroupStore::datasetGroupMetadata( const QgsMeshDatasetIndex &index ) const
{
  QgsMeshDatasetGroupStore::DatasetGroup  group = datasetGroup( index.group() );
  if ( group.first )
    return group.first->datasetGroupMetadata( group.second );
  else
    return QgsMeshDatasetGroupMetadata();
}

int QgsMeshDatasetGroupStore::datasetCount( int groupIndex ) const
{
  QgsMeshDatasetGroupStore::DatasetGroup  group = datasetGroup( groupIndex );
  if ( group.first )
    return group.first->datasetCount( group.second );
  else
    return 0;
}

QgsMeshDatasetMetadata QgsMeshDatasetGroupStore::datasetMetadata( const QgsMeshDatasetIndex &index ) const
{
  QgsMeshDatasetGroupStore::DatasetGroup  group = datasetGroup( index.group() );
  if ( group.first )
    return group.first->datasetMetadata( QgsMeshDatasetIndex( group.second, index.dataset() ) );
  else
    return QgsMeshDatasetMetadata();
}

QgsMeshDatasetValue QgsMeshDatasetGroupStore::datasetValue( const QgsMeshDatasetIndex &index, int valueIndex ) const
{
  QgsMeshDatasetGroupStore::DatasetGroup  group = datasetGroup( index.group() );
  if ( group.first )
    return group.first->datasetValue( QgsMeshDatasetIndex( group.second, index.dataset() ), valueIndex );
  else
    return QgsMeshDatasetValue();
}

QgsMeshDataBlock QgsMeshDatasetGroupStore::datasetValues( const QgsMeshDatasetIndex &index, int valueIndex, int count ) const
{
  QgsMeshDatasetGroupStore::DatasetGroup  group = datasetGroup( index.group() );
  if ( group.first )
    return group.first->datasetValues( QgsMeshDatasetIndex( group.second, index.dataset() ), valueIndex, count );
  else
    return QgsMeshDataBlock();
}

QgsMesh3DDataBlock QgsMeshDatasetGroupStore::dataset3dValues( const QgsMeshDatasetIndex &index, int faceIndex, int count ) const
{
  QgsMeshDatasetGroupStore::DatasetGroup  group = datasetGroup( index.group() );
  if ( group.first )
    return group.first->dataset3dValues( QgsMeshDatasetIndex( group.second, index.dataset() ), faceIndex, count );
  else
    return QgsMesh3DDataBlock();
}

QgsMeshDataBlock QgsMeshDatasetGroupStore::areFacesActive( const QgsMeshDatasetIndex &index, int faceIndex, int count ) const
{
  QgsMeshDatasetGroupStore::DatasetGroup  group = datasetGroup( index.group() );
  if ( group.first )
    return group.first->areFacesActive( QgsMeshDatasetIndex( group.second, index.dataset() ), faceIndex, count );
  else
    return QgsMeshDataBlock();
}

bool QgsMeshDatasetGroupStore::isFaceActive( const QgsMeshDatasetIndex &index, int faceIndex ) const
{
  QgsMeshDatasetGroupStore::DatasetGroup  group = datasetGroup( index.group() );
  if ( group.first )
    return group.first->isFaceActive( QgsMeshDatasetIndex( group.second, index.dataset() ), faceIndex );
  else
    return false;
}

QgsMeshDatasetIndex QgsMeshDatasetGroupStore::datasetIndexAtTime(
  qint64 time,
  int groupIndex, QgsMeshDataProviderTemporalCapabilities::MatchingTemporalDatasetMethod method ) const
{
  QgsMeshDatasetGroupStore::DatasetGroup  group = datasetGroup( groupIndex );
  if ( !group.first )
    return QgsMeshDatasetIndex();

  const QDateTime &referenceTime = mPersistentProvider ? mPersistentProvider->temporalCapabilities()->referenceTime() : QDateTime();

  return QgsMeshDatasetIndex( groupIndex,
                              group.first->datasetIndexAtTime( referenceTime, group.second, time, method ).dataset() );
}

QList<QgsMeshDatasetIndex> QgsMeshDatasetGroupStore::datasetIndexInTimeInterval(
  qint64 time1,
  qint64 time2,
  int groupIndex ) const
{
  const QgsMeshDatasetGroupStore::DatasetGroup  group = datasetGroup( groupIndex );
  if ( !group.first )
    return QList<QgsMeshDatasetIndex>();

  const QDateTime &referenceTime = mPersistentProvider ? mPersistentProvider->temporalCapabilities()->referenceTime() : QDateTime();

  const QList<QgsMeshDatasetIndex> datasetIndexes = group.first->datasetIndexInTimeInterval( referenceTime, group.second, time1, time2 );

  QList<QgsMeshDatasetIndex> ret;
  ret.reserve( datasetIndexes.count() );

  for ( const QgsMeshDatasetIndex &sourceDatasetIndex : datasetIndexes )
    ret.append( QgsMeshDatasetIndex( groupIndex, sourceDatasetIndex.dataset() ) );

  return ret;
}

qint64 QgsMeshDatasetGroupStore::datasetRelativeTime( const QgsMeshDatasetIndex &index ) const
{
  QgsMeshDatasetGroupStore::DatasetGroup  group = datasetGroup( index.group() );
  if ( !group.first || group.second < 0 )
    return INVALID_MESHLAYER_TIME;

  QgsMeshDatasetIndex nativeIndex( group.second, index.dataset() );

  if ( group.first == mPersistentProvider )
    return mPersistentProvider->temporalCapabilities()->datasetTime( nativeIndex );
  else if ( group.first == &mExtraDatasets )
    return mExtraDatasets.datasetRelativeTime( nativeIndex );

  return INVALID_MESHLAYER_TIME;

}

bool QgsMeshDatasetGroupStore::hasTemporalCapabilities() const
{
  return ( mPersistentProvider && mPersistentProvider->temporalCapabilities()->hasTemporalCapabilities() ) ||
         ( mExtraDatasets.hasTemporalCapabilities() );
}

QDomElement QgsMeshDatasetGroupStore::writeXml( QDomDocument &doc, const QgsReadWriteContext &context )
{
  Q_UNUSED( context );
  QDomElement storeElement = doc.createElement( QStringLiteral( "mesh-dataset-groups-store" ) );
  storeElement.appendChild( mDatasetGroupTreeRootItem->writeXml( doc, context ) );

  QMap < int, DatasetGroup>::const_iterator it = mRegistry.constBegin();
  while ( it != mRegistry.constEnd() )
  {
    QDomElement elemDataset;
    if ( it.value().first == mPersistentProvider )
    {
      elemDataset = doc.createElement( QStringLiteral( "mesh-dataset" ) );
      elemDataset.setAttribute( QStringLiteral( "global-index" ), it.key() );
      elemDataset.setAttribute( QStringLiteral( "source-type" ), QStringLiteral( "persitent-provider" ) );
      elemDataset.setAttribute( QStringLiteral( "source-index" ), it.value().second );
    }
    else if ( it.value().first == &mExtraDatasets )
    {
      QgsMeshDatasetGroupTreeItem *item = mDatasetGroupTreeRootItem->childFromDatasetGroupIndex( it.key() );
      if ( item )
      {
        elemDataset = mExtraDatasets.writeXml( it.value().second, doc, context );
        if ( !elemDataset.isNull() )
          elemDataset.setAttribute( QStringLiteral( "global-index" ), it.key() );
      }
    }

    if ( !elemDataset.isNull() )
      storeElement.appendChild( elemDataset );
    ++it;
  }

  for ( auto it = mGroupNameToGlobalIndex.constBegin(); it != mGroupNameToGlobalIndex.constEnd(); ++it )
  {
    QDomElement elemNameToIndex = doc.createElement( QStringLiteral( "name-to-global-index" ) );
    elemNameToIndex.setAttribute( QStringLiteral( "name" ), it.key() );
    elemNameToIndex.setAttribute( QStringLiteral( "global-index" ), it.value() );

    storeElement.appendChild( elemNameToIndex );
  }

  return storeElement;
}

void QgsMeshDatasetGroupStore::readXml( const QDomElement &storeElem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context );
  mRegistry.clear();
  QDomElement datasetElem = storeElem.firstChildElement( "mesh-dataset" );
  QMap<int, QgsMeshDatasetGroup *> extraDatasetGroups;
  while ( !datasetElem.isNull() )
  {
    int globalIndex = datasetElem.attribute( QStringLiteral( "global-index" ) ).toInt();

    const QString sourceType = datasetElem.attribute( QStringLiteral( "source-type" ) );
    if ( sourceType == QLatin1String( "persitent-provider" ) )
    {
      mPersistentExtraDatasetGroupIndexes.append( globalIndex );
    }
    else if ( sourceType == QLatin1String( "virtual" ) )
    {
      QgsMeshDatasetSourceInterface *source = &mExtraDatasets;
      QString name = datasetElem.attribute( QStringLiteral( "name" ) );
      QString formula = datasetElem.attribute( QStringLiteral( "formula" ) );
      qint64 startTime = datasetElem.attribute( QStringLiteral( "start-time" ) ).toLongLong();
      qint64 endTime = datasetElem.attribute( QStringLiteral( "end-time" ) ).toLongLong();

      QgsMeshDatasetGroup *dsg = new QgsMeshVirtualDatasetGroup( name, formula, mLayer, startTime, endTime );
      extraDatasetGroups[globalIndex] = dsg;
      int sourceIndex = mExtraDatasets.addDatasetGroup( dsg );

      mRegistry[globalIndex] = DatasetGroup{source, sourceIndex};
    }
    else
    {
      QgsDebugError( QStringLiteral( "Unhandled source-type: %1." ).arg( sourceType ) );
    }

    datasetElem = datasetElem.nextSiblingElement( QStringLiteral( "mesh-dataset" ) );
  }

  QDomElement nameToIndexElem = storeElem.firstChildElement( "name-to-global-index" );
  mGroupNameToGlobalIndex.clear();
  while ( !nameToIndexElem.isNull() )
  {
    QString name = nameToIndexElem.attribute( QStringLiteral( "name" ) );
    int globalIndex = nameToIndexElem.attribute( QStringLiteral( "global-index" ) ).toInt();

    mGroupNameToGlobalIndex.insert( name, globalIndex );

    nameToIndexElem = nameToIndexElem.nextSiblingElement( QStringLiteral( "name-to-global-index" ) );
  }

  QDomElement rootTreeItemElem = storeElem.firstChildElement( QStringLiteral( "mesh-dataset-group-tree-item" ) );
  if ( !rootTreeItemElem.isNull() )
  {
    const QgsMeshDatasetGroupTreeItem groupTreeItem( rootTreeItemElem, context );
    setDatasetGroupTreeItem( &groupTreeItem );
  }
}

int QgsMeshDatasetGroupStore::globalDatasetGroupIndexInSource( QgsMeshDatasetSourceInterface *source, int nativeGroupIndex ) const
{
  for ( QMap<int, DatasetGroup>::const_iterator it = mRegistry.cbegin(); it != mRegistry.cend(); ++it )
  {
    if ( it.value().first == source && it.value().second == nativeGroupIndex )
      return it.key();
  }

  return -1;
}

int QgsMeshDatasetGroupStore::indexFromGroupName( const QString &groupName ) const
{
  return mGroupNameToGlobalIndex.value( groupName, -1 );
}

QString QgsMeshDatasetGroupStore::groupName( int groupIndex ) const
{
  return  datasetGroupMetadata( groupIndex ).name();
}

bool QgsMeshDatasetGroupStore::saveDatasetGroup( QString filePath, int groupIndex, QString driver )
{
  DatasetGroup group = datasetGroup( groupIndex );

  bool fail = true;
  if ( group.first && group.second >= 0 )
    fail = mPersistentProvider->persistDatasetGroup( filePath, driver, group.first, group.second );

  if ( !fail )
  {
    eraseDatasetGroup( group );
    group.first = mPersistentProvider;
    group.second = mPersistentProvider->datasetGroupCount() - 1;
    mRegistry[groupIndex] = group;
    //update the item type
    if ( mDatasetGroupTreeRootItem )
    {
      QgsMeshDatasetGroupTreeItem *item = mDatasetGroupTreeRootItem->childFromDatasetGroupIndex( groupIndex );
      if ( item )
        item->setPersistentDatasetGroup( filePath );
    }
  }

  return fail;
}

void QgsMeshDatasetGroupStore::onPersistentDatasetAdded( int count )
{
  Q_ASSERT( mPersistentProvider );

  int providerTotalCount = mPersistentProvider->datasetGroupCount();
  int providerBeginIndex = mPersistentProvider->datasetGroupCount() - count;
  QList<int> newGroupIndexes;
  for ( int i = providerBeginIndex; i < providerTotalCount; ++i )
  {
    const QString groupName = mPersistentProvider->datasetGroupMetadata( i ).name();
    if ( mGroupNameToGlobalIndex.empty() && i < mPersistentExtraDatasetGroupIndexes.count() )
    {
      // This happens with QGIS project saved with version < 3.28
      mRegistry[mPersistentExtraDatasetGroupIndexes.at( i )] = DatasetGroup( mPersistentProvider, i );
    }
    else if ( mGroupNameToGlobalIndex.contains( groupName ) )
    {
      // The dataset group name is associated with an index, so it is not a new dataset group
      registerDatasetGroup( DatasetGroup{mPersistentProvider, i} );
    }
    else
    {
      int newGroupIndex = registerDatasetGroup( DatasetGroup{mPersistentProvider, i} );
      if ( newGroupIndex != -1 )
        newGroupIndexes.append( newGroupIndex );
    }
  }

  if ( !newGroupIndexes.isEmpty() )
  {
    createDatasetGroupTreeItems( newGroupIndexes );
    mPersistentExtraDatasetGroupIndexes.append( newGroupIndexes );

    for ( int groupIndex : std::as_const( newGroupIndexes ) )
      syncItemToDatasetGroup( groupIndex );

    emit datasetGroupsAdded( newGroupIndexes );
  }
}

void QgsMeshDatasetGroupStore::removePersistentProvider()
{
  if ( !mPersistentProvider )
    return;

  disconnect( mPersistentProvider, &QgsMeshDataProvider::datasetGroupsAdded, this, &QgsMeshDatasetGroupStore::onPersistentDatasetAdded );

  QMap < int, DatasetGroup>::iterator it = mRegistry.begin();
  while ( it != mRegistry.end() )
  {
    if ( it.value().first == mPersistentProvider )
      it = mRegistry.erase( it );
    else
      ++it;
  }

  mPersistentProvider = nullptr;
}

int QgsMeshDatasetGroupStore::newIndex()
{
  QSet usedIndex = qgis::listToSet( mRegistry.keys() );
  usedIndex.unite( qgis::listToSet( mGroupNameToGlobalIndex.values() ) );
  int index = 0;

  while ( usedIndex.contains( index ) )
    ++index;

  return index;
}

int QgsMeshDatasetGroupStore::registerDatasetGroup( const QgsMeshDatasetGroupStore::DatasetGroup &group )
{
  const QString &name = group.first->datasetGroupMetadata( group.second ).name();
  auto it = mGroupNameToGlobalIndex.find( name );

  int groupIndex;
  if ( it != mGroupNameToGlobalIndex.end() )
  {
    groupIndex = it.value();

    if ( mRegistry.contains( groupIndex ) )
    {
      QgsDebugError( QStringLiteral( "Duplicate group name for %1." ).arg( name ) );
      return -1; // The registry has already a group with this index, we can't have two groups with the same name
    }
  }
  else
  {
    groupIndex = newIndex();
    mGroupNameToGlobalIndex.insert( name, groupIndex );
  }

  mRegistry[groupIndex] = group;
  return groupIndex;
}

void QgsMeshDatasetGroupStore::eraseDatasetGroup( const QgsMeshDatasetGroupStore::DatasetGroup &group )
{
  if ( group.first == mPersistentProvider )
    return; //removing persistent dataset group from the store is not allowed
  else if ( group.first == &mExtraDatasets )
    eraseExtraDataset( group.second );
}

void QgsMeshDatasetGroupStore::eraseExtraDataset( int indexInExtraStore )
{
  mExtraDatasets.removeDatasetGroup( indexInExtraStore );

  //search dataset with index greater than indexInExtraStore and decrement it
  QMap < int, DatasetGroup>::iterator it = mRegistry.begin();
  while ( it != mRegistry.end() )
  {
    int localIndex = it.value().second;
    if ( it.value().first == &mExtraDatasets && localIndex > indexInExtraStore )
      it->second = localIndex - 1;
    ++it;
  }
}

void QgsMeshDatasetGroupStore::checkDatasetConsistency( QgsMeshDatasetSourceInterface *source )
{
  // check if datasets of source are present, if not, add them
  QList<int> indexes;
  for ( int i = 0; i < source->datasetGroupCount(); ++i )
  {
    int globalIndex =  globalDatasetGroupIndexInSource( source, i );
    if ( globalIndex == -1 )
      globalIndex = registerDatasetGroup( DatasetGroup{source, i} );

    if ( globalIndex != - 1 )
      indexes.append( globalIndex );
  }

  if ( !indexes.isEmpty() )
    createDatasetGroupTreeItems( indexes );

  const QList<int> globalIndexes = mRegistry.keys();
  for ( int globalIndex :  globalIndexes )
  {
    if ( mRegistry.value( globalIndex ).first == source )
      syncItemToDatasetGroup( globalIndex );
  }
}

void QgsMeshDatasetGroupStore::removeUnregisteredItemFromTree()
{
  QList<QgsMeshDatasetGroupTreeItem *> itemsToCheck;
  QList<int> indexItemToRemove;
  for ( int i = 0; i < mDatasetGroupTreeRootItem->childCount(); ++i )
    itemsToCheck.append( mDatasetGroupTreeRootItem->child( i ) );

  while ( !itemsToCheck.isEmpty() )
  {
    QgsMeshDatasetGroupTreeItem *item = itemsToCheck.takeFirst();
    int globalIndex = item->datasetGroupIndex();
    if ( !mRegistry.contains( globalIndex ) )
      indexItemToRemove.append( globalIndex );
    for ( int i = 0; i < item->childCount(); ++i )
      itemsToCheck.append( item->child( i ) );
  }

  for ( int i : indexItemToRemove )
  {
    QgsMeshDatasetGroupTreeItem *item = mDatasetGroupTreeRootItem->childFromDatasetGroupIndex( i );
    if ( item )
      item->parentItem()->removeChild( item );
  }
}

void QgsMeshDatasetGroupStore::unregisterGroupNotPresentInTree()
{
  if ( !mDatasetGroupTreeRootItem )
  {
    mRegistry.clear();
    return;
  }

  QMap < int, DatasetGroup>::iterator it = mRegistry.begin();
  while ( it != mRegistry.end() )
  {
    DatasetGroup datasetGroup = it.value();
    int globalIndex = it.key();
    if ( ! mDatasetGroupTreeRootItem->childFromDatasetGroupIndex( globalIndex ) // Not in the tree item
         && datasetGroup.first != mPersistentProvider ) // and not persistent
    {
      it = mRegistry.erase( it ); //remove from registry
      eraseDatasetGroup( datasetGroup ); //remove from where the dataset group is stored
    }
    else
      ++it;
  }
}

void QgsMeshDatasetGroupStore::syncItemToDatasetGroup( int groupIndex )
{
  if ( !mDatasetGroupTreeRootItem )
    return;
  const DatasetGroup group = datasetGroup( groupIndex );
  QgsMeshDatasetGroupTreeItem *item = mDatasetGroupTreeRootItem->childFromDatasetGroupIndex( groupIndex );
  if ( group.first == mPersistentProvider && mPersistentProvider )
  {
    const QgsMeshDatasetGroupMetadata meta = mPersistentProvider->datasetGroupMetadata( group.second );
    if ( item )
      item->setPersistentDatasetGroup( meta.uri() );
  }
  else if ( group.first == &mExtraDatasets )
  {
    if ( item )
      item->setDatasetGroup( mExtraDatasets.datasetGroup( group.second ) );
  }
}

void QgsMeshDatasetGroupStore::createDatasetGroupTreeItems( const QList<int> &indexes )
{
  QMap<QString, QgsMeshDatasetGroupTreeItem *> mNameToItem;

  for ( int i = 0; i < indexes.count(); ++i )
  {
    int groupIndex = indexes.at( i );
    if ( mDatasetGroupTreeRootItem->childFromDatasetGroupIndex( groupIndex ) )
      continue; //item already exists
    const QgsMeshDatasetGroupMetadata meta = datasetGroupMetadata( groupIndex );
    const QString name = meta.name();
    const QStringList subdatasets = name.split( '/' );

    QString displayName = name;
    QgsMeshDatasetGroupTreeItem *parent = mDatasetGroupTreeRootItem.get();

    if ( subdatasets.size() == 2 )
    {
      auto it = mNameToItem.find( subdatasets[0] );
      if ( it == mNameToItem.end() )
        QgsDebugError( QStringLiteral( "Unable to find parent group for %1." ).arg( name ) );
      else
      {
        displayName = subdatasets[1];
        parent = it.value();
      }
    }
    else if ( subdatasets.size() != 1 )
      QgsDebugError( QStringLiteral( "Ignoring too deep child group name %1." ).arg( name ) );

    QgsMeshDatasetGroupTreeItem *item = new QgsMeshDatasetGroupTreeItem( displayName, name, meta.isVector(), groupIndex );
    parent->appendChild( item );
    if ( mNameToItem.contains( name ) )
      QgsDebugError( QStringLiteral( "Group %1 is not unique" ).arg( displayName ) );
    mNameToItem[name] = item;
  }
}

int QgsMeshExtraDatasetStore::addDatasetGroup( QgsMeshDatasetGroup *datasetGroup )
{
  int groupIndex = mGroups.size();
  mGroups.push_back( std::unique_ptr<QgsMeshDatasetGroup>( datasetGroup ) );

  if ( datasetGroup->datasetCount() > 1 )
  {
    mTemporalCapabilities->setHasTemporalCapabilities( true );
    for ( int i = 0; i < datasetGroup->datasetCount(); ++i )
      mTemporalCapabilities->addDatasetTime( groupIndex, datasetGroup->datasetMetadata( i ).time() );
  }

  return mGroups.size() - 1;
}

void QgsMeshExtraDatasetStore::removeDatasetGroup( int index )
{
  if ( index < datasetGroupCount() )
    mGroups.erase( mGroups.begin() + index );


  updateTemporalCapabilities();
}

bool QgsMeshExtraDatasetStore::hasTemporalCapabilities() const
{
  return mTemporalCapabilities->hasTemporalCapabilities();
}

qint64 QgsMeshExtraDatasetStore::datasetRelativeTime( QgsMeshDatasetIndex index ) const
{
  return mTemporalCapabilities->datasetTime( index );
}

QString QgsMeshExtraDatasetStore::description( int groupIndex ) const
{
  if ( groupIndex >= 0 && groupIndex < int( mGroups.size() ) )
    return mGroups.at( groupIndex )->description();
  else
    return QString();
}

QgsMeshDatasetGroup *QgsMeshExtraDatasetStore::datasetGroup( int groupIndex ) const
{
  if ( groupIndex >= 0 && groupIndex < int( mGroups.size() ) )
    return mGroups[groupIndex].get();
  else
    return nullptr;
}

bool QgsMeshExtraDatasetStore::addDataset( const QString &uri )
{
  Q_UNUSED( uri );
  return false;
}

QStringList QgsMeshExtraDatasetStore::extraDatasets() const
{
  return QStringList();
}

int QgsMeshExtraDatasetStore::datasetGroupCount() const
{
  return mGroups.size();
}

int QgsMeshExtraDatasetStore::datasetCount( int groupIndex ) const
{
  if ( groupIndex >= 0 && groupIndex < datasetGroupCount() )
    return mGroups.at( groupIndex )->datasetCount();
  else
    return 0;
}

QgsMeshDatasetGroupMetadata QgsMeshExtraDatasetStore::datasetGroupMetadata( int groupIndex ) const
{
  if ( groupIndex >= 0 && groupIndex < datasetGroupCount() )
    return mGroups.at( groupIndex )->groupMetadata();
  else
    return QgsMeshDatasetGroupMetadata();
}

QgsMeshDatasetMetadata QgsMeshExtraDatasetStore::datasetMetadata( QgsMeshDatasetIndex index ) const
{
  int groupIndex = index.group();
  if ( index.isValid() && groupIndex < datasetGroupCount() )
  {
    int datasetIndex = index.dataset();
    const QgsMeshDatasetGroup *group = mGroups.at( groupIndex ).get();
    if ( datasetIndex < group->datasetCount() )
      return group->datasetMetadata( datasetIndex );
  }
  return QgsMeshDatasetMetadata();
}

QgsMeshDatasetValue QgsMeshExtraDatasetStore::datasetValue( QgsMeshDatasetIndex index, int valueIndex ) const
{
  int groupIndex = index.group();
  if ( index.isValid() && groupIndex < datasetGroupCount() )
  {
    const QgsMeshDatasetGroup *group = mGroups.at( groupIndex ).get();
    int datasetIndex = index.dataset();
    if ( datasetIndex < group->datasetCount() )
      return group->dataset( datasetIndex )->datasetValue( valueIndex );
  }

  return QgsMeshDatasetValue();
}

QgsMeshDataBlock QgsMeshExtraDatasetStore::datasetValues( QgsMeshDatasetIndex index, int valueIndex, int count ) const
{
  int groupIndex = index.group();
  if ( index.isValid() && groupIndex < datasetGroupCount() )
  {
    const QgsMeshDatasetGroup *group = mGroups.at( groupIndex ).get();
    int datasetIndex = index.dataset();
    if ( datasetIndex < group->datasetCount() )
      return group->dataset( datasetIndex )->datasetValues( group->isScalar(), valueIndex, count );
  }

  return QgsMeshDataBlock();
}

QgsMesh3DDataBlock QgsMeshExtraDatasetStore::dataset3dValues( QgsMeshDatasetIndex index, int faceIndex, int count ) const
{
  // Not supported for now
  Q_UNUSED( index )
  Q_UNUSED( faceIndex )
  Q_UNUSED( count )
  return QgsMesh3DDataBlock();
}

bool QgsMeshExtraDatasetStore::isFaceActive( QgsMeshDatasetIndex index, int faceIndex ) const
{
  int groupIndex = index.group();
  if ( index.isValid() && groupIndex < datasetGroupCount() )
  {
    const QgsMeshDatasetGroup *group = mGroups.at( groupIndex ).get();
    int datasetIndex = index.dataset();
    if ( datasetIndex < group->datasetCount() )
      return group->dataset( datasetIndex )->isActive( faceIndex );
  }

  return false;
}

QgsMeshDataBlock QgsMeshExtraDatasetStore::areFacesActive( QgsMeshDatasetIndex index, int faceIndex, int count ) const
{
  int groupIndex = index.group();
  if ( index.isValid() && groupIndex < datasetGroupCount() )
  {
    const QgsMeshDatasetGroup *group = mGroups.at( groupIndex ).get();
    int datasetIndex = index.dataset();
    if ( datasetIndex < group->datasetCount() )
      return group->dataset( datasetIndex )->areFacesActive( faceIndex, count );
  }
  return QgsMeshDataBlock();
}

bool QgsMeshExtraDatasetStore::persistDatasetGroup( const QString &outputFilePath,
    const QString &outputDriver,
    const QgsMeshDatasetGroupMetadata &meta,
    const QVector<QgsMeshDataBlock> &datasetValues,
    const QVector<QgsMeshDataBlock> &datasetActive,
    const QVector<double> &times )
{
  Q_UNUSED( outputFilePath )
  Q_UNUSED( outputDriver )
  Q_UNUSED( meta )
  Q_UNUSED( datasetValues )
  Q_UNUSED( datasetActive )
  Q_UNUSED( times )
  return true; // not implemented/supported
}

bool QgsMeshExtraDatasetStore::persistDatasetGroup( const QString &outputFilePath,
    const QString &outputDriver,
    QgsMeshDatasetSourceInterface *source,
    int datasetGroupIndex )
{
  Q_UNUSED( outputFilePath )
  Q_UNUSED( outputDriver )
  Q_UNUSED( source )
  Q_UNUSED( datasetGroupIndex )
  return true; // not implemented/supported
}

QDomElement QgsMeshExtraDatasetStore::writeXml( int groupIndex, QDomDocument &doc, const QgsReadWriteContext &context )
{
  if ( groupIndex >= 0 && groupIndex < int( mGroups.size() ) && mGroups[groupIndex] )
    return mGroups[groupIndex]->writeXml( doc, context );
  else
    return QDomElement();
}

void QgsMeshExtraDatasetStore::updateTemporalCapabilities()
{
  //update temporal capabilitie
  mTemporalCapabilities->clear();
  bool hasTemporal = false;
  for ( size_t g = 0; g < mGroups.size(); ++g )
  {
    const QgsMeshDatasetGroup *group = mGroups[g].get();
    hasTemporal |= group->datasetCount() > 1;
    for ( int i = 0; i < group->datasetCount(); ++i )
      mTemporalCapabilities->addDatasetTime( g, group->datasetMetadata( i ).time() );
  }

  mTemporalCapabilities->setHasTemporalCapabilities( hasTemporal );
}
