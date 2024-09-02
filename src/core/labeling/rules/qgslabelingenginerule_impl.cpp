/***************************************************************************
    qgslabelingenginerule_impl.cpp
    ---------------------
  Date                 : August 2024
  Copyright            : (C) 2024 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslabelingenginerule_impl.h"
#include "qgsunittypes.h"
#include "qgssymbollayerutils.h"
#include "labelposition.h"
#include "feature.h"
#include "qgsvectorlayerfeatureiterator.h"
#include "qgsthreadingutils.h"
#include "qgsspatialindex.h"
#include "qgsgeos.h"

//
// QgsAbstractLabelingEngineRuleDistanceFromFeature
//

QgsAbstractLabelingEngineRuleDistanceFromFeature::QgsAbstractLabelingEngineRuleDistanceFromFeature() = default;
QgsAbstractLabelingEngineRuleDistanceFromFeature::~QgsAbstractLabelingEngineRuleDistanceFromFeature() = default;

bool QgsAbstractLabelingEngineRuleDistanceFromFeature::prepare( QgsRenderContext &context )
{
  if ( !mTargetLayer )
    return false;

  QGIS_CHECK_OTHER_QOBJECT_THREAD_ACCESS( mTargetLayer );
  mTargetLayerSource = std::make_unique< QgsVectorLayerFeatureSource >( mTargetLayer.get() );

  mDistanceMapUnits = context.convertToMapUnits( mDistance, mDistanceUnit, mDistanceUnitScale );
  return true;
}

void QgsAbstractLabelingEngineRuleDistanceFromFeature::writeXml( QDomDocument &, QDomElement &element, const QgsReadWriteContext & ) const
{
  element.setAttribute( QStringLiteral( "distance" ), mDistance );
  element.setAttribute( QStringLiteral( "distanceUnit" ), QgsUnitTypes::encodeUnit( mDistanceUnit ) );
  element.setAttribute( QStringLiteral( "distanceUnitScale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mDistanceUnitScale ) );
  element.setAttribute( QStringLiteral( "cost" ), mCost );

  if ( mLabeledLayer )
  {
    element.setAttribute( QStringLiteral( "labeledLayer" ), mLabeledLayer.layerId );
    element.setAttribute( QStringLiteral( "labeledLayerName" ), mLabeledLayer.name );
    element.setAttribute( QStringLiteral( "labeledLayerSource" ), mLabeledLayer.source );
    element.setAttribute( QStringLiteral( "labeledLayerProvider" ), mLabeledLayer.provider );
  }
  if ( mTargetLayer )
  {
    element.setAttribute( QStringLiteral( "targetLayer" ), mTargetLayer.layerId );
    element.setAttribute( QStringLiteral( "targetLayerName" ), mTargetLayer.name );
    element.setAttribute( QStringLiteral( "targetLayerSource" ), mTargetLayer.source );
    element.setAttribute( QStringLiteral( "targetLayerProvider" ), mTargetLayer.provider );
  }
}

void QgsAbstractLabelingEngineRuleDistanceFromFeature::readXml( const QDomElement &element, const QgsReadWriteContext & )
{
  mDistance = element.attribute( QStringLiteral( "distance" ), QStringLiteral( "0" ) ).toDouble();
  mDistanceUnit = QgsUnitTypes::decodeRenderUnit( element.attribute( QStringLiteral( "distanceUnit" ) ) );
  mDistanceUnitScale =  QgsSymbolLayerUtils::decodeMapUnitScale( element.attribute( QStringLiteral( "distanceUnitScale" ) ) );
  mCost = element.attribute( QStringLiteral( "cost" ), QStringLiteral( "0" ) ).toDouble();

  {
    const QString layerId = element.attribute( QStringLiteral( "labeledLayer" ) );
    const QString layerName = element.attribute( QStringLiteral( "labeledLayerName" ) );
    const QString layerSource = element.attribute( QStringLiteral( "labeledLayerSource" ) );
    const QString layerProvider = element.attribute( QStringLiteral( "labeledLayerProvider" ) );
    mLabeledLayer = QgsVectorLayerRef( layerId, layerName, layerSource, layerProvider );
  }
  {
    const QString layerId = element.attribute( QStringLiteral( "targetLayer" ) );
    const QString layerName = element.attribute( QStringLiteral( "targetLayerName" ) );
    const QString layerSource = element.attribute( QStringLiteral( "targetLayerSource" ) );
    const QString layerProvider = element.attribute( QStringLiteral( "targetLayerProvider" ) );
    mTargetLayer = QgsVectorLayerRef( layerId, layerName, layerSource, layerProvider );
  }
}

void QgsAbstractLabelingEngineRuleDistanceFromFeature::resolveReferences( const QgsProject *project )
{
  mLabeledLayer.resolve( project );
  mTargetLayer.resolve( project );
}

bool QgsAbstractLabelingEngineRuleDistanceFromFeature::candidateIsIllegal( const pal::LabelPosition *candidate, QgsLabelingEngineContext &context ) const
{
  // hard blocks on candidates only apply when cost == 10
  if ( mCost < 10 )
    return false;

  if ( candidate->getFeaturePart()->feature()->provider()->layerId() != mLabeledLayer.layerId )
  {
    return false;
  }

  if ( !mTargetLayerSource )
    return false;

  return candidateExceedsTolerance( candidate, context );
}

void QgsAbstractLabelingEngineRuleDistanceFromFeature::alterCandidateCost( pal::LabelPosition *candidate, QgsLabelingEngineContext &context ) const
{
  // cost of 10 = hard block, handled in candidateIsIllegal
  if ( mCost >= 10 )
    return;

  if ( candidate->getFeaturePart()->feature()->provider()->layerId() != mLabeledLayer.layerId )
  {
    return;
  }

  if ( !mTargetLayerSource )
    return;

  if ( candidateExceedsTolerance( candidate, context ) )
  {
    // magic number alert! / 1000 here is completely arbitrary, an attempt to balance against the cost scaling of other factors
    // assigned by the inscrutible logic of the pal engine internals
    candidate->setCost( candidate->cost() + mCost / 1000 );
  }
}

QgsVectorLayer *QgsAbstractLabelingEngineRuleDistanceFromFeature::labeledLayer()
{
  return mLabeledLayer.get();
}

void QgsAbstractLabelingEngineRuleDistanceFromFeature::setLabeledLayer( QgsVectorLayer *layer )
{
  mLabeledLayer = layer;
}

QgsVectorLayer *QgsAbstractLabelingEngineRuleDistanceFromFeature::targetLayer()
{
  return mTargetLayer.get();
}

void QgsAbstractLabelingEngineRuleDistanceFromFeature::setTargetLayer( QgsVectorLayer *layer )
{
  mTargetLayer = layer;
}

void QgsAbstractLabelingEngineRuleDistanceFromFeature::copyCommonProperties( QgsAbstractLabelingEngineRuleDistanceFromFeature *other ) const
{
  other->mLabeledLayer = mLabeledLayer;
  other->mTargetLayer = mTargetLayer;
  other->mDistance = mDistance;
  other->mDistanceUnit = mDistanceUnit;
  other->mDistanceUnitScale = mDistanceUnitScale;
  other->mCost = mCost;
}

void QgsAbstractLabelingEngineRuleDistanceFromFeature::initialize( QgsLabelingEngineContext &context )
{
  QgsFeatureRequest req;
  req.setDestinationCrs( context.renderContext().coordinateTransform().destinationCrs(), context.renderContext().transformContext() );
  req.setFilterRect( context.extent() );
  req.setNoAttributes();

  QgsFeatureIterator it = mTargetLayerSource->getFeatures( req );

  mIndex = std::make_unique< QgsSpatialIndex >( it, context.renderContext().feedback(), QgsSpatialIndex::Flag::FlagStoreFeatureGeometries );

  mInitialized = true;
}

bool QgsAbstractLabelingEngineRuleDistanceFromFeature::candidateExceedsTolerance( const pal::LabelPosition *candidate, QgsLabelingEngineContext &context ) const
{
  if ( !mInitialized )
    const_cast< QgsAbstractLabelingEngineRuleDistanceFromFeature * >( this )->initialize( context );

  const QgsRectangle candidateBounds = candidate->boundingBox();
  const QgsRectangle expandedBounds = candidateBounds.buffered( mDistanceMapUnits );

  const QList<QgsFeatureId> overlapCandidates = mIndex->intersects( expandedBounds );
  if ( overlapCandidates.empty() )
    return !mMustBeDistant;

  GEOSContextHandle_t geosctxt = QgsGeosContext::get();

  const GEOSPreparedGeometry *candidateGeos = candidate->preparedMultiPartGeom();
  for ( const QgsFeatureId overlapCandidateId : overlapCandidates )
  {
    if ( context.renderContext().feedback() && context.renderContext().feedback()->isCanceled() )
      break;

    try
    {
      geos::unique_ptr featureCandidate = QgsGeos::asGeos( mIndex->geometry( overlapCandidateId ).constGet() );
#if GEOS_VERSION_MAJOR>3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR>=10 )
      if ( GEOSPreparedDistanceWithin_r( geosctxt, candidateGeos, featureCandidate.get(), mDistanceMapUnits ) )
      {
        return mMustBeDistant;
      }
#else
      QgsDebugError( QStringLiteral( "This rule requires GEOS 3.10+" ) );
      return false;
#endif
    }
    catch ( GEOSException &e )
    {
      QgsDebugError( QStringLiteral( "GEOS exception: %1" ).arg( e.what() ) );
    }
  }

  return !mMustBeDistant;
}

//
// QgsLabelingEngineRuleMinimumDistanceLabelToFeature
//

QgsLabelingEngineRuleMinimumDistanceLabelToFeature::QgsLabelingEngineRuleMinimumDistanceLabelToFeature() = default;
QgsLabelingEngineRuleMinimumDistanceLabelToFeature::~QgsLabelingEngineRuleMinimumDistanceLabelToFeature() = default;

QgsLabelingEngineRuleMinimumDistanceLabelToFeature *QgsLabelingEngineRuleMinimumDistanceLabelToFeature::clone() const
{
  std::unique_ptr< QgsLabelingEngineRuleMinimumDistanceLabelToFeature> res = std::make_unique< QgsLabelingEngineRuleMinimumDistanceLabelToFeature >();
  copyCommonProperties( res.get() );
  return res.release();
}

QString QgsLabelingEngineRuleMinimumDistanceLabelToFeature::id() const
{
  return QStringLiteral( "minimumDistanceLabelToFeature" );
}


//
// QgsLabelingEngineRuleMaximumDistanceLabelToFeature
//

QgsLabelingEngineRuleMaximumDistanceLabelToFeature::QgsLabelingEngineRuleMaximumDistanceLabelToFeature()
{
  mMustBeDistant = false;
}

QgsLabelingEngineRuleMaximumDistanceLabelToFeature::~QgsLabelingEngineRuleMaximumDistanceLabelToFeature() = default;

QgsLabelingEngineRuleMaximumDistanceLabelToFeature *QgsLabelingEngineRuleMaximumDistanceLabelToFeature::clone() const
{
  std::unique_ptr< QgsLabelingEngineRuleMaximumDistanceLabelToFeature > res = std::make_unique< QgsLabelingEngineRuleMaximumDistanceLabelToFeature >();
  copyCommonProperties( res.get() );
  return res.release();
}

QString QgsLabelingEngineRuleMaximumDistanceLabelToFeature::id() const
{
  return QStringLiteral( "maximumDistanceLabelToFeature" );
}


//
// QgsLabelingEngineRuleMinimumDistanceLabelToLabel
//

QgsLabelingEngineRuleMinimumDistanceLabelToLabel::QgsLabelingEngineRuleMinimumDistanceLabelToLabel() = default;
QgsLabelingEngineRuleMinimumDistanceLabelToLabel::~QgsLabelingEngineRuleMinimumDistanceLabelToLabel() = default;

QgsLabelingEngineRuleMinimumDistanceLabelToLabel *QgsLabelingEngineRuleMinimumDistanceLabelToLabel::clone() const
{
  std::unique_ptr< QgsLabelingEngineRuleMinimumDistanceLabelToLabel> res = std::make_unique< QgsLabelingEngineRuleMinimumDistanceLabelToLabel >();
  res->mLabeledLayer = mLabeledLayer;
  res->mTargetLayer = mTargetLayer;
  res->mDistance = mDistance;
  res->mDistanceUnit = mDistanceUnit;
  res->mDistanceUnitScale = mDistanceUnitScale;
  return res.release();
}

QString QgsLabelingEngineRuleMinimumDistanceLabelToLabel::id() const
{
  return QStringLiteral( "minimumDistanceLabelToLabel" );
}

void QgsLabelingEngineRuleMinimumDistanceLabelToLabel::writeXml( QDomDocument &, QDomElement &element, const QgsReadWriteContext & ) const
{
  element.setAttribute( QStringLiteral( "distance" ), mDistance );
  element.setAttribute( QStringLiteral( "distanceUnit" ), QgsUnitTypes::encodeUnit( mDistanceUnit ) );
  element.setAttribute( QStringLiteral( "distanceUnitScale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mDistanceUnitScale ) );

  if ( mLabeledLayer )
  {
    element.setAttribute( QStringLiteral( "labeledLayer" ), mLabeledLayer.layerId );
    element.setAttribute( QStringLiteral( "labeledLayerName" ), mLabeledLayer.name );
    element.setAttribute( QStringLiteral( "labeledLayerSource" ), mLabeledLayer.source );
    element.setAttribute( QStringLiteral( "labeledLayerProvider" ), mLabeledLayer.provider );
  }
  if ( mTargetLayer )
  {
    element.setAttribute( QStringLiteral( "targetLayer" ), mTargetLayer.layerId );
    element.setAttribute( QStringLiteral( "targetLayerName" ), mTargetLayer.name );
    element.setAttribute( QStringLiteral( "targetLayerSource" ), mTargetLayer.source );
    element.setAttribute( QStringLiteral( "targetLayerProvider" ), mTargetLayer.provider );
  }
}

void QgsLabelingEngineRuleMinimumDistanceLabelToLabel::readXml( const QDomElement &element, const QgsReadWriteContext & )
{
  mDistance = element.attribute( QStringLiteral( "distance" ), QStringLiteral( "0" ) ).toDouble();
  mDistanceUnit = QgsUnitTypes::decodeRenderUnit( element.attribute( QStringLiteral( "distanceUnit" ) ) );
  mDistanceUnitScale =  QgsSymbolLayerUtils::decodeMapUnitScale( element.attribute( QStringLiteral( "distanceUnitScale" ) ) );

  {
    const QString layerId = element.attribute( QStringLiteral( "labeledLayer" ) );
    const QString layerName = element.attribute( QStringLiteral( "labeledLayerName" ) );
    const QString layerSource = element.attribute( QStringLiteral( "labeledLayerSource" ) );
    const QString layerProvider = element.attribute( QStringLiteral( "labeledLayerProvider" ) );
    mLabeledLayer = QgsVectorLayerRef( layerId, layerName, layerSource, layerProvider );
  }
  {
    const QString layerId = element.attribute( QStringLiteral( "targetLayer" ) );
    const QString layerName = element.attribute( QStringLiteral( "targetLayerName" ) );
    const QString layerSource = element.attribute( QStringLiteral( "targetLayerSource" ) );
    const QString layerProvider = element.attribute( QStringLiteral( "targetLayerProvider" ) );
    mTargetLayer = QgsVectorLayerRef( layerId, layerName, layerSource, layerProvider );
  }
}

void QgsLabelingEngineRuleMinimumDistanceLabelToLabel::resolveReferences( const QgsProject *project )
{
  mLabeledLayer.resolve( project );
  mTargetLayer.resolve( project );
}

bool QgsLabelingEngineRuleMinimumDistanceLabelToLabel::prepare( QgsRenderContext &context )
{
  mDistanceMapUnits = context.convertToMapUnits( mDistance, mDistanceUnit, mDistanceUnitScale );
  return true;
}

QgsRectangle QgsLabelingEngineRuleMinimumDistanceLabelToLabel::modifyCandidateConflictSearchBoundingBox( const QgsRectangle &candidateBounds ) const
{
  return candidateBounds.buffered( mDistanceMapUnits );
}

bool QgsLabelingEngineRuleMinimumDistanceLabelToLabel::candidatesAreConflicting( const pal::LabelPosition *lp1, const pal::LabelPosition *lp2 ) const
{
  // conflicts are commutative -- we need to check both layers
  if (
    ( lp1->getFeaturePart()->feature()->provider()->layerId() == mLabeledLayer.layerId
      && lp2->getFeaturePart()->feature()->provider()->layerId() == mTargetLayer.layerId )
    ||
    ( lp2->getFeaturePart()->feature()->provider()->layerId() == mLabeledLayer.layerId
      && lp1->getFeaturePart()->feature()->provider()->layerId() == mTargetLayer.layerId )
  )
  {
    GEOSContextHandle_t geosctxt = QgsGeosContext::get();
    try
    {
#if GEOS_VERSION_MAJOR>3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR>=10 )
      if ( GEOSPreparedDistanceWithin_r( geosctxt, lp1->preparedMultiPartGeom(), lp2->multiPartGeom(), mDistanceMapUnits ) )
      {
        return true;
      }
#else
      QgsDebugError( QStringLiteral( "This rule requires GEOS 3.10+" ) );
      return false;
#endif
    }
    catch ( GEOSException &e )
    {
      QgsDebugError( QStringLiteral( "GEOS exception: %1" ).arg( e.what() ) );
    }
  }

  return false;
}

QgsVectorLayer *QgsLabelingEngineRuleMinimumDistanceLabelToLabel::labeledLayer()
{
  return mLabeledLayer.get();
}

void QgsLabelingEngineRuleMinimumDistanceLabelToLabel::setLabeledLayer( QgsVectorLayer *layer )
{
  mLabeledLayer = layer;
}

QgsVectorLayer *QgsLabelingEngineRuleMinimumDistanceLabelToLabel::targetLayer()
{
  return mTargetLayer.get();
}

void QgsLabelingEngineRuleMinimumDistanceLabelToLabel::setTargetLayer( QgsVectorLayer *layer )
{
  mTargetLayer = layer;
}


//
// QgsLabelingEngineRuleAvoidLabelOverlapWithFeature
//

QgsLabelingEngineRuleAvoidLabelOverlapWithFeature::QgsLabelingEngineRuleAvoidLabelOverlapWithFeature() = default;
QgsLabelingEngineRuleAvoidLabelOverlapWithFeature::~QgsLabelingEngineRuleAvoidLabelOverlapWithFeature() = default;

QgsLabelingEngineRuleAvoidLabelOverlapWithFeature *QgsLabelingEngineRuleAvoidLabelOverlapWithFeature::clone() const
{
  std::unique_ptr< QgsLabelingEngineRuleAvoidLabelOverlapWithFeature> res = std::make_unique< QgsLabelingEngineRuleAvoidLabelOverlapWithFeature >();
  res->mLabeledLayer = mLabeledLayer;
  res->mTargetLayer = mTargetLayer;
  return res.release();
}

QString QgsLabelingEngineRuleAvoidLabelOverlapWithFeature::id() const
{
  return QStringLiteral( "avoidLabelOverlapWithFeature" );
}

bool QgsLabelingEngineRuleAvoidLabelOverlapWithFeature::prepare( QgsRenderContext & )
{
  if ( !mTargetLayer )
    return false;

  QGIS_CHECK_OTHER_QOBJECT_THREAD_ACCESS( mTargetLayer );
  mTargetLayerSource = std::make_unique< QgsVectorLayerFeatureSource >( mTargetLayer.get() );
  return true;
}

void QgsLabelingEngineRuleAvoidLabelOverlapWithFeature::writeXml( QDomDocument &, QDomElement &element, const QgsReadWriteContext & ) const
{
  if ( mLabeledLayer )
  {
    element.setAttribute( QStringLiteral( "labeledLayer" ), mLabeledLayer.layerId );
    element.setAttribute( QStringLiteral( "labeledLayerName" ), mLabeledLayer.name );
    element.setAttribute( QStringLiteral( "labeledLayerSource" ), mLabeledLayer.source );
    element.setAttribute( QStringLiteral( "labeledLayerProvider" ), mLabeledLayer.provider );
  }
  if ( mTargetLayer )
  {
    element.setAttribute( QStringLiteral( "targetLayer" ), mTargetLayer.layerId );
    element.setAttribute( QStringLiteral( "targetLayerName" ), mTargetLayer.name );
    element.setAttribute( QStringLiteral( "targetLayerSource" ), mTargetLayer.source );
    element.setAttribute( QStringLiteral( "targetLayerProvider" ), mTargetLayer.provider );
  }
}

void QgsLabelingEngineRuleAvoidLabelOverlapWithFeature::readXml( const QDomElement &element, const QgsReadWriteContext & )
{
  {
    const QString layerId = element.attribute( QStringLiteral( "labeledLayer" ) );
    const QString layerName = element.attribute( QStringLiteral( "labeledLayerName" ) );
    const QString layerSource = element.attribute( QStringLiteral( "labeledLayerSource" ) );
    const QString layerProvider = element.attribute( QStringLiteral( "labeledLayerProvider" ) );
    mLabeledLayer = QgsVectorLayerRef( layerId, layerName, layerSource, layerProvider );
  }
  {
    const QString layerId = element.attribute( QStringLiteral( "targetLayer" ) );
    const QString layerName = element.attribute( QStringLiteral( "targetLayerName" ) );
    const QString layerSource = element.attribute( QStringLiteral( "targetLayerSource" ) );
    const QString layerProvider = element.attribute( QStringLiteral( "targetLayerProvider" ) );
    mTargetLayer = QgsVectorLayerRef( layerId, layerName, layerSource, layerProvider );
  }
}

void QgsLabelingEngineRuleAvoidLabelOverlapWithFeature::resolveReferences( const QgsProject *project )
{
  mLabeledLayer.resolve( project );
  mTargetLayer.resolve( project );
}

bool QgsLabelingEngineRuleAvoidLabelOverlapWithFeature::candidateIsIllegal( const pal::LabelPosition *candidate, QgsLabelingEngineContext &context ) const
{
  if ( candidate->getFeaturePart()->feature()->provider()->layerId() != mLabeledLayer.layerId )
  {
    return false;
  }

  if ( !mTargetLayerSource )
    return false;

  if ( !mInitialized )
    const_cast< QgsLabelingEngineRuleAvoidLabelOverlapWithFeature * >( this )->initialize( context );

  const QList<QgsFeatureId> overlapCandidates = mIndex->intersects( candidate->boundingBox() );
  if ( overlapCandidates.empty() )
    return false;

  GEOSContextHandle_t geosctxt = QgsGeosContext::get();

  const GEOSPreparedGeometry *candidateGeos = candidate->preparedMultiPartGeom();
  for ( const QgsFeatureId overlapCandidateId : overlapCandidates )
  {
    if ( context.renderContext().feedback() && context.renderContext().feedback()->isCanceled() )
      break;

    try
    {
      geos::unique_ptr featureCandidate = QgsGeos::asGeos( mIndex->geometry( overlapCandidateId ).constGet() );
      if ( GEOSPreparedIntersects_r( geosctxt, candidateGeos, featureCandidate.get() ) == 1 )
        return true;
    }
    catch ( GEOSException &e )
    {
      QgsDebugError( QStringLiteral( "GEOS exception: %1" ).arg( e.what() ) );
    }
  }

  return false;
}

QgsVectorLayer *QgsLabelingEngineRuleAvoidLabelOverlapWithFeature::labeledLayer()
{
  return mLabeledLayer.get();
}

void QgsLabelingEngineRuleAvoidLabelOverlapWithFeature::setLabeledLayer( QgsVectorLayer *layer )
{
  mLabeledLayer = layer;
}

QgsVectorLayer *QgsLabelingEngineRuleAvoidLabelOverlapWithFeature::targetLayer()
{
  return mTargetLayer.get();
}

void QgsLabelingEngineRuleAvoidLabelOverlapWithFeature::setTargetLayer( QgsVectorLayer *layer )
{
  mTargetLayer = layer;
}

void QgsLabelingEngineRuleAvoidLabelOverlapWithFeature::initialize( QgsLabelingEngineContext &context )
{
  QgsFeatureRequest req;
  req.setDestinationCrs( context.renderContext().coordinateTransform().destinationCrs(), context.renderContext().transformContext() );
  req.setFilterRect( context.extent() );
  req.setNoAttributes();

  QgsFeatureIterator it = mTargetLayerSource->getFeatures( req );

  mIndex = std::make_unique< QgsSpatialIndex >( it, context.renderContext().feedback(), QgsSpatialIndex::Flag::FlagStoreFeatureGeometries );

  mInitialized = true;
}
