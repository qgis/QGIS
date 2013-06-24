/***************************************************************************
    qgsvectorlayerfeatureiterator.cpp
    ---------------------
    begin                : Dezember 2012
    copyright            : (C) 2012 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsvectorlayerfeatureiterator.h"

#include "qgsmaplayerregistry.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayereditbuffer.h"
#include "qgsvectorlayerjoinbuffer.h"

QgsVectorLayerFeatureIterator::QgsVectorLayerFeatureIterator( QgsVectorLayer* layer, const QgsFeatureRequest& request )
    : QgsAbstractFeatureIterator( request ), L( layer )
{
  QgsVectorLayerJoinBuffer* joinBuffer = L->mJoinBuffer;

  if ( L->editBuffer() )
  {
    mAddedFeatures = QgsFeatureMap( L->editBuffer()->addedFeatures() );
    mChangedGeometries = QgsGeometryMap( L->editBuffer()->changedGeometries() );
    mDeletedFeatureIds = QgsFeatureIds( L->editBuffer()->deletedFeatureIds() );
    mChangedAttributeValues = QgsChangedAttributesMap( L->editBuffer()->changedAttributeValues() );
    mAddedAttributes = QList<QgsField>( L->editBuffer()->addedAttributes() );
    mDeletedAttributeIds = QgsAttributeList( L->editBuffer()->deletedAttributeIds() );
  }

  // prepare joins: may add more attributes to fetch (in order to allow join)
  if ( joinBuffer->containsJoins() )
    prepareJoins();

  // by default provider's request is the same
  mProviderRequest = mRequest;

  if ( mProviderRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
  {
    // prepare list of attributes to match provider fields
    QgsAttributeList providerSubset;
    QgsAttributeList subset = mProviderRequest.subsetOfAttributes();
    const QgsFields &pendingFields = L->pendingFields();
    int nPendingFields = pendingFields.count();
    for ( int i = 0; i < subset.count(); ++i )
    {
      int attrIndex = subset[i];
      if ( attrIndex < 0 || attrIndex >= nPendingFields ) continue;
      if ( L->pendingFields().fieldOrigin( attrIndex ) == QgsFields::OriginProvider )
        providerSubset << L->pendingFields().fieldOriginIndex( attrIndex );
    }
    mProviderRequest.setSubsetOfAttributes( providerSubset );
  }

  if ( request.filterType() == QgsFeatureRequest::FilterFid )
  {
    mFetchedFid = false;
  }
  else // no filter or filter by rect
  {
    mProviderIterator = L->dataProvider()->getFeatures( mProviderRequest );

    rewindEditBuffer();
  }
}


QgsVectorLayerFeatureIterator::~QgsVectorLayerFeatureIterator()
{
  close();
}



bool QgsVectorLayerFeatureIterator::nextFeature( QgsFeature& f )
{
  f.setValid( false );

  if ( mClosed )
    return false;

  if ( mRequest.filterType() == QgsFeatureRequest::FilterFid )
  {
    if ( mFetchedFid )
      return false;
    bool res = nextFeatureFid( f );
    mFetchedFid = true;
    return res;
  }

  if ( mRequest.filterType() == QgsFeatureRequest::FilterRect )
  {
    if ( fetchNextChangedGeomFeature( f ) )
      return true;

    // no more changed geometries
  }

  if ( fetchNextAddedFeature( f ) )
    return true;

  // no more added features

  while ( mProviderIterator.nextFeature( f ) )
  {
    if ( mFetchConsidered.contains( f.id() ) )
      continue;

    // TODO[MD]: just one resize of attributes
    f.setFields( &L->mUpdatedFields );

    // update attributes
    updateChangedAttributes( f );

    if ( !mFetchJoinInfo.isEmpty() )
      addJoinedAttributes( f );

    // update geometry
    if ( !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) )
      updateFeatureGeometry( f );

    return true;
  }

  close();
  return false;
}



bool QgsVectorLayerFeatureIterator::rewind()
{
  if ( mClosed )
    return false;

  if ( mRequest.filterType() == QgsFeatureRequest::FilterFid )
  {
    mFetchedFid = false;
  }
  else
  {
    mProviderIterator.rewind();
    rewindEditBuffer();
  }

  return true;
}

bool QgsVectorLayerFeatureIterator::close()
{
  if ( mClosed )
    return false;

  mProviderIterator.close();

  mClosed = true;
  return true;
}




bool QgsVectorLayerFeatureIterator::fetchNextAddedFeature( QgsFeature& f )
{
  for ( ; mFetchAddedFeaturesIt != mAddedFeatures.constEnd(); mFetchAddedFeaturesIt++ )
  {
    QgsFeatureId fid = mFetchAddedFeaturesIt->id();

    if ( mFetchConsidered.contains( fid ) )
      // must have changed geometry outside rectangle
      continue;

    if ( mRequest.filterType() == QgsFeatureRequest::FilterRect &&
         mFetchAddedFeaturesIt->geometry() &&
         !mFetchAddedFeaturesIt->geometry()->intersects( mRequest.filterRect() ) )
      // skip added features not in rectangle
      continue;

    useAddedFeature( *mFetchAddedFeaturesIt, f );

    mFetchAddedFeaturesIt++;
    return true;
  }

  return false; // no more added features
}


void QgsVectorLayerFeatureIterator::useAddedFeature( const QgsFeature& src, QgsFeature& f )
{
  f.setFeatureId( src.id() );
  f.setValid( true );
  f.setFields( &L->mUpdatedFields );

  if ( src.geometry() && !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) )
    f.setGeometry( *src.geometry() );

  // TODO[MD]: if subset set just some attributes

  f.setAttributes( src.attributes() );

  if ( !mFetchJoinInfo.isEmpty() )
    addJoinedAttributes( f );
}



bool QgsVectorLayerFeatureIterator::fetchNextChangedGeomFeature( QgsFeature& f )
{
  // check if changed geometries are in rectangle
  for ( ; mFetchChangedGeomIt != mChangedGeometries.constEnd(); mFetchChangedGeomIt++ )
  {
    QgsFeatureId fid = mFetchChangedGeomIt.key();

    if ( mFetchConsidered.contains( fid ) )
      // skip deleted features
      continue;

    mFetchConsidered << fid;

    if ( !mFetchChangedGeomIt->intersects( mRequest.filterRect() ) )
      // skip changed geometries not in rectangle and don't check again
      continue;

    useChangedAttributeFeature( fid, *mFetchChangedGeomIt, f );

    // return complete feature
    mFetchChangedGeomIt++;
    return true;
  }

  return false; // no more changed geometries
}


void QgsVectorLayerFeatureIterator::useChangedAttributeFeature( QgsFeatureId fid, const QgsGeometry& geom, QgsFeature& f )
{
  f.setFeatureId( fid );
  f.setValid( true );
  f.setFields( &L->mUpdatedFields );

  if ( !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) )
    f.setGeometry( geom );

  bool subsetAttrs = ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes );
  if ( !subsetAttrs || ( subsetAttrs && mRequest.subsetOfAttributes().count() > 0 ) )
  {
    // retrieve attributes from provider
    QgsFeature tmp;
    //mDataProvider->featureAtId( fid, tmp, false, mFetchProvAttributes );
    QgsFeatureRequest request;
    request.setFilterFid( fid ).setFlags( QgsFeatureRequest::NoGeometry ).setSubsetOfAttributes( mProviderRequest.subsetOfAttributes() );
    QgsFeatureIterator fi = L->dataProvider()->getFeatures( request );
    if ( fi.nextFeature( tmp ) )
    {
      updateChangedAttributes( tmp );
      f.setAttributes( tmp.attributes() );
    }
  }

  if ( !mFetchJoinInfo.isEmpty() )
    addJoinedAttributes( f );
}



void QgsVectorLayerFeatureIterator::rewindEditBuffer()
{
  mFetchConsidered = mDeletedFeatureIds;

  mFetchAddedFeaturesIt = mAddedFeatures.constBegin();
  mFetchChangedGeomIt = mChangedGeometries.constBegin();
}



void QgsVectorLayerFeatureIterator::prepareJoins()
{
  QgsAttributeList fetchAttributes = ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes ) ? mRequest.subsetOfAttributes() : L->pendingAllAttributesList();
  QgsAttributeList sourceJoinFields; // attributes that also need to be fetched from this layer in order to have joins working

  mFetchJoinInfo.clear();

  QgsVectorLayerJoinBuffer* joinBuffer = L->mJoinBuffer;
  const QgsFields& fields = L->pendingFields();

  for ( QgsAttributeList::const_iterator attIt = fetchAttributes.constBegin(); attIt != fetchAttributes.constEnd(); ++attIt )
  {
    if ( fields.fieldOrigin( *attIt ) != QgsFields::OriginJoin )
      continue;

    int sourceLayerIndex;
    const QgsVectorJoinInfo* joinInfo = joinBuffer->joinForFieldIndex( *attIt, fields, sourceLayerIndex );
    Q_ASSERT( joinInfo );

    QgsVectorLayer* joinLayer = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( joinInfo->joinLayerId ) );
    Q_ASSERT( joinLayer );

    if ( !mFetchJoinInfo.contains( joinLayer ) )
    {
      FetchJoinInfo info;
      info.joinInfo = joinInfo;
      info.joinLayer = joinLayer;

      if ( joinInfo->targetFieldName.isEmpty() )
        info.targetField = joinInfo->targetFieldIndex;    //for compatibility with 1.x
      else
        info.targetField = fields.indexFromName( joinInfo->targetFieldName );

      if ( joinInfo->joinFieldName.isEmpty() )
        info.joinField = joinInfo->joinFieldIndex;      //for compatibility with 1.x
      else
        info.joinField = joinLayer->pendingFields().indexFromName( joinInfo->joinFieldName );

      info.indexOffset = *attIt - sourceLayerIndex;
      if ( info.joinField < sourceLayerIndex )
        info.indexOffset++;

      // for joined fields, we always need to request the targetField from the provider too
      if ( !fetchAttributes.contains( info.targetField ) )
        sourceJoinFields << info.targetField;

      mFetchJoinInfo.insert( joinLayer, info );
    }

    // store field source index - we'll need it when fetching from provider
    mFetchJoinInfo[ joinLayer ].attributes.push_back( sourceLayerIndex );
  }

  // add sourceJoinFields if we're using a subset
  if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
    mRequest.setSubsetOfAttributes( mRequest.subsetOfAttributes() + sourceJoinFields );
}


void QgsVectorLayerFeatureIterator::addJoinedAttributes( QgsFeature &f )
{
  // make sure we have space for newly added attributes
  f.attributes().resize( L->pendingFields().count() );  // f.attributes().count() + mJoinedAttributesCount );

  QMap<QgsVectorLayer*, FetchJoinInfo>::const_iterator joinIt = mFetchJoinInfo.constBegin();
  for ( ; joinIt != mFetchJoinInfo.constEnd(); ++joinIt )
  {
    const FetchJoinInfo& info = joinIt.value();
    Q_ASSERT( joinIt.key() );

    QVariant targetFieldValue = f.attribute( info.targetField );
    if ( !targetFieldValue.isValid() )
      continue;

    const QHash< QString, QgsAttributes>& memoryCache = info.joinInfo->cachedAttributes;
    if ( memoryCache.isEmpty() )
      info.addJoinedAttributesDirect( f, targetFieldValue );
    else
      info.addJoinedAttributesCached( f, targetFieldValue );
  }
}



void QgsVectorLayerFeatureIterator::FetchJoinInfo::addJoinedAttributesCached( QgsFeature& f, const QVariant& joinValue ) const
{
  const QHash<QString, QgsAttributes>& memoryCache = joinInfo->cachedAttributes;
  QHash<QString, QgsAttributes>::const_iterator it = memoryCache.find( joinValue.toString() );
  if ( it == memoryCache.constEnd() )
    return; // joined value not found -> leaving the attributes empty (null)

  int index = indexOffset;

  const QgsAttributes& featureAttributes = it.value();
  for ( int i = 0; i < featureAttributes.count(); ++i )
  {
    // skip the join field to avoid double field names (fields often have the same name)
    if ( i == joinField )
      continue;

    f.setAttribute( index++, featureAttributes[i] );
  }
}



void QgsVectorLayerFeatureIterator::FetchJoinInfo::addJoinedAttributesDirect( QgsFeature& f, const QVariant& joinValue ) const
{
  // no memory cache, query the joined values by setting substring
  QString subsetString = joinLayer->dataProvider()->subsetString(); // provider might already have a subset string
  QString bkSubsetString = subsetString;
  if ( !subsetString.isEmpty() )
  {
    subsetString.append( " AND " );
  }

  QString joinFieldName;
  if ( joinInfo->joinFieldName.isEmpty() && joinInfo->joinFieldIndex >= 0 && joinInfo->joinFieldIndex < joinLayer->pendingFields().count() )
    joinFieldName = joinLayer->pendingFields().field( joinInfo->joinFieldIndex ).name();   // for compatibility with 1.x
  else
    joinFieldName = joinInfo->joinFieldName;

  subsetString.append( "\"" + joinFieldName + "\"" + " = " + "\"" + joinValue.toString() + "\"" );
  joinLayer->dataProvider()->setSubsetString( subsetString, false );

  // select (no geometry)
  QgsFeatureRequest request;
  request.setFlags( QgsFeatureRequest::NoGeometry );
  request.setSubsetOfAttributes( attributes );
  QgsFeatureIterator fi = joinLayer->getFeatures( request );

  // get first feature
  QgsFeature fet;
  if ( fi.nextFeature( fet ) )
  {
    int index = indexOffset;
    const QgsAttributes& attr = fet.attributes();
    for ( int i = 0; i < attr.count(); ++i )
    {
      if ( i == joinField )
        continue;

      f.setAttribute( index++, attr[i] );
    }
  }
  else
  {
    // no suitable join feature found, keeping empty (null) attributes
  }

  joinLayer->dataProvider()->setSubsetString( bkSubsetString, false );
}




bool QgsVectorLayerFeatureIterator::nextFeatureFid( QgsFeature& f )
{
  QgsFeatureId featureId = mRequest.filterFid();

  // deleted already?
  if ( mDeletedFeatureIds.contains( featureId ) )
    return false;

  // has changed geometry?
  if ( !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) && mChangedGeometries.contains( featureId ) )
  {
    useChangedAttributeFeature( featureId, mChangedGeometries[featureId], f );
    return true;
  }

  // added features
  for ( QgsFeatureMap::ConstIterator iter = mAddedFeatures.constBegin(); iter != mAddedFeatures.constEnd(); ++iter )
  {
    if ( iter->id() == featureId )
    {
      useAddedFeature( *iter, f );
      return true;
    }
  }

  // regular features
  QgsFeatureIterator fi = L->dataProvider()->getFeatures( mProviderRequest );
  if ( fi.nextFeature( f ) )
  {
    updateChangedAttributes( f );

    if ( !mFetchJoinInfo.isEmpty() )
      addJoinedAttributes( f );

    return true;
  }

  return false;
}

void QgsVectorLayerFeatureIterator::updateChangedAttributes( QgsFeature &f )
{
  QgsAttributes& attrs = f.attributes();

  // remove all attributes that will disappear - from higher indices to lower
  for ( int idx = mDeletedAttributeIds.count() - 1; idx >= 0; --idx )
  {
    attrs.remove( mDeletedAttributeIds[idx] );
  }

  // adjust size to accommodate added attributes
  attrs.resize( attrs.count() + mAddedAttributes.count() );

  // update changed attributes
  if ( mChangedAttributeValues.contains( f.id() ) )
  {
    const QgsAttributeMap &map = mChangedAttributeValues[f.id()];
    for ( QgsAttributeMap::const_iterator it = map.begin(); it != map.end(); it++ )
      attrs[it.key()] = it.value();
  }
}

void QgsVectorLayerFeatureIterator::updateFeatureGeometry( QgsFeature &f )
{
  if ( mChangedGeometries.contains( f.id() ) )
    f.setGeometry( mChangedGeometries[f.id()] );
}
