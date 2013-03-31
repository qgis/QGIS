/***************************************************************************
  qgsvectorlayercache.cpp
  Cache features of a vector layer
  -------------------
         begin                : January 2013
         copyright            : (C) Matthias Kuhn
         email                : matthias dot kuhn at gmx dot ch

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

QgsVectorLayerCache::QgsVectorLayerCache( QgsVectorLayer* layer, int cacheSize, QObject* parent )
    : QObject( parent )
    , mLayer( layer )
{
  mCache.setMaxCost( cacheSize );

  connect( mLayer, SIGNAL( featureDeleted( QgsFeatureId ) ), SLOT( featureDeleted( QgsFeatureId ) ) );
  connect( mLayer, SIGNAL( featureAdded( QgsFeatureId ) ), SLOT( featureAdded( QgsFeatureId ) ) );
  connect( mLayer, SIGNAL( layerDeleted() ), SLOT( layerDeleted() ) );

  setCacheGeometry( true );
  setCacheSubsetOfAttributes( mLayer->pendingAllAttributesList() );
  setCacheAddedAttributes( true );

  connect( mLayer, SIGNAL( attributeDeleted( int ) ), SLOT( attributeDeleted( int ) ) );
  connect( mLayer, SIGNAL( attributeValueChanged( QgsFeatureId, int, const QVariant& ) ), SLOT( attributeValueChanged( QgsFeatureId, int, const QVariant& ) ) );
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
  mCacheGeometry = cacheGeometry;
  if ( cacheGeometry )
  {
    connect( mLayer, SIGNAL( geometryChanged( QgsFeatureId, QgsGeometry& ) ), SLOT( geometryChanged( QgsFeatureId, QgsGeometry& ) ) );
  }
  else
  {
    disconnect( mLayer, SIGNAL( geometryChanged( QgsFeatureId, QgsGeometry& ) ), this, SLOT( geometryChanged( QgsFeatureId, QgsGeometry& ) ) );
  }
}

void QgsVectorLayerCache::setCacheSubsetOfAttributes( const QgsAttributeList& attributes )
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
    QgsFeatureIterator it = getFeatures( QgsFeatureRequest()
                                         .setSubsetOfAttributes( mCachedAttributes )
                                         .setFlags( !mCacheGeometry ? QgsFeatureRequest::NoGeometry : QgsFeatureRequest::Flags( 0 ) ) );

    int i = 0;

    QTime t;
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
  }
}

void QgsVectorLayerCache::setCacheAddedAttributes( bool cacheAddedAttributes )
{
  if ( cacheAddedAttributes )
  {
    connect( mLayer, SIGNAL( attributeAdded( int ) ), SLOT( attributeAdded( int ) ) );
  }
  else
  {
    disconnect( mLayer, SIGNAL( attributeAdded( int ) ), this, SLOT( attributeAdded( int ) ) );
  }
}

bool QgsVectorLayerCache::featureAtId( QgsFeatureId featureId, QgsFeature& feature, bool skipCache )
{
  bool featureFound = false;

  QgsCachedFeature* cachedFeature = NULL;

  if ( !skipCache )
  {
    cachedFeature = mCache[ featureId ];
  }

  if ( cachedFeature != NULL )
  {
    feature = QgsFeature( *cachedFeature->feature() );
    featureFound = true;
  }
  else if ( mLayer->getFeatures( QgsFeatureRequest()
                                 .setFilterFid( featureId )
                                 .setSubsetOfAttributes( mCachedAttributes )
                                 .setFlags( !mCacheGeometry ? QgsFeatureRequest::NoGeometry : QgsFeatureRequest::Flags( 0 ) ) )
            .nextFeature( feature ) )
  {
    cacheFeature( feature );
    featureFound = true;
  }

  return featureFound;
}

bool QgsVectorLayerCache::removeCachedFeature( QgsFeatureId fid )
{
  return mCache.remove( fid );
}

QgsVectorLayer* QgsVectorLayerCache::layer()
{
  return mLayer;
}

void QgsVectorLayerCache::requestCompleted( QgsFeatureRequest featureRequest, QgsFeatureIds fids )
{
  // If a request is too large for the cache don't notify to prevent from indexing incomplete requests
  if ( fids.count() < mCache.size() )
  {
    foreach ( QgsAbstractCacheIndex* idx, mCacheIndices )
    {
      idx->requestCompleted( featureRequest, fids );
    }
  }
}

void QgsVectorLayerCache::featureRemoved( QgsFeatureId fid )
{
  foreach ( QgsAbstractCacheIndex* idx, mCacheIndices )
  {
    idx->flushFeature( fid );
  }
}

void QgsVectorLayerCache::attributeValueChanged( QgsFeatureId fid, int field, const QVariant& value )
{
  QgsCachedFeature* cachedFeat = mCache[ fid ];

  if ( NULL != cachedFeat )
  {
    cachedFeat->mFeature->setAttribute( field, value );
  }
}

void QgsVectorLayerCache::featureDeleted( QgsFeatureId fid )
{
  mCache.remove( fid );
}

void QgsVectorLayerCache::featureAdded( QgsFeatureId fid )
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
}

void QgsVectorLayerCache::attributeAdded( int field )
{
  Q_UNUSED( field )
  mCachedAttributes.append( field );
  mCache.clear();
}

void QgsVectorLayerCache::attributeDeleted( int field )
{
  foreach ( QgsFeatureId fid, mCache.keys() )
  {
    mCache[ fid ]->mFeature->deleteAttribute( field );
  }
}

void QgsVectorLayerCache::geometryChanged( QgsFeatureId fid, QgsGeometry& geom )
{
  QgsCachedFeature* cachedFeat = mCache[ fid ];

  if ( cachedFeat != NULL )
  {
    cachedFeat->mFeature->setGeometry( geom );
  }
}

void QgsVectorLayerCache::layerDeleted()
{
  emit( cachedLayerDeleted() );

  mLayer = NULL;
}

QgsFeatureIterator QgsVectorLayerCache::getFeatures( const QgsFeatureRequest &featureRequest )
{
  QgsFeatureIterator it;
  bool requiresWriterIt = true; // If a not yet cached, but cachable request is made, this stays true.

  if ( checkInformationCovered( featureRequest ) )
  {
    // Check if an index is able to deliver the requested features
    foreach ( QgsAbstractCacheIndex *idx, mCacheIndices )
    {
      QgsFeatureIds featureIds;

      if ( idx->getCachedIds( featureIds, featureRequest ) )
      {
        it = QgsFeatureIterator( new QgsCachedFeatureIterator( this, featureRequest, featureIds ) );
        requiresWriterIt = false;
        break;
      }
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
    it = QgsFeatureIterator( new QgsCachedFeatureWriterIterator( this, featureRequest ) );
  }

  return it;
}

bool QgsVectorLayerCache::checkInformationCovered( const QgsFeatureRequest& featureRequest )
{
  QgsAttributeList requestedAttributes;

  if ( false == featureRequest.flags().testFlag( QgsFeatureRequest::SubsetOfAttributes ) )
  {
    requestedAttributes = mLayer->pendingAllAttributesList();
  }
  else
  {
    requestedAttributes = featureRequest.subsetOfAttributes();
  }

  // Check if we even cache the information requested
  foreach ( int attr, requestedAttributes )
  {
    if ( !mCachedAttributes.contains( attr ) )
    {
      return false;
    }
  }

  // If the request needs geometry but we don't cache this...
  if ( false == featureRequest.flags().testFlag( QgsFeatureRequest::NoGeometry )
       && false == mCacheGeometry )
  {
    return false;
  }

  return true;
}
