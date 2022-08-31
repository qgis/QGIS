/***************************************************************************
  qgsvectorlayercache.cpp
  Cache features of a vector layer
  -------------------
         begin                : January 2013
         copyright            : (C) Matthias Kuhn
         email                : matthias at opengis dot ch

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorlayercache.h"
#include "qgscacheindex.h"
#include "qgscachedfeatureiterator.h"
#include "qgsvectorlayerjoininfo.h"
#include "qgsvectorlayerjoinbuffer.h"
#include "qgsvectorlayer.h"

#include <QElapsedTimer>

QgsVectorLayerCache::QgsVectorLayerCache( QgsVectorLayer *layer, int cacheSize, QObject *parent )
  : QObject( parent )
  , mLayer( layer )
{
  mCache.setMaxCost( cacheSize );

  connect( mLayer, &QgsVectorLayer::featureDeleted, this, &QgsVectorLayerCache::featureDeleted );
  connect( mLayer, &QgsVectorLayer::featureAdded, this, &QgsVectorLayerCache::onFeatureAdded );
  connect( mLayer, &QgsVectorLayer::destroyed, this, &QgsVectorLayerCache::layerDeleted );

  setCacheGeometry( true );
  setCacheSubsetOfAttributes( mLayer->attributeList() );
  setCacheAddedAttributes( true );

  connect( mLayer, &QgsVectorLayer::attributeDeleted, this, &QgsVectorLayerCache::attributeDeleted );
  connect( mLayer, &QgsVectorLayer::updatedFields, this, &QgsVectorLayerCache::invalidate );
  connect( mLayer, &QgsVectorLayer::dataChanged, this, &QgsVectorLayerCache::invalidate );
  connect( mLayer, &QgsVectorLayer::attributeValueChanged, this, &QgsVectorLayerCache::onAttributeValueChanged );

  connectJoinedLayers();
}

QgsVectorLayerCache::~QgsVectorLayerCache()
{
  qDeleteAll( mCacheIndices );
  mCacheIndices.clear();
}

void QgsVectorLayerCache::setCacheSize( int cacheSize )
{
  mCache.setMaxCost( cacheSize );
}

int QgsVectorLayerCache::cacheSize()
{
  return mCache.maxCost();
}

void QgsVectorLayerCache::setCacheGeometry( bool cacheGeometry )
{
  bool shouldCacheGeometry = cacheGeometry && mLayer->isSpatial();
  bool mustInvalidate = shouldCacheGeometry && !mCacheGeometry; // going from no geometry -> geometry, so have to clear existing cache entries
  mCacheGeometry = shouldCacheGeometry;
  if ( cacheGeometry )
  {
    connect( mLayer, &QgsVectorLayer::geometryChanged, this, &QgsVectorLayerCache::geometryChanged, Qt::UniqueConnection );
  }
  else
  {
    disconnect( mLayer, &QgsVectorLayer::geometryChanged, this, &QgsVectorLayerCache::geometryChanged );
  }
  if ( mustInvalidate )
  {
    invalidate();
  }
}

void QgsVectorLayerCache::setCacheSubsetOfAttributes( const QgsAttributeList &attributes )
{
  mCachedAttributes = attributes;
}

void QgsVectorLayerCache::setFullCache( bool fullCache )
{
  mFullCache = fullCache;

  if ( mFullCache )
  {
    // Add a little more than necessary...
    setCacheSize( mLayer->featureCount() + 100 );

    // Initialize the cache...
    QgsFeatureIterator it( new QgsCachedFeatureWriterIterator( this, QgsFeatureRequest()
                           .setSubsetOfAttributes( mCachedAttributes )
                           .setFlags( mCacheGeometry ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry ) ) );

    int i = 0;

    QElapsedTimer t;
    t.start();

    QgsFeature f;
    while ( it.nextFeature( f ) )
    {
      ++i;

      if ( t.elapsed() > 1000 )
      {
        bool cancel = false;
        emit progress( i, cancel );
        if ( cancel )
          break;

        t.restart();
      }
    }

    it.close();

    emit finished();
  }
}

void QgsVectorLayerCache::addCacheIndex( QgsAbstractCacheIndex *cacheIndex )
{
  mCacheIndices.append( cacheIndex );
}

void QgsVectorLayerCache::setCacheAddedAttributes( bool cacheAddedAttributes )
{
  if ( cacheAddedAttributes )
  {
    connect( mLayer, &QgsVectorLayer::attributeAdded, this, &QgsVectorLayerCache::attributeAdded );
  }
  else
  {
    disconnect( mLayer, &QgsVectorLayer::attributeAdded, this, &QgsVectorLayerCache::attributeAdded );
  }
}

bool QgsVectorLayerCache::featureAtId( QgsFeatureId featureId, QgsFeature &feature, bool skipCache )
{
  bool featureFound = false;

  QgsCachedFeature *cachedFeature = nullptr;

  if ( !skipCache )
  {
    cachedFeature = mCache[ featureId ];
  }

  if ( cachedFeature )
  {
    feature = QgsFeature( *cachedFeature->feature() );
    featureFound = true;
  }
  else if ( mLayer->getFeatures( QgsFeatureRequest()
                                 .setFilterFid( featureId )
                                 .setSubsetOfAttributes( mCachedAttributes )
                                 .setFlags( !mCacheGeometry ? QgsFeatureRequest::NoGeometry : QgsFeatureRequest::Flags() ) )
            .nextFeature( feature ) )
  {
    cacheFeature( feature );
    featureFound = true;
  }

  return featureFound;
}

bool QgsVectorLayerCache::removeCachedFeature( QgsFeatureId fid )
{
  bool removed = mCache.remove( fid );
  if ( removed )
  {
    if ( auto unorderedIt = std::find( mCacheUnorderedKeys.begin(), mCacheUnorderedKeys.end(), fid ); unorderedIt != mCacheUnorderedKeys.end() )
    {
      mCacheUnorderedKeys.erase( unorderedIt );

      if ( auto orderedIt = std::find( mCacheOrderedKeys.begin(), mCacheOrderedKeys.end(), fid ); orderedIt != mCacheOrderedKeys.end() )
        mCacheOrderedKeys.erase( orderedIt );
    }
  }
  return removed;
}

QgsVectorLayer *QgsVectorLayerCache::layer()
{
  return mLayer;
}

QgsCoordinateReferenceSystem QgsVectorLayerCache::sourceCrs() const
{
  return mLayer->crs();
}

QgsWkbTypes::Type QgsVectorLayerCache::wkbType() const
{
  return mLayer->wkbType();
}

QgsFields QgsVectorLayerCache::fields() const
{
  return mLayer->fields();
}

long long QgsVectorLayerCache::featureCount() const
{
  return mLayer->featureCount();
}

void QgsVectorLayerCache::requestCompleted( const QgsFeatureRequest &featureRequest, const QgsFeatureIds &fids )
{
  // If a request is too large for the cache don't notify to prevent from indexing incomplete requests
  if ( fids.count() <= mCache.size() )
  {
    for ( const auto &idx : std::as_const( mCacheIndices ) )
    {
      idx->requestCompleted( featureRequest, fids );
    }
    if ( featureRequest.filterType() == QgsFeatureRequest::FilterNone &&
         ( featureRequest.spatialFilterType() == Qgis::SpatialFilterType::NoFilter || featureRequest.filterRect().contains( mLayer->extent() ) ) )
    {
      mFullCache = true;
    }
  }
}

void QgsVectorLayerCache::featureRemoved( QgsFeatureId fid )
{
  const auto constMCacheIndices = mCacheIndices;
  for ( QgsAbstractCacheIndex *idx : constMCacheIndices )
  {
    idx->flushFeature( fid );
  }
}

void QgsVectorLayerCache::onAttributeValueChanged( QgsFeatureId fid, int field, const QVariant &value )
{
  QgsCachedFeature *cachedFeat = mCache[ fid ];

  if ( cachedFeat )
  {
    cachedFeat->mFeature->setAttribute( field, value );
  }

  emit attributeValueChanged( fid, field, value );
}

void QgsVectorLayerCache::onJoinAttributeValueChanged( QgsFeatureId fid, int field, const QVariant &value )
{
  const QgsVectorLayer *joinLayer = qobject_cast<const QgsVectorLayer *>( sender() );

  const auto constVectorJoins = mLayer->vectorJoins();
  for ( const QgsVectorLayerJoinInfo &info : constVectorJoins )
  {
    if ( joinLayer == info.joinLayer() )
    {
      const QgsFeature feature = mLayer->joinBuffer()->targetedFeatureOf( &info, joinLayer->getFeature( fid ) );

      const QString fieldName = info.prefixedFieldName( joinLayer->fields().field( field ) );
      const int fieldIndex = mLayer->fields().indexFromName( fieldName );

      if ( feature.isValid() && fieldIndex != -1 )
      {
        onAttributeValueChanged( feature.id(), fieldIndex, value );
        return;
      }
    }
  }
}

void QgsVectorLayerCache::featureDeleted( QgsFeatureId fid )
{
  mCache.remove( fid );

  if ( auto it = mCacheUnorderedKeys.find( fid ); it != mCacheUnorderedKeys.end() )
  {
    mCacheUnorderedKeys.erase( it );
    if ( auto orderedIt = std::find( mCacheOrderedKeys.begin(), mCacheOrderedKeys.end(), fid ); orderedIt != mCacheOrderedKeys.end() )
      mCacheOrderedKeys.erase( orderedIt );
  }
}

void QgsVectorLayerCache::onFeatureAdded( QgsFeatureId fid )
{
  if ( mFullCache )
  {
    if ( cacheSize() <= mLayer->featureCount() )
    {
      setCacheSize( mLayer->featureCount() + 100 );
    }

    QgsFeature feat;
    featureAtId( fid, feat );
  }
  emit featureAdded( fid );
}

void QgsVectorLayerCache::attributeAdded( int field )
{
  Q_UNUSED( field )
  mCachedAttributes.append( field );
  invalidate();
}

void QgsVectorLayerCache::attributeDeleted( int field )
{
  QgsAttributeList attrs = mCachedAttributes;
  mCachedAttributes.clear();

  const auto constAttrs = attrs;
  for ( int attr : constAttrs )
  {
    if ( attr < field )
      mCachedAttributes << attr;
    else if ( attr > field )
      mCachedAttributes << attr - 1;
  }
}

void QgsVectorLayerCache::geometryChanged( QgsFeatureId fid, const QgsGeometry &geom )
{
  QgsCachedFeature *cachedFeat = mCache[ fid ];

  if ( cachedFeat )
  {
    cachedFeat->mFeature->setGeometry( geom );
  }
}

void QgsVectorLayerCache::layerDeleted()
{
  emit cachedLayerDeleted();
  mLayer = nullptr;
}

void QgsVectorLayerCache::invalidate()
{
  mCache.clear();
  mCacheOrderedKeys.clear();
  mCacheUnorderedKeys.clear();
  mFullCache = false;
  emit invalidated();
}

bool QgsVectorLayerCache::canUseCacheForRequest( const QgsFeatureRequest &featureRequest, QgsFeatureIterator &it )
{
  // check first for available indices
  const auto constMCacheIndices = mCacheIndices;
  for ( QgsAbstractCacheIndex *idx : constMCacheIndices )
  {
    if ( idx->getCacheIterator( it, featureRequest ) )
    {
      return true;
    }
  }

  // no indexes available, but maybe we have already cached all required features anyway?
  switch ( featureRequest.filterType() )
  {
    case QgsFeatureRequest::FilterFid:
    {
      if ( mCache.contains( featureRequest.filterFid() ) )
      {
        it = QgsFeatureIterator( new QgsCachedFeatureIterator( this, featureRequest ) );
        return true;
      }
      break;
    }
    case QgsFeatureRequest::FilterFids:
    {
      if ( cachedFeatureIds().contains( featureRequest.filterFids() ) )
      {
        it = QgsFeatureIterator( new QgsCachedFeatureIterator( this, featureRequest ) );
        return true;
      }
      break;
    }
    case QgsFeatureRequest::FilterNone:
    case QgsFeatureRequest::FilterExpression:
    {
      if ( mFullCache )
      {
        it = QgsFeatureIterator( new QgsCachedFeatureIterator( this, featureRequest ) );
        return true;
      }
      break;
    }

  }
  return false;
}

QgsFeatureIterator QgsVectorLayerCache::getFeatures( const QgsFeatureRequest &featureRequest )
{
  QgsFeatureIterator it;
  bool requiresWriterIt = true; // If a not yet cached, but cacheable request is made, this stays true.

  if ( checkInformationCovered( featureRequest ) )
  {
    // If we have a full cache available, run on this
    if ( mFullCache )
    {
      it = QgsFeatureIterator( new QgsCachedFeatureIterator( this, featureRequest ) );
      requiresWriterIt = false;
    }
    else
    {
      // may still be able to satisfy request using cache
      requiresWriterIt = !canUseCacheForRequest( featureRequest, it );
    }
  }
  else
  {
    // Let the layer answer the request, so no caching of requests
    // we don't want to cache is done
    requiresWriterIt = false;
    it = mLayer->getFeatures( featureRequest );
  }

  if ( requiresWriterIt && mLayer->dataProvider() )
  {
    // No index was able to satisfy the request
    QgsFeatureRequest myRequest = QgsFeatureRequest( featureRequest );

    // Make sure if we cache the geometry, it gets fetched
    if ( mCacheGeometry && mLayer->isSpatial() )
      myRequest.setFlags( featureRequest.flags() & ~QgsFeatureRequest::NoGeometry );

    // Make sure, all the cached attributes are requested as well
    const QgsAttributeList requestSubset = featureRequest.subsetOfAttributes();
    QSet<int> attrs( requestSubset.begin(), requestSubset.end() );
    for ( int attr : std::as_const( mCachedAttributes ) )
      attrs.insert( attr );
    myRequest.setSubsetOfAttributes( QgsAttributeList( attrs.begin(), attrs.end() ) );

    it = QgsFeatureIterator( new QgsCachedFeatureWriterIterator( this, myRequest ) );
  }

  return it;
}

bool QgsVectorLayerCache::isFidCached( const QgsFeatureId fid ) const
{
  return mCache.contains( fid );
}

QgsFeatureIds QgsVectorLayerCache::cachedFeatureIds() const
{
  const QList< QgsFeatureId > keys = mCache.keys();
  return QgsFeatureIds( keys.begin(), keys.end() );
}

bool QgsVectorLayerCache::checkInformationCovered( const QgsFeatureRequest &featureRequest )
{
  QgsAttributeList requestedAttributes;

  if ( !featureRequest.flags().testFlag( QgsFeatureRequest::SubsetOfAttributes ) )
  {
    requestedAttributes = mLayer->attributeList();
  }
  else
  {
    requestedAttributes = featureRequest.subsetOfAttributes();
  }

  // Check if we even cache the information requested
  const auto constRequestedAttributes = requestedAttributes;
  for ( int attr : constRequestedAttributes )
  {
    if ( !mCachedAttributes.contains( attr ) )
    {
      return false;
    }
  }

  // If the request needs geometry but we don't cache this...
  return !( !featureRequest.flags().testFlag( QgsFeatureRequest::NoGeometry )
            && !mCacheGeometry );
}

void QgsVectorLayerCache::connectJoinedLayers() const
{
  const auto constVectorJoins = mLayer->vectorJoins();
  for ( const QgsVectorLayerJoinInfo &info : constVectorJoins )
  {
    const QgsVectorLayer *vl = info.joinLayer();
    if ( vl )
      connect( vl, &QgsVectorLayer::attributeValueChanged, this, &QgsVectorLayerCache::onJoinAttributeValueChanged );
  }
}
