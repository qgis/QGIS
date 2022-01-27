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
#include "qgssimplifymethod.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayereditbuffer.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerjoinbuffer.h"
#include "qgsexpressioncontext.h"
#include "qgsdistancearea.h"
#include "qgsproject.h"
#include "qgsmessagelog.h"
#include "qgsexception.h"
#include "qgsexpressioncontextutils.h"
#include "qgsgeometryengine.h"

#include <QThreadStorage>
#include <QStack>

QgsVectorLayerFeatureSource::QgsVectorLayerFeatureSource( const QgsVectorLayer *layer )
{
  const QMutexLocker locker( &layer->mFeatureSourceConstructorMutex );
  const QgsVectorDataProvider *provider = layer->dataProvider();
  if ( provider )
    mProviderFeatureSource.reset( provider->featureSource() );
  mFields = layer->fields();
  mId = layer->id();

  // update layer's join caches if necessary
  if ( layer->mJoinBuffer->containsJoins() )
    layer->mJoinBuffer->createJoinCaches();

  mJoinBuffer.reset( layer->mJoinBuffer->clone() );

  mExpressionFieldBuffer.reset( new QgsExpressionFieldBuffer( *layer->mExpressionFieldBuffer ) );
  mCrs = layer->crs();

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
      // If we are inside a transaction the iterator "sees" the current status
      if ( provider && ! provider->transaction() )
      {
        mAddedFeatures = QgsFeatureMap( layer->editBuffer()->addedFeatures() );
        mChangedGeometries = QgsGeometryMap( layer->editBuffer()->changedGeometries() );
        mDeletedFeatureIds = QgsFeatureIds( layer->editBuffer()->deletedFeatureIds() );
        mChangedAttributeValues = QgsChangedAttributesMap( layer->editBuffer()->changedAttributeValues() );
        mAddedAttributes = QList<QgsField>( layer->editBuffer()->addedAttributes() );
        mDeletedAttributeIds = QgsAttributeList( layer->editBuffer()->deletedAttributeIds() );
      }
#if 0
    }
#endif
  }

  const std::unique_ptr< QgsExpressionContextScope > layerScope( QgsExpressionContextUtils::layerScope( layer ) );
  mLayerScope = *layerScope;
}

QgsVectorLayerFeatureSource::~QgsVectorLayerFeatureSource() = default;

QgsFeatureIterator QgsVectorLayerFeatureSource::getFeatures( const QgsFeatureRequest &request )
{
  // return feature iterator that does not own this source
  return QgsFeatureIterator( new QgsVectorLayerFeatureIterator( this, false, request ) );
}

QgsFields QgsVectorLayerFeatureSource::fields() const
{
  return mFields;
}

QgsCoordinateReferenceSystem QgsVectorLayerFeatureSource::crs() const
{
  return mCrs;
}

QString QgsVectorLayerFeatureSource::id() const
{
  return mId;
}


QgsVectorLayerFeatureIterator::QgsVectorLayerFeatureIterator( QgsVectorLayerFeatureSource *source, bool ownSource, const QgsFeatureRequest &request )
  : QgsAbstractFeatureIteratorFromSource<QgsVectorLayerFeatureSource>( source, ownSource, request )
  , mFetchedFid( false )
{
  if ( mRequest.destinationCrs().isValid() && mRequest.destinationCrs() != mSource->mCrs )
  {
    mTransform = QgsCoordinateTransform( mSource->mCrs, mRequest.destinationCrs(), mRequest.transformContext() );
  }

  // prepare spatial filter geometries for optimal speed
  // since the mDistanceWithin* constraint member variables are all in the DESTINATION CRS,
  // we set all these upfront before any transformation to the source CRS is done.

  switch ( mRequest.spatialFilterType() )
  {
    case Qgis::SpatialFilterType::NoFilter:
    case Qgis::SpatialFilterType::BoundingBox:
      break;

    case Qgis::SpatialFilterType::DistanceWithin:
      if ( !mRequest.referenceGeometry().isEmpty() )
      {
        // Note that regardless of whether or not we'll ultimately be able to handoff this check to the underlying provider,
        // we still need these reference geometry constraints in the vector layer iterator as we need them to check against
        // the features from the vector layer's edit buffer! (In other words, we cannot completely hand off responsibility for
        // these checks to the provider and ignore them locally)
        mDistanceWithinGeom = mRequest.referenceGeometry();
        mDistanceWithinEngine = mRequest.referenceGeometryEngine();
        mDistanceWithinEngine->prepareGeometry();
        mDistanceWithin = mRequest.distanceWithin();
      }
      break;
  }

  bool canDelegateLimitToProvider = true;
  try
  {
    switch ( updateRequestToSourceCrs( mRequest, mTransform ) )
    {
      case QgsAbstractFeatureIterator::RequestToSourceCrsResult::Success:
        break;

      case QgsAbstractFeatureIterator::RequestToSourceCrsResult::DistanceWithinMustBeCheckedManually:
        // we have to disable any limit on the provider's request -- since that request may be returning features which are outside the
        // distance tolerance, we'll have to fetch them all and then handle the limit check manually only after testing for the distance within constraint
        canDelegateLimitToProvider = false;
        break;
    }

    // mFilterRect is in the source CRS, so we set that now (after request transformation has been done)
    mFilterRect = mRequest.filterRect();
  }
  catch ( QgsCsException & )
  {
    // can't reproject request filters
    close();
    return;
  }

  // check whether the order by clause(s) can be delegated to the provider
  mDelegatedOrderByToProvider = !mSource->mHasEditBuffer;
  if ( !mRequest.orderBy().isEmpty() )
  {
    QSet<int> attributeIndexes;
    const auto usedAttributeIndices = mRequest.orderBy().usedAttributeIndices( mSource->mFields );
    for ( const int attrIndex : usedAttributeIndices )
    {
      if ( mSource->mFields.fieldOrigin( attrIndex ) != QgsFields::OriginProvider )
        mDelegatedOrderByToProvider = false;

      attributeIndexes << attrIndex;
    }

    if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes && !mDelegatedOrderByToProvider )
    {
      attributeIndexes += qgis::listToSet( mRequest.subsetOfAttributes() );
      mRequest.setSubsetOfAttributes( qgis::setToList( attributeIndexes ) );
    }
  }

  if ( mRequest.filterType() == QgsFeatureRequest::FilterExpression )
  {
    mRequest.expressionContext()->setFields( mSource->mFields );
    mRequest.filterExpression()->prepare( mRequest.expressionContext() );

    if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
    {
      // ensure that all fields required for filter expressions are prepared
      QSet<int> attributeIndexes = mRequest.filterExpression()->referencedAttributeIndexes( mSource->mFields );
      attributeIndexes += qgis::listToSet( mRequest.subsetOfAttributes() );
      mRequest.setSubsetOfAttributes( qgis::setToList( attributeIndexes ) );
    }
  }

  prepareFields();

  mHasVirtualAttributes = !mFetchJoinInfo.isEmpty() || !mExpressionFieldInfo.isEmpty();

  // by default provider's request is the same
  mProviderRequest = mRequest;
  // but we remove any destination CRS parameter - that is handled in QgsVectorLayerFeatureIterator,
  // not at the provider level. Otherwise virtual fields depending on geometry would have incorrect
  // values
  if ( mRequest.destinationCrs().isValid() )
  {
    mProviderRequest.setDestinationCrs( QgsCoordinateReferenceSystem(), mRequest.transformContext() );
  }

  if ( !mDelegatedOrderByToProvider )
  {
    mProviderRequest.setOrderBy( QgsFeatureRequest::OrderBy() );
  }

  if ( !canDelegateLimitToProvider )
  {
    mProviderRequest.setLimit( -1 );
  }

  if ( mProviderRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
  {
    // prepare list of attributes to match provider fields
    QSet<int> providerSubset;
    const QgsAttributeList subset = mProviderRequest.subsetOfAttributes();
    const int nPendingFields = mSource->mFields.count();
    for ( const int attrIndex : subset )
    {
      if ( attrIndex < 0 || attrIndex >= nPendingFields )
        continue;
      if ( mSource->mFields.fieldOrigin( attrIndex ) == QgsFields::OriginProvider )
        providerSubset << mSource->mFields.fieldOriginIndex( attrIndex );
    }

    // This is done in order to be prepared to do fallback order bys
    // and be sure we have the required columns.
    // TODO:
    // It would be nicer to first check if we can compile the order by
    // and only modify the subset if we cannot.
    if ( !mProviderRequest.orderBy().isEmpty() )
    {
      const auto usedAttributeIndices = mProviderRequest.orderBy().usedAttributeIndices( mSource->mFields );
      for ( const int attrIndex : usedAttributeIndices )
      {
        providerSubset << attrIndex;
      }
    }

    mProviderRequest.setSubsetOfAttributes( qgis::setToList( providerSubset ) );
  }

  if ( mProviderRequest.filterType() == QgsFeatureRequest::FilterExpression )
  {
    const bool needsGeom = mProviderRequest.filterExpression()->needsGeometry();
    const auto constReferencedColumns = mProviderRequest.filterExpression()->referencedColumns();
    for ( const QString &field : constReferencedColumns )
    {
      const int idx = source->mFields.lookupField( field );

      // If there are fields in the expression which are not of origin provider, the provider will not be able to filter based on them.
      // In this case we disable the expression filter.
      if ( source->mFields.fieldOrigin( idx ) != QgsFields::OriginProvider )
      {
        mProviderRequest.disableFilter();
        // can't limit at provider side
        mProviderRequest.setLimit( -1 );
        if ( needsGeom )
        {
          // have to get geometry from provider in order to evaluate expression on client
          mProviderRequest.setFlags( mProviderRequest.flags() & ~QgsFeatureRequest::NoGeometry );
        }
        break;
      }
    }
  }

  if ( mSource->mHasEditBuffer )
  {
    mChangedFeaturesRequest = mProviderRequest;
    QgsFeatureIds changedIds;
    QgsChangedAttributesMap::const_iterator attIt = mSource->mChangedAttributeValues.constBegin();
    for ( ; attIt != mSource->mChangedAttributeValues.constEnd(); ++attIt )
    {
      changedIds << attIt.key();
    }
    mChangedFeaturesRequest.setFilterFids( changedIds );

    if ( mChangedFeaturesRequest.limit() > 0 )
    {
      int providerLimit = mProviderRequest.limit();

      // features may be deleted in buffer, so increase limit sent to provider
      providerLimit += mSource->mDeletedFeatureIds.size();

      if ( mProviderRequest.filterType() == QgsFeatureRequest::FilterExpression )
      {
        // attribute changes may mean some features no longer match expression, so increase limit sent to provider
        providerLimit += mSource->mChangedAttributeValues.size();
      }

      if ( mProviderRequest.filterType() == QgsFeatureRequest::FilterExpression || mProviderRequest.spatialFilterType() != Qgis::SpatialFilterType::NoFilter )
      {
        // geometry changes may mean some features no longer match expression or rect, so increase limit sent to provider
        providerLimit += mSource->mChangedGeometries.size();
      }

      mProviderRequest.setLimit( providerLimit );
      mChangedFeaturesRequest.setLimit( providerLimit );
    }
  }

  if ( request.filterType() == QgsFeatureRequest::FilterFid )
  {
    mFetchedFid = false;
  }
  else // no filter or filter by rect
  {
    if ( mSource->mProviderFeatureSource )
    {
      if ( mSource->mHasEditBuffer )
      {
        mChangedFeaturesIterator = mSource->mProviderFeatureSource->getFeatures( mChangedFeaturesRequest );
      }
      else
      {
        mProviderIterator = mSource->mProviderFeatureSource->getFeatures( mProviderRequest );
      }
    }

    rewindEditBuffer();
  }
}


QgsVectorLayerFeatureIterator::~QgsVectorLayerFeatureIterator()
{
  qDeleteAll( mExpressionFieldInfo );

  close();
}

/// @cond private

/**
 * This class guards against infinite recursion.
 * The counter will be created per thread and hasStackOverflow will return
 * TRUE if more than maxDepth instances are created in parallel in a thread.
 */
class QgsThreadStackOverflowGuard
{
  public:

    QgsThreadStackOverflowGuard( QThreadStorage<QStack<QString>> &storage, const QString &stackFrameInformation, int maxDepth )
      : mStorage( storage )
      , mMaxDepth( maxDepth )
    {
      if ( !storage.hasLocalData() )
      {
        storage.setLocalData( QStack<QString>() );
      }

      storage.localData().push( stackFrameInformation );
    }

    ~QgsThreadStackOverflowGuard()
    {
      mStorage.localData().pop();
    }

    bool hasStackOverflow() const
    {
      if ( mStorage.localData().size() > mMaxDepth )
        return true;
      else
        return false;
    }

    QString topFrames() const
    {
      QStringList dumpStack;
      const QStack<QString> &stack = mStorage.localData();

      const int dumpSize = std::min( static_cast<int>( stack.size() ), 10 );
      for ( int i = 0; i < dumpSize; ++i )
      {
        dumpStack += stack.at( i );
      }

      return dumpStack.join( '\n' );
    }

    int depth() const
    {
      return mStorage.localData().size();
    }

  private:
    QThreadStorage<QStack<QString>> &mStorage;
    int mMaxDepth;
};

/// @endcond private

bool QgsVectorLayerFeatureIterator::fetchFeature( QgsFeature &f )
{
  f.setValid( false );

  if ( mClosed )
    return false;

  static QThreadStorage<QStack<QString>> sStack;

  const QgsThreadStackOverflowGuard guard( sStack, mSource->id(), 4 );

  if ( guard.hasStackOverflow() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Stack overflow, too many nested feature iterators.\nIterated layers:\n%3\n..." ).arg( mSource->id(), guard.topFrames() ), QObject::tr( "General" ), Qgis::MessageLevel::Critical );
    return false;
  }

  if ( mRequest.filterType() == QgsFeatureRequest::FilterFid )
  {
    if ( mFetchedFid )
      return false;
    const bool res = nextFeatureFid( f );
    if ( res && postProcessFeature( f ) )
    {
      mFetchedFid = true;
      return res;
    }
    else
    {
      return false;
    }
  }

  if ( !mFilterRect.isNull() )
  {
    if ( fetchNextChangedGeomFeature( f ) )
      return true;

    // no more changed geometries
  }

  if ( mRequest.filterType() == QgsFeatureRequest::FilterExpression )
  {
    if ( fetchNextChangedAttributeFeature( f ) )
      return true;

    if ( fetchNextChangedGeomFeature( f ) )
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
    if ( mSource->mProviderFeatureSource )
    {
      mProviderIterator = mSource->mProviderFeatureSource->getFeatures( mProviderRequest );
      mProviderIterator.setInterruptionChecker( mInterruptionChecker );
    }
  }

  while ( mProviderIterator.nextFeature( f ) )
  {
    if ( mFetchConsidered.contains( f.id() ) )
      continue;

    // TODO[MD]: just one resize of attributes
    f.setFields( mSource->mFields );

    // update attributes
    if ( mSource->mHasEditBuffer )
      updateChangedAttributes( f );

    if ( mHasVirtualAttributes )
      addVirtualAttributes( f );

    if ( mRequest.filterType() == QgsFeatureRequest::FilterExpression && mProviderRequest.filterType() != QgsFeatureRequest::FilterExpression )
    {
      //filtering by expression, and couldn't do it on the provider side
      mRequest.expressionContext()->setFeature( f );
      if ( !mRequest.filterExpression()->evaluate( mRequest.expressionContext() ).toBool() )
      {
        //feature did not match filter
        continue;
      }
    }

    // update geometry
    // TODO[MK]: FilterRect check after updating the geometry
    if ( !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) )
      updateFeatureGeometry( f );

    if ( !postProcessFeature( f ) )
      continue;

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

void QgsVectorLayerFeatureIterator::setInterruptionChecker( QgsFeedback *interruptionChecker )
{
  mProviderIterator.setInterruptionChecker( interruptionChecker );
  mInterruptionChecker = interruptionChecker;
}

bool QgsVectorLayerFeatureIterator::isValid() const
{
  return mProviderIterator.isValid();
}

bool QgsVectorLayerFeatureIterator::fetchNextAddedFeature( QgsFeature &f )
{
  while ( mFetchAddedFeaturesIt != mSource->mAddedFeatures.constBegin() )
  {
    --mFetchAddedFeaturesIt;
    const QgsFeatureId fid = mFetchAddedFeaturesIt->id();

    if ( mFetchConsidered.contains( fid ) )
      // must have changed geometry outside rectangle
      continue;

    useAddedFeature( *mFetchAddedFeaturesIt, f );

    // can't test for feature acceptance until after calling useAddedFeature
    // since acceptFeature may rely on virtual fields
    if ( !mRequest.acceptFeature( f ) )
      // skip features which are not accepted by the filter
      continue;

    if ( !postProcessFeature( f ) )
      continue;

    return true;
  }

  mFetchAddedFeaturesIt = mSource->mAddedFeatures.constBegin();
  return false; // no more added features
}


void QgsVectorLayerFeatureIterator::useAddedFeature( const QgsFeature &src, QgsFeature &f )
{
  // since QgsFeature is implicitly shared, it's more efficient to just copy the
  // whole feature, even if flags like NoGeometry or a subset of attributes is set at the request.
  // This helps potentially avoid an unnecessary detach of the feature
  f = src;
  f.setValid( true );
  f.setFields( mSource->mFields );

  if ( mHasVirtualAttributes )
    addVirtualAttributes( f );
}



bool QgsVectorLayerFeatureIterator::fetchNextChangedGeomFeature( QgsFeature &f )
{
  // check if changed geometries are in rectangle
  for ( ; mFetchChangedGeomIt != mSource->mChangedGeometries.constEnd(); mFetchChangedGeomIt++ )
  {
    const QgsFeatureId fid = mFetchChangedGeomIt.key();

    if ( mFetchConsidered.contains( fid ) )
      // skip deleted features
      continue;

    mFetchConsidered << fid;

    if ( !mFilterRect.isNull() && !mFetchChangedGeomIt->intersects( mFilterRect ) )
      // skip changed geometries not in rectangle and don't check again
      continue;

    useChangedAttributeFeature( fid, *mFetchChangedGeomIt, f );

    if ( mRequest.filterType() == QgsFeatureRequest::FilterExpression )
    {
      mRequest.expressionContext()->setFeature( f );
      if ( !mRequest.filterExpression()->evaluate( mRequest.expressionContext() ).toBool() )
      {
        continue;
      }
    }

    if ( postProcessFeature( f ) )
    {
      // return complete feature
      mFetchChangedGeomIt++;
      return true;
    }
  }

  return false; // no more changed geometries
}

bool QgsVectorLayerFeatureIterator::fetchNextChangedAttributeFeature( QgsFeature &f )
{
  while ( mChangedFeaturesIterator.nextFeature( f ) )
  {
    if ( mFetchConsidered.contains( f.id() ) )
      continue;

    mFetchConsidered << f.id();

    updateChangedAttributes( f );

    // also update geometry if needed
    const auto changedGeometryIt = mSource->mChangedGeometries.constFind( f.id() );
    if ( changedGeometryIt != mSource->mChangedGeometries.constEnd() )
    {
      if ( !mFilterRect.isNull() && !changedGeometryIt->intersects( mFilterRect ) )
        // skip changed geometries not in rectangle and don't check again
        continue;

      f.setGeometry( *changedGeometryIt );
    }

    if ( mHasVirtualAttributes )
      addVirtualAttributes( f );

    mRequest.expressionContext()->setFeature( f );
    if ( mRequest.filterExpression()->evaluate( mRequest.expressionContext() ).toBool() && postProcessFeature( f ) )
    {
      return true;
    }
  }

  return false;
}


void QgsVectorLayerFeatureIterator::useChangedAttributeFeature( QgsFeatureId fid, const QgsGeometry &geom, QgsFeature &f )
{
  f.setId( fid );
  f.setValid( true );
  f.setFields( mSource->mFields );

  if ( !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) ||
       ( mRequest.filterType() == QgsFeatureRequest::FilterExpression && mRequest.filterExpression()->needsGeometry() ) )
  {
    f.setGeometry( geom );
  }

  const bool subsetAttrs = ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes );
  if ( !subsetAttrs || !mRequest.subsetOfAttributes().isEmpty() )
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

void QgsVectorLayerFeatureIterator::prepareJoin( int fieldIdx )
{
  if ( !mSource->mFields.exists( fieldIdx ) )
    return;

  if ( mSource->mFields.fieldOrigin( fieldIdx ) != QgsFields::OriginJoin )
    return;

  int sourceLayerIndex;
  const QgsVectorLayerJoinInfo *joinInfo = mSource->mJoinBuffer->joinForFieldIndex( fieldIdx, mSource->mFields, sourceLayerIndex );
  Q_ASSERT( joinInfo );

  QgsVectorLayer *joinLayer = joinInfo->joinLayer();
  if ( !joinLayer )
    return;  // invalid join (unresolved reference to layer)

  if ( !mFetchJoinInfo.contains( joinInfo ) )
  {
    FetchJoinInfo info;
    info.joinInfo = joinInfo;
    info.joinSource = std::make_shared< QgsVectorLayerFeatureSource >( joinLayer );
    info.indexOffset = mSource->mJoinBuffer->joinedFieldsOffset( joinInfo, mSource->mFields );
    info.targetField = mSource->mFields.indexFromName( joinInfo->targetFieldName() );
    info.joinField = joinLayer->fields().indexFromName( joinInfo->joinFieldName() );
    info.joinLayerFields = joinLayer->fields();

    // for joined fields, we always need to request the targetField from the provider too
    if ( !mPreparedFields.contains( info.targetField ) && !mFieldsToPrepare.contains( info.targetField ) )
      mFieldsToPrepare << info.targetField;

    if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes && !mRequest.subsetOfAttributes().contains( info.targetField ) )
      mRequest.setSubsetOfAttributes( mRequest.subsetOfAttributes() << info.targetField );

    mFetchJoinInfo.insert( joinInfo, info );
  }

  // store field source index - we'll need it when fetching from provider
  mFetchJoinInfo[ joinInfo ].attributes.push_back( sourceLayerIndex );
  mFetchJoinInfo[ joinInfo ].attributesSourceToDestLayerMap[sourceLayerIndex] = fieldIdx;
}


void QgsVectorLayerFeatureIterator::prepareExpression( int fieldIdx )
{
  static QThreadStorage<QStack<QString>> sStack;

  const QgsThreadStackOverflowGuard guard( sStack, mSource->id(), 4 );

  if ( guard.hasStackOverflow() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Stack overflow when preparing field %1 of layer %2.\nLast frames:\n%3\n..." ).arg( mSource->fields().at( fieldIdx ).name(), mSource->id(), guard.topFrames() ), QObject::tr( "General" ), Qgis::MessageLevel::Critical );
    return;
  }

  const QList<QgsExpressionFieldBuffer::ExpressionField> &exps = mSource->mExpressionFieldBuffer->expressions();

  const int oi = mSource->mFields.fieldOriginIndex( fieldIdx );
  std::unique_ptr<QgsExpression> exp = std::make_unique<QgsExpression>( exps[oi].cachedExpression );

  QgsDistanceArea da;
  da.setSourceCrs( mSource->mCrs, QgsProject::instance()->transformContext() );
  da.setEllipsoid( QgsProject::instance()->ellipsoid() );
  exp->setGeomCalculator( &da );
  exp->setDistanceUnits( QgsProject::instance()->distanceUnits() );
  exp->setAreaUnits( QgsProject::instance()->areaUnits() );

  if ( !mExpressionContext )
    createExpressionContext();
  exp->prepare( mExpressionContext.get() );
  const QSet<int> referencedColumns = exp->referencedAttributeIndexes( mSource->fields() );

  QSet<int> requestedAttributes = qgis::listToSet( mRequest.subsetOfAttributes() );

  for ( const int dependentFieldIdx : referencedColumns )
  {
    if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
    {
      requestedAttributes += dependentFieldIdx;
    }
    // also need to fetch this dependent field
    if ( !mPreparedFields.contains( dependentFieldIdx ) && !mFieldsToPrepare.contains( dependentFieldIdx ) )
      mFieldsToPrepare << dependentFieldIdx;
  }

  if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
  {
    mRequest.setSubsetOfAttributes( qgis::setToList( requestedAttributes ) );
  }

  if ( exp->needsGeometry() )
  {
    mRequest.setFlags( mRequest.flags() & ~QgsFeatureRequest::NoGeometry );
  }

  mExpressionFieldInfo.insert( fieldIdx, exp.release() );
}

void QgsVectorLayerFeatureIterator::prepareFields()
{
  mPreparedFields.clear();
  mFieldsToPrepare.clear();
  mFetchJoinInfo.clear();
  mOrderedJoinInfoList.clear();

  mExpressionContext.reset();

  mFieldsToPrepare = ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
                     ? mRequest.subsetOfAttributes()
                     : mSource->mFields.allAttributesList();

  while ( !mFieldsToPrepare.isEmpty() )
  {
    const int fieldIdx = mFieldsToPrepare.takeFirst();
    if ( mPreparedFields.contains( fieldIdx ) )
      continue;

    mPreparedFields << fieldIdx;
    prepareField( fieldIdx );
  }

  //sort joins by dependency
  if ( !mFetchJoinInfo.empty() )
  {
    createOrderedJoinList();
  }
}

void QgsVectorLayerFeatureIterator::createOrderedJoinList()
{
  mOrderedJoinInfoList = mFetchJoinInfo.values();
  if ( mOrderedJoinInfoList.size() < 2 )
  {
    return;
  }

  QSet<int> resolvedFields; //todo: get provider / virtual fields without joins

  //add all provider fields without joins as resolved fields
  QList< int >::const_iterator prepFieldIt = mPreparedFields.constBegin();
  for ( ; prepFieldIt != mPreparedFields.constEnd(); ++prepFieldIt )
  {
    if ( mSource->mFields.fieldOrigin( *prepFieldIt ) != QgsFields::OriginJoin )
    {
      resolvedFields.insert( *prepFieldIt );
    }
  }

  //iterate through the joins. If target field is not yet covered, move the entry to the end of the list

  //some join combinations might not have a resolution at all
  const int maxIterations = ( mOrderedJoinInfoList.size() + 1 ) * mOrderedJoinInfoList.size() / 2.0;
  int currentIteration = 0;

  for ( int i = 0; i < mOrderedJoinInfoList.size() - 1; ++i )
  {
    if ( !resolvedFields.contains( mOrderedJoinInfoList.at( i ).targetField ) )
    {
      mOrderedJoinInfoList.append( mOrderedJoinInfoList.at( i ) );
      mOrderedJoinInfoList.removeAt( i );
      --i;
    }
    else
    {
      const int offset = mOrderedJoinInfoList.at( i ).indexOffset;
      const int joinField = mOrderedJoinInfoList.at( i ).joinField;

      const QgsAttributeList attributes = mOrderedJoinInfoList.at( i ).attributes;
      for ( int n = 0; n < attributes.size(); n++ )
      {
        if ( n != joinField )
        {
          resolvedFields.insert( joinField < n ? n + offset - 1 : n + offset );
        }
      }
    }

    ++currentIteration;
    if ( currentIteration >= maxIterations )
    {
      break;
    }
  }
}

bool QgsVectorLayerFeatureIterator::postProcessFeature( QgsFeature &feature )
{
  bool result = checkGeometryValidity( feature );
  if ( result )
    geometryToDestinationCrs( feature, mTransform );

  if ( result && mDistanceWithinEngine && feature.hasGeometry() )
  {
    result = mDistanceWithinEngine->distanceWithin( feature.geometry().constGet(), mDistanceWithin );
  }

  return result;
}

bool QgsVectorLayerFeatureIterator::checkGeometryValidity( const QgsFeature &feature )
{
  if ( !feature.hasGeometry() )
    return true;

  switch ( mRequest.invalidGeometryCheck() )
  {
    case QgsFeatureRequest::GeometryNoCheck:
      return true;

    case QgsFeatureRequest::GeometrySkipInvalid:
    {
      if ( !feature.geometry().isGeosValid() )
      {
        QgsMessageLog::logMessage( QObject::tr( "Geometry error: One or more input features have invalid geometry." ), QString(), Qgis::MessageLevel::Critical );
        if ( mRequest.invalidGeometryCallback() )
        {
          mRequest.invalidGeometryCallback()( feature );
        }
        return false;
      }
      break;
    }

    case QgsFeatureRequest::GeometryAbortOnInvalid:
      if ( !feature.geometry().isGeosValid() )
      {
        QgsMessageLog::logMessage( QObject::tr( "Geometry error: One or more input features have invalid geometry." ), QString(), Qgis::MessageLevel::Critical );
        close();
        if ( mRequest.invalidGeometryCallback() )
        {
          mRequest.invalidGeometryCallback()( feature );
        }
        return false;
      }
      break;
  }

  return true;
}

void QgsVectorLayerFeatureIterator::prepareField( int fieldIdx )
{
  switch ( mSource->mFields.fieldOrigin( fieldIdx ) )
  {
    case QgsFields::OriginExpression:
      prepareExpression( fieldIdx );
      break;

    case QgsFields::OriginJoin:
      if ( mSource->mJoinBuffer->containsJoins() )
      {
        prepareJoin( fieldIdx );
      }
      break;

    case QgsFields::OriginUnknown:
    case QgsFields::OriginProvider:
    case QgsFields::OriginEdit:
      break;
  }
}

void QgsVectorLayerFeatureIterator::addJoinedAttributes( QgsFeature &f )
{
  QList< FetchJoinInfo >::const_iterator joinIt = mOrderedJoinInfoList.constBegin();
  for ( ; joinIt != mOrderedJoinInfoList.constEnd(); ++joinIt )
  {
    const QVariant targetFieldValue = f.attribute( joinIt->targetField );
    if ( !targetFieldValue.isValid() )
      continue;

    const QHash< QString, QgsAttributes> &memoryCache = joinIt->joinInfo->cachedAttributes;
    if ( memoryCache.isEmpty() )
      joinIt->addJoinedAttributesDirect( f, targetFieldValue );
    else
      joinIt->addJoinedAttributesCached( f, targetFieldValue );
  }
}

void QgsVectorLayerFeatureIterator::addVirtualAttributes( QgsFeature &f )
{
  // make sure we have space for newly added attributes
  QgsAttributes attr = f.attributes();
  attr.resize( mSource->mFields.count() );  // Provider attrs count + joined attrs count + expression attrs count
  f.setAttributes( attr );

  // possible TODO - handle combinations of expression -> join -> expression -> join?
  // but for now, write that off as too complex and an unlikely rare, unsupported use case

  QList< int > fetchedVirtualAttributes;
  //first, check through joins for any virtual fields we need
  QMap<const QgsVectorLayerJoinInfo *, FetchJoinInfo>::const_iterator joinIt = mFetchJoinInfo.constBegin();
  for ( ; joinIt != mFetchJoinInfo.constEnd(); ++joinIt )
  {
    if ( mExpressionFieldInfo.contains( joinIt->targetField ) )
    {
      // have to calculate expression field before we can handle this join
      addExpressionAttribute( f, joinIt->targetField );
      fetchedVirtualAttributes << joinIt->targetField;
    }
  }

  if ( !mFetchJoinInfo.isEmpty() )
    addJoinedAttributes( f );

  // add remaining expression fields
  if ( !mExpressionFieldInfo.isEmpty() )
  {
    QMap<int, QgsExpression *>::ConstIterator it = mExpressionFieldInfo.constBegin();
    for ( ; it != mExpressionFieldInfo.constEnd(); ++it )
    {
      if ( fetchedVirtualAttributes.contains( it.key() ) )
        continue;

      addExpressionAttribute( f, it.key() );
    }
  }
}

void QgsVectorLayerFeatureIterator::addExpressionAttribute( QgsFeature &f, int attrIndex )
{
  QgsExpression *exp = mExpressionFieldInfo.value( attrIndex );
  if ( exp )
  {
    if ( !mExpressionContext )
      createExpressionContext();

    mExpressionContext->setFeature( f );
    QVariant val = exp->evaluate( mExpressionContext.get() );
    ( void )mSource->mFields.at( attrIndex ).convertCompatible( val );
    f.setAttribute( attrIndex, val );
  }
  else
  {
    f.setAttribute( attrIndex, QVariant() );
  }
}

bool QgsVectorLayerFeatureIterator::prepareSimplification( const QgsSimplifyMethod &simplifyMethod )
{
  Q_UNUSED( simplifyMethod )
  return false;
}

bool QgsVectorLayerFeatureIterator::providerCanSimplify( QgsSimplifyMethod::MethodType methodType ) const
{
  Q_UNUSED( methodType )
  return false;
}


void QgsVectorLayerFeatureIterator::FetchJoinInfo::addJoinedAttributesCached( QgsFeature &f, const QVariant &joinValue ) const
{
  const QHash<QString, QgsAttributes> &memoryCache = joinInfo->cachedAttributes;
  const QHash<QString, QgsAttributes>::const_iterator it = memoryCache.find( joinValue.toString() );
  if ( it == memoryCache.constEnd() )
    return; // joined value not found -> leaving the attributes empty (null)

  int index = indexOffset;

  const QgsAttributes &featureAttributes = it.value();
  for ( int i = 0; i < featureAttributes.count(); ++i )
  {
    f.setAttribute( index++, featureAttributes.at( i ) );
  }
}



void QgsVectorLayerFeatureIterator::FetchJoinInfo::addJoinedAttributesDirect( QgsFeature &f, const QVariant &joinValue ) const
{
#if 0 // this is not thread safe -- we cannot access the layer here as this will be called from non-main threads.
  // Shortcut
  if ( joinLayer && ! joinLayer->hasFeatures() )
  {
    return;
  }
#endif

  // no memory cache, query the joined values by setting substring
  QString subsetString;

  const QString joinFieldName = joinInfo->joinFieldName();

  subsetString.append( QStringLiteral( "\"%1\"" ).arg( joinFieldName ) );

  if ( joinValue.isNull() )
  {
    subsetString += QLatin1String( " IS NULL" );
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
        v.replace( '\'', QLatin1String( "''" ) );
        v.prepend( '\'' ).append( '\'' );
        break;
    }
    subsetString += '=' + v;
  }

  QList<int> joinedAttributeIndices;

  // maybe user requested just a subset of layer's attributes
  // so we do not have to cache everything
  if ( joinInfo->hasSubset() )
  {
    const QStringList subsetNames = QgsVectorLayerJoinInfo::joinFieldNamesSubset( *joinInfo );
    const QVector<int> subsetIndices = QgsVectorLayerJoinBuffer::joinSubsetIndices( joinLayerFields, subsetNames );
    joinedAttributeIndices = qgis::setToList( qgis::listToSet( attributes ).intersect( qgis::listToSet( subsetIndices.toList() ) ) );
  }
  else
  {
    joinedAttributeIndices = attributes;
  }

  // we don't need the join field, it is already present in the other table
  joinedAttributeIndices.removeAll( joinField );

  // select (no geometry)
  QgsFeatureRequest request;
  request.setFlags( QgsFeatureRequest::NoGeometry );
  request.setSubsetOfAttributes( joinedAttributeIndices );
  request.setFilterExpression( subsetString );
  request.setLimit( 1 );
  QgsFeatureIterator fi = joinSource->getFeatures( request );

  // get first feature
  const QList<int> sourceAttrIndexes = attributesSourceToDestLayerMap.keys();
  QgsFeature fet;
  if ( fi.nextFeature( fet ) )
  {
    const QgsAttributes attr = fet.attributes();

    for ( const int sourceAttrIndex : sourceAttrIndexes )
    {
      if ( sourceAttrIndex == joinField )
        continue;

      const int destAttrIndex = attributesSourceToDestLayerMap.value( sourceAttrIndex );

      f.setAttribute( destAttrIndex, attr.at( sourceAttrIndex ) );
    }
  }
  else
  {
    // no suitable join feature found, keeping empty (null) attributes
  }
}




bool QgsVectorLayerFeatureIterator::nextFeatureFid( QgsFeature &f )
{
  const QgsFeatureId featureId = mRequest.filterFid();

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
    f.setFields( mSource->mFields );

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
  QgsAttributes attrs = f.attributes();

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
  f.setAttributes( attrs );
}

void QgsVectorLayerFeatureIterator::updateFeatureGeometry( QgsFeature &f )
{
  if ( mSource->mChangedGeometries.contains( f.id() ) )
    f.setGeometry( mSource->mChangedGeometries[f.id()] );
}

void QgsVectorLayerFeatureIterator::createExpressionContext()
{
  mExpressionContext = std::make_unique< QgsExpressionContext >();
  mExpressionContext->appendScope( QgsExpressionContextUtils::globalScope() );
  mExpressionContext->appendScope( QgsExpressionContextUtils::projectScope( QgsProject::instance() ) );
  mExpressionContext->appendScope( new QgsExpressionContextScope( mSource->mLayerScope ) );
  mExpressionContext->setFeedback( mRequest.feedback() );
}

bool QgsVectorLayerFeatureIterator::prepareOrderBy( const QList<QgsFeatureRequest::OrderByClause> &orderBys )
{
  Q_UNUSED( orderBys )
  return mDelegatedOrderByToProvider;
}


//
// QgsVectorLayerSelectedFeatureSource
//

QgsVectorLayerSelectedFeatureSource::QgsVectorLayerSelectedFeatureSource( QgsVectorLayer *layer )
  : mSource( layer )
  , mSelectedFeatureIds( layer->selectedFeatureIds() )
  , mWkbType( layer->wkbType() )
  , mName( layer->name() )
  , mLayer( layer )
{}

QgsFeatureIterator QgsVectorLayerSelectedFeatureSource::getFeatures( const QgsFeatureRequest &request ) const
{
  QgsFeatureRequest req( request );

  // while QgsVectorLayerSelectedFeatureIterator will reject any features not in mSelectedFeatureIds,
  // we still tweak the feature request to only request selected feature ids wherever we can -- this
  // allows providers to optimise the request and avoid requesting features we don't need
  // note that we can't do this for some request types - e.g. expression based requests, so
  // in that case we just pass the request on to the provider and let QgsVectorLayerSelectedFeatureIterator
  // do ALL the filtering
  if ( req.filterFids().isEmpty() && req.filterType() == QgsFeatureRequest::FilterNone )
  {
    req.setFilterFids( mSelectedFeatureIds );
  }
  else if ( !req.filterFids().isEmpty() )
  {
    QgsFeatureIds reqIds = mSelectedFeatureIds;
    reqIds.intersect( req.filterFids() );
    req.setFilterFids( reqIds );
  }

  return QgsFeatureIterator( new QgsVectorLayerSelectedFeatureIterator( mSelectedFeatureIds, req, mSource ) );
}

QgsCoordinateReferenceSystem QgsVectorLayerSelectedFeatureSource::sourceCrs() const
{
  return mSource.crs();
}

QgsFields QgsVectorLayerSelectedFeatureSource::fields() const
{
  return mSource.fields();
}

QgsWkbTypes::Type QgsVectorLayerSelectedFeatureSource::wkbType() const
{
  return mWkbType;
}

long long QgsVectorLayerSelectedFeatureSource::featureCount() const
{
  return mSelectedFeatureIds.count();
}

QString QgsVectorLayerSelectedFeatureSource::sourceName() const
{
  return mName;
}

QgsExpressionContextScope *QgsVectorLayerSelectedFeatureSource::createExpressionContextScope() const
{
  if ( mLayer )
    return mLayer->createExpressionContextScope();
  else
    return nullptr;
}

QgsFeatureSource::SpatialIndexPresence QgsVectorLayerSelectedFeatureSource::hasSpatialIndex() const
{
  if ( mLayer )
    return mLayer->hasSpatialIndex();
  else
    return QgsFeatureSource::SpatialIndexUnknown;
}

//
// QgsVectorLayerSelectedFeatureIterator
//

///@cond PRIVATE
QgsVectorLayerSelectedFeatureIterator::QgsVectorLayerSelectedFeatureIterator( const QgsFeatureIds &selectedFeatureIds, const QgsFeatureRequest &request, QgsVectorLayerFeatureSource &source )
  : QgsAbstractFeatureIterator( request )
  , mSelectedFeatureIds( selectedFeatureIds )
{
  QgsFeatureRequest sourceRequest = request;
  if ( sourceRequest.filterType() == QgsFeatureRequest::FilterExpression && sourceRequest.limit() > 0 )
  {
    // we can't pass the request limit to the provider here - otherwise the provider will
    // limit the number of returned features and may only return a bunch of matching features
    // which AREN'T in the selected feature set
    sourceRequest.setLimit( -1 );
  }
  mIterator = source.getFeatures( sourceRequest );
}

bool QgsVectorLayerSelectedFeatureIterator::rewind()
{
  return mIterator.rewind();
}

bool QgsVectorLayerSelectedFeatureIterator::close()
{
  return mIterator.close();
}

bool QgsVectorLayerSelectedFeatureIterator::fetchFeature( QgsFeature &f )
{
  while ( mIterator.nextFeature( f ) )
  {
    if ( mSelectedFeatureIds.contains( f.id() ) )
      return true;
  }
  return false;
}

///@endcond
