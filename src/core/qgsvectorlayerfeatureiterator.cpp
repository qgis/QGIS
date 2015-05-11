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

#include "qgsexpressionfieldbuffer.h"
#include "qgsgeometrysimplifier.h"
#include "qgsmaplayerregistry.h"
#include "qgssimplifymethod.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayereditbuffer.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerjoinbuffer.h"

QgsVectorLayerFeatureSource::QgsVectorLayerFeatureSource( QgsVectorLayer *layer )
{
  mProviderFeatureSource = layer->dataProvider()->featureSource();
  mFields = layer->pendingFields();
  mJoinBuffer = layer->mJoinBuffer->clone();
  mExpressionFieldBuffer = new QgsExpressionFieldBuffer( *layer->mExpressionFieldBuffer );

  mCanBeSimplified = layer->hasGeometryType() && layer->geometryType() != QGis::Point;

  mHasEditBuffer = layer->editBuffer();
  if ( mHasEditBuffer )
  {
#if 0
    // TODO[MD]: after merge
    if ( request.filterType() == QgsFeatureRequest::FilterFid )
    {

      // only copy relevant parts
      if ( L->editBuffer()->addedFeatures().contains( request.filterFid() ) )
        mAddedFeatures.insert( request.filterFid(), L->editBuffer()->addedFeatures()[ request.filterFid()] );

      if ( L->editBuffer()->changedGeometries().contains( request.filterFid() ) )
        mChangedGeometries.insert( request.filterFid(), L->editBuffer()->changedGeometries()[ request.filterFid()] );

      if ( L->editBuffer()->deletedFeatureIds().contains( request.filterFid() ) )
        mDeletedFeatureIds.insert( request.filterFid() );

      if ( L->editBuffer()->changedAttributeValues().contains( request.filterFid() ) )
        mChangedAttributeValues.insert( request.filterFid(), L->editBuffer()->changedAttributeValues()[ request.filterFid()] );

      if ( L->editBuffer()->changedAttributeValues().contains( request.filterFid() ) )
        mChangedFeaturesRequest.setFilterFids( QgsFeatureIds() << request.filterFid() );
    }
    else
    {
#endif
      mAddedFeatures = QgsFeatureMap( layer->editBuffer()->addedFeatures() );
      mChangedGeometries = QgsGeometryMap( layer->editBuffer()->changedGeometries() );
      mDeletedFeatureIds = QgsFeatureIds( layer->editBuffer()->deletedFeatureIds() );
      mChangedAttributeValues = QgsChangedAttributesMap( layer->editBuffer()->changedAttributeValues() );
      mAddedAttributes = QList<QgsField>( layer->editBuffer()->addedAttributes() );
      mDeletedAttributeIds = QgsAttributeList( layer->editBuffer()->deletedAttributeIds() );
#if 0
    }
#endif
  }
}

QgsVectorLayerFeatureSource::~QgsVectorLayerFeatureSource()
{
  delete mJoinBuffer;
  delete mExpressionFieldBuffer;
  delete mProviderFeatureSource;
}

QgsFeatureIterator QgsVectorLayerFeatureSource::getFeatures( const QgsFeatureRequest& request )
{
  // return feature iterator that does not own this source
  return QgsFeatureIterator( new QgsVectorLayerFeatureIterator( this, false, request ) );
}


QgsVectorLayerFeatureIterator::QgsVectorLayerFeatureIterator( QgsVectorLayerFeatureSource* source, bool ownSource, const QgsFeatureRequest& request )
    : QgsAbstractFeatureIteratorFromSource<QgsVectorLayerFeatureSource>( source, ownSource, request )
    , mFetchedFid( false )
    , mEditGeometrySimplifier( 0 )
{

  // prepare joins: may add more attributes to fetch (in order to allow join)
  if ( mSource->mJoinBuffer->containsJoins() )
    prepareJoins();

  prepareExpressions();

  mHasVirtualAttributes = !mFetchJoinInfo.isEmpty() || !mExpressionFieldInfo.isEmpty();

  // by default provider's request is the same
  mProviderRequest = mRequest;

  if ( mProviderRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
  {
    // prepare list of attributes to match provider fields
    QgsAttributeList providerSubset;
    QgsAttributeList subset = mProviderRequest.subsetOfAttributes();
    int nPendingFields = mSource->mFields.count();
    for ( int i = 0; i < subset.count(); ++i )
    {
      int attrIndex = subset[i];
      if ( attrIndex < 0 || attrIndex >= nPendingFields ) continue;
      if ( mSource->mFields.fieldOrigin( attrIndex ) == QgsFields::OriginProvider )
        providerSubset << mSource->mFields.fieldOriginIndex( attrIndex );
    }
    mProviderRequest.setSubsetOfAttributes( providerSubset );
  }

  if ( mSource->mHasEditBuffer )
  {
    mChangedFeaturesRequest = mProviderRequest;
    mChangedFeaturesRequest.setFilterFids( mSource->mChangedAttributeValues.keys().toSet() );
  }

  if ( request.filterType() == QgsFeatureRequest::FilterFid )
  {
    mFetchedFid = false;
  }
  else // no filter or filter by rect
  {
    if ( mSource->mHasEditBuffer )
    {
      mChangedFeaturesIterator = mSource->mProviderFeatureSource->getFeatures( mChangedFeaturesRequest );
    }
    else
    {
      mProviderIterator = mSource->mProviderFeatureSource->getFeatures( mProviderRequest );
    }

    rewindEditBuffer();
  }

  if ( mRequest.filterType() == QgsFeatureRequest::FilterExpression )
  {
    mRequest.filterExpression()->prepare( mSource->mFields );
  }
}


QgsVectorLayerFeatureIterator::~QgsVectorLayerFeatureIterator()
{
  delete mEditGeometrySimplifier;
  mEditGeometrySimplifier = NULL;

  qDeleteAll( mExpressionFieldInfo.values() );

  close();
}



bool QgsVectorLayerFeatureIterator::fetchFeature( QgsFeature& f )
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

  if ( mRequest.filterType() == QgsFeatureRequest::FilterExpression )
  {
    if ( fetchNextChangedAttributeFeature( f ) )
      return true;

    // no more changed features
  }

  while ( fetchNextAddedFeature( f ) )
  {
    return true;
  }
  // no more added features

  if ( mProviderIterator.isClosed() )
  {
    mChangedFeaturesIterator.close();
    mProviderIterator = mSource->mProviderFeatureSource->getFeatures( mProviderRequest );
  }

  while ( mProviderIterator.nextFeature( f ) )
  {
    if ( mFetchConsidered.contains( f.id() ) )
      continue;

    // TODO[MD]: just one resize of attributes
    f.setFields( &mSource->mFields );

    // update attributes
    if ( mSource->mHasEditBuffer )
      updateChangedAttributes( f );

    if ( mHasVirtualAttributes )
      addVirtualAttributes( f );

    // update geometry
    // TODO[MK]: FilterRect check after updating the geometry
    if ( !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) )
      updateFeatureGeometry( f );

    return true;
  }
  // no more provider features

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

  iteratorClosed();

  mClosed = true;
  return true;
}




bool QgsVectorLayerFeatureIterator::fetchNextAddedFeature( QgsFeature& f )
{
  while ( mFetchAddedFeaturesIt-- != mSource->mAddedFeatures.constBegin() )
  {
    QgsFeatureId fid = mFetchAddedFeaturesIt->id();

    if ( mFetchConsidered.contains( fid ) )
      // must have changed geometry outside rectangle
      continue;

    if ( !mRequest.acceptFeature( *mFetchAddedFeaturesIt ) )
      // skip features which are not accepted by the filter
      continue;

    useAddedFeature( *mFetchAddedFeaturesIt, f );

    return true;
  }

  mFetchAddedFeaturesIt = mSource->mAddedFeatures.constBegin();
  return false; // no more added features
}


void QgsVectorLayerFeatureIterator::useAddedFeature( const QgsFeature& src, QgsFeature& f )
{
  f.setFeatureId( src.id() );
  f.setValid( true );
  f.setFields( &mSource->mFields );

  if ( src.constGeometry() && !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) )
  {
    f.setGeometry( new QgsGeometry( *src.constGeometry() ) );

    // simplify the edited geometry using its simplifier configured
    if ( mEditGeometrySimplifier )
    {
      QgsGeometry* geometry = f.geometry();
      QGis::GeometryType geometryType = geometry->type();
      if ( geometryType == QGis::Line || geometryType == QGis::Polygon ) mEditGeometrySimplifier->simplifyGeometry( geometry );
    }
  }

  // TODO[MD]: if subset set just some attributes

  f.setAttributes( src.attributes() );

  if ( mHasVirtualAttributes )
    addVirtualAttributes( f );
}



bool QgsVectorLayerFeatureIterator::fetchNextChangedGeomFeature( QgsFeature& f )
{
  // check if changed geometries are in rectangle
  for ( ; mFetchChangedGeomIt != mSource->mChangedGeometries.constEnd(); mFetchChangedGeomIt++ )
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

bool QgsVectorLayerFeatureIterator::fetchNextChangedAttributeFeature( QgsFeature& f )
{
  while ( mChangedFeaturesIterator.nextFeature( f ) )
  {
    mFetchConsidered << f.id();

    updateChangedAttributes( f );

    if ( mHasVirtualAttributes )
      addVirtualAttributes( f );

    if ( mRequest.filterType() == QgsFeatureRequest::FilterExpression )
    {
      if ( mRequest.filterExpression()->evaluate( &f ).toBool() )
      {
        return true;
      }
    }
    else
    {
      return true;
    }
  }

  return false;
}


void QgsVectorLayerFeatureIterator::useChangedAttributeFeature( QgsFeatureId fid, const QgsGeometry& geom, QgsFeature& f )
{
  f.setFeatureId( fid );
  f.setValid( true );
  f.setFields( &mSource->mFields );

  if ( !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) )
  {
    f.setGeometry( geom );

    // simplify the edited geometry using its simplifier configured
    if ( mEditGeometrySimplifier )
    {
      QgsGeometry* geometry = f.geometry();
      QGis::GeometryType geometryType = geometry->type();
      if ( geometryType == QGis::Line || geometryType == QGis::Polygon ) mEditGeometrySimplifier->simplifyGeometry( geometry );
    }
  }

  bool subsetAttrs = ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes );
  if ( !subsetAttrs || ( subsetAttrs && mRequest.subsetOfAttributes().count() > 0 ) )
  {
    // retrieve attributes from provider
    QgsFeature tmp;
    //mDataProvider->featureAtId( fid, tmp, false, mFetchProvAttributes );
    QgsFeatureRequest request;
    request.setFilterFid( fid ).setFlags( QgsFeatureRequest::NoGeometry );
    if ( subsetAttrs )
    {
      request.setSubsetOfAttributes( mProviderRequest.subsetOfAttributes() );
    }
    QgsFeatureIterator fi = mSource->mProviderFeatureSource->getFeatures( request );
    if ( fi.nextFeature( tmp ) )
    {
      if ( mHasVirtualAttributes || mSource->mHasEditBuffer )
        updateChangedAttributes( tmp );
      f.setAttributes( tmp.attributes() );
    }
  }

  addVirtualAttributes( f );
}



void QgsVectorLayerFeatureIterator::rewindEditBuffer()
{
  mFetchConsidered = mSource->mDeletedFeatureIds;

  mFetchAddedFeaturesIt = mSource->mAddedFeatures.constEnd();
  mFetchChangedGeomIt = mSource->mChangedGeometries.constBegin();
}



void QgsVectorLayerFeatureIterator::prepareJoins()
{
  QgsAttributeList fetchAttributes = ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes ) ? mRequest.subsetOfAttributes() : mSource->mFields.allAttributesList();
  QgsAttributeList sourceJoinFields; // attributes that also need to be fetched from this layer in order to have joins working

  mFetchJoinInfo.clear();

  for ( QgsAttributeList::const_iterator attIt = fetchAttributes.constBegin(); attIt != fetchAttributes.constEnd(); ++attIt )
  {
    if ( !mSource->mFields.exists( *attIt ) )
      continue;

    if ( mSource->mFields.fieldOrigin( *attIt ) != QgsFields::OriginJoin )
      continue;

    int sourceLayerIndex;
    const QgsVectorJoinInfo* joinInfo = mSource->mJoinBuffer->joinForFieldIndex( *attIt, mSource->mFields, sourceLayerIndex );
    Q_ASSERT( joinInfo );

    QgsVectorLayer* joinLayer = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( joinInfo->joinLayerId ) );
    Q_ASSERT( joinLayer );

    if ( !mFetchJoinInfo.contains( joinInfo ) )
    {
      FetchJoinInfo info;
      info.joinInfo = joinInfo;
      info.joinLayer = joinLayer;
      info.indexOffset = mSource->mJoinBuffer->joinedFieldsOffset( joinInfo, mSource->mFields );

      if ( joinInfo->targetFieldName.isEmpty() )
        info.targetField = joinInfo->targetFieldIndex;    //for compatibility with 1.x
      else
        info.targetField = mSource->mFields.indexFromName( joinInfo->targetFieldName );

      if ( joinInfo->joinFieldName.isEmpty() )
        info.joinField = joinInfo->joinFieldIndex;      //for compatibility with 1.x
      else
        info.joinField = joinLayer->pendingFields().indexFromName( joinInfo->joinFieldName );

      // for joined fields, we always need to request the targetField from the provider too
      if ( !fetchAttributes.contains( info.targetField ) )
        sourceJoinFields << info.targetField;

      mFetchJoinInfo.insert( joinInfo, info );
    }

    // store field source index - we'll need it when fetching from provider
    mFetchJoinInfo[ joinInfo ].attributes.push_back( sourceLayerIndex );
  }

  // add sourceJoinFields if we're using a subset
  if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
    mRequest.setSubsetOfAttributes( mRequest.subsetOfAttributes() + sourceJoinFields );
}

void QgsVectorLayerFeatureIterator::prepareExpressions()
{
  const QList<QgsExpressionFieldBuffer::ExpressionField> exps = mSource->mExpressionFieldBuffer->expressions();

  for ( int i = 0; i < mSource->mFields.count(); i++ )
  {
    if ( mSource->mFields.fieldOrigin( i ) == QgsFields::OriginExpression )
    {
      // Only prepare if there is no subset defined or the subset contains this field
      if ( !( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
           || mRequest.subsetOfAttributes().contains( i ) )
      {
        int oi = mSource->mFields.fieldOriginIndex( i );
        QgsExpression* exp = new QgsExpression( exps[oi].expression );
        exp->prepare( mSource->mFields );
        mExpressionFieldInfo.insert( i, exp );

        if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
        {
          QgsAttributeList attrs;
          Q_FOREACH ( const QString& col, exp->referencedColumns() )
          {
            attrs.append( mSource->mFields.fieldNameIndex( col ) );
          }

          mRequest.setSubsetOfAttributes( mRequest.subsetOfAttributes() + attrs );
        }

        if ( exp->needsGeometry() )
        {
          mRequest.setFlags( mRequest.flags() & ~QgsFeatureRequest::NoGeometry );
        }
      }
    }
  }
}

void QgsVectorLayerFeatureIterator::addJoinedAttributes( QgsFeature &f )
{
  QMap<const QgsVectorJoinInfo*, FetchJoinInfo>::const_iterator joinIt = mFetchJoinInfo.constBegin();
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

void QgsVectorLayerFeatureIterator::addVirtualAttributes( QgsFeature& f )
{
  // make sure we have space for newly added attributes
  f.attributes().resize( mSource->mFields.count() );  // Provider attrs count + joined attrs count + expression attrs count

  if ( !mFetchJoinInfo.isEmpty() )
    addJoinedAttributes( f );

  if ( !mExpressionFieldInfo.isEmpty() )
  {
    QMap<int, QgsExpression*>::ConstIterator it = mExpressionFieldInfo.constBegin();

    for ( ; it != mExpressionFieldInfo.constEnd(); ++it )
    {
      QgsExpression* exp = it.value();
      QVariant val = exp->evaluate( f );
      mSource->mFields.at( it.key() ).convertCompatible( val );;
      f.setAttribute( it.key(), val );
    }
  }
}

bool QgsVectorLayerFeatureIterator::prepareSimplification( const QgsSimplifyMethod& simplifyMethod )
{
  delete mEditGeometrySimplifier;
  mEditGeometrySimplifier = NULL;

  // setup simplification for edited geometries to fetch
  if ( !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) && simplifyMethod.methodType() != QgsSimplifyMethod::NoSimplification && mSource->mCanBeSimplified )
  {
    mEditGeometrySimplifier = QgsSimplifyMethod::createGeometrySimplifier( simplifyMethod );
    return mEditGeometrySimplifier != NULL;
  }
  return false;
}

bool QgsVectorLayerFeatureIterator::providerCanSimplify( QgsSimplifyMethod::MethodType methodType ) const
{
  Q_UNUSED( methodType );
#if 0
  // TODO[MD]: after merge
  QgsVectorDataProvider* provider = L->dataProvider();

  if ( provider && methodType != QgsSimplifyMethod::NoSimplification )
  {
    int capabilities = provider->capabilities();

    if ( methodType == QgsSimplifyMethod::OptimizeForRendering )
    {
      return ( capabilities & QgsVectorDataProvider::SimplifyGeometries );
    }
    else if ( methodType == QgsSimplifyMethod::PreserveTopology )
    {
      return ( capabilities & QgsVectorDataProvider::SimplifyGeometriesWithTopologicalValidation );
    }
  }
#endif
  return false;
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
    subsetString.prepend( "(" ).append( ") AND " );
  }

  QString joinFieldName;
  if ( joinInfo->joinFieldName.isEmpty() && joinInfo->joinFieldIndex >= 0 && joinInfo->joinFieldIndex < joinLayer->pendingFields().count() )
    joinFieldName = joinLayer->pendingFields().field( joinInfo->joinFieldIndex ).name();   // for compatibility with 1.x
  else
    joinFieldName = joinInfo->joinFieldName;

  subsetString.append( QString( "\"%1\"" ).arg( joinFieldName ) );

  if ( joinValue.isNull() )
  {
    subsetString += " IS NULL";
  }
  else
  {
    QString v = joinValue.toString();
    switch ( joinValue.type() )
    {
      case QVariant::Int:
      case QVariant::LongLong:
      case QVariant::Double:
        break;

      default:
      case QVariant::String:
        v.replace( "'", "''" );
        v.prepend( "'" ).append( "'" );
        break;
    }
    subsetString += "=" + v;
  }

  joinLayer->dataProvider()->setSubsetString( subsetString, false );

  // maybe user requested just a subset of layer's attributes
  // so we do not have to cache everything
  bool hasSubset = joinInfo->joinFieldNamesSubset();
  QVector<int> subsetIndices;
  if ( hasSubset )
    subsetIndices = QgsVectorLayerJoinBuffer::joinSubsetIndices( joinLayer, *joinInfo->joinFieldNamesSubset() );

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
    if ( hasSubset )
    {
      for ( int i = 0; i < subsetIndices.count(); ++i )
        f.setAttribute( index++, attr[ subsetIndices[i] ] );
    }
    else
    {
      // use all fields except for the one used for join (has same value as exiting field in target layer)
      for ( int i = 0; i < attr.count(); ++i )
      {
        if ( i == joinField )
          continue;

        f.setAttribute( index++, attr[i] );
      }
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
  if ( mSource->mDeletedFeatureIds.contains( featureId ) )
    return false;

  // has changed geometry?
  if ( !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) && mSource->mChangedGeometries.contains( featureId ) )
  {
    useChangedAttributeFeature( featureId, mSource->mChangedGeometries[featureId], f );
    return true;
  }

  // added features
  for ( QgsFeatureMap::ConstIterator iter = mSource->mAddedFeatures.constBegin(); iter != mSource->mAddedFeatures.constEnd(); ++iter )
  {
    if ( iter->id() == featureId )
    {
      useAddedFeature( *iter, f );
      return true;
    }
  }

  // regular features
  QgsFeatureIterator fi = mSource->mProviderFeatureSource->getFeatures( mProviderRequest );
  if ( fi.nextFeature( f ) )
  {
    if ( mSource->mHasEditBuffer )
      updateChangedAttributes( f );

    if ( mHasVirtualAttributes )
      addVirtualAttributes( f );

    return true;
  }

  return false;
}

void QgsVectorLayerFeatureIterator::updateChangedAttributes( QgsFeature &f )
{
  QgsAttributes& attrs = f.attributes();

  // remove all attributes that will disappear - from higher indices to lower
  for ( int idx = mSource->mDeletedAttributeIds.count() - 1; idx >= 0; --idx )
  {
    attrs.remove( mSource->mDeletedAttributeIds[idx] );
  }

  // adjust size to accommodate added attributes
  attrs.resize( attrs.count() + mSource->mAddedAttributes.count() );

  // update changed attributes
  if ( mSource->mChangedAttributeValues.contains( f.id() ) )
  {
    const QgsAttributeMap &map = mSource->mChangedAttributeValues[f.id()];
    for ( QgsAttributeMap::const_iterator it = map.begin(); it != map.end(); ++it )
      attrs[it.key()] = it.value();
  }
}

void QgsVectorLayerFeatureIterator::updateFeatureGeometry( QgsFeature &f )
{
  if ( mSource->mChangedGeometries.contains( f.id() ) )
    f.setGeometry( mSource->mChangedGeometries[f.id()] );
}

