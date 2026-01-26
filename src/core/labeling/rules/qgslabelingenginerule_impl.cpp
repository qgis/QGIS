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

#include "feature.h"
#include "labelposition.h"
#include "qgsgeos.h"
#include "qgsspatialindex.h"
#include "qgssymbollayerutils.h"
#include "qgsthreadingutils.h"
#include "qgsunittypes.h"
#include "qgsvectorlayerfeatureiterator.h"

#if GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR < 10
#include "qgsmessagelog.h"
#endif

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
  element.setAttribute( u"distance"_s, mDistance );
  element.setAttribute( u"distanceUnit"_s, QgsUnitTypes::encodeUnit( mDistanceUnit ) );
  element.setAttribute( u"distanceUnitScale"_s, QgsSymbolLayerUtils::encodeMapUnitScale( mDistanceUnitScale ) );
  element.setAttribute( u"cost"_s, mCost );

  if ( mLabeledLayer )
  {
    element.setAttribute( u"labeledLayer"_s, mLabeledLayer.layerId );
    element.setAttribute( u"labeledLayerName"_s, mLabeledLayer.name );
    element.setAttribute( u"labeledLayerSource"_s, mLabeledLayer.source );
    element.setAttribute( u"labeledLayerProvider"_s, mLabeledLayer.provider );
  }
  if ( mTargetLayer )
  {
    element.setAttribute( u"targetLayer"_s, mTargetLayer.layerId );
    element.setAttribute( u"targetLayerName"_s, mTargetLayer.name );
    element.setAttribute( u"targetLayerSource"_s, mTargetLayer.source );
    element.setAttribute( u"targetLayerProvider"_s, mTargetLayer.provider );
  }
}

void QgsAbstractLabelingEngineRuleDistanceFromFeature::readXml( const QDomElement &element, const QgsReadWriteContext & )
{
  mDistance = element.attribute( u"distance"_s, u"5"_s ).toDouble();
  mDistanceUnit = QgsUnitTypes::decodeRenderUnit( element.attribute( u"distanceUnit"_s ) );
  mDistanceUnitScale =  QgsSymbolLayerUtils::decodeMapUnitScale( element.attribute( u"distanceUnitScale"_s ) );
  mCost = element.attribute( u"cost"_s, u"10"_s ).toDouble();

  {
    const QString layerId = element.attribute( u"labeledLayer"_s );
    const QString layerName = element.attribute( u"labeledLayerName"_s );
    const QString layerSource = element.attribute( u"labeledLayerSource"_s );
    const QString layerProvider = element.attribute( u"labeledLayerProvider"_s );
    mLabeledLayer = QgsMapLayerRef( layerId, layerName, layerSource, layerProvider );
  }
  {
    const QString layerId = element.attribute( u"targetLayer"_s );
    const QString layerName = element.attribute( u"targetLayerName"_s );
    const QString layerSource = element.attribute( u"targetLayerSource"_s );
    const QString layerProvider = element.attribute( u"targetLayerProvider"_s );
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

bool QgsAbstractLabelingEngineRuleDistanceFromFeature::isAvailable() const
{
#if GEOS_VERSION_MAJOR>3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR>=10 )
  return true;
#else
  return false;
#endif
}

QgsMapLayer *QgsAbstractLabelingEngineRuleDistanceFromFeature::labeledLayer() const
{
  return mLabeledLayer.get();
}

void QgsAbstractLabelingEngineRuleDistanceFromFeature::setLabeledLayer( QgsMapLayer *layer )
{
  mLabeledLayer = layer;
}

QgsVectorLayer *QgsAbstractLabelingEngineRuleDistanceFromFeature::targetLayer() const
{
  return mTargetLayer.get();
}

void QgsAbstractLabelingEngineRuleDistanceFromFeature::setTargetLayer( QgsVectorLayer *layer )
{
  mTargetLayer = layer;
}

void QgsAbstractLabelingEngineRuleDistanceFromFeature::copyCommonProperties( QgsAbstractLabelingEngineRule *other ) const
{
  QgsAbstractLabelingEngineRule::copyCommonProperties( other );
  if ( QgsAbstractLabelingEngineRuleDistanceFromFeature *otherRule = dynamic_cast< QgsAbstractLabelingEngineRuleDistanceFromFeature * >( other ) )
  {
    otherRule->mLabeledLayer = mLabeledLayer;
    otherRule->mTargetLayer = mTargetLayer;
    otherRule->mDistance = mDistance;
    otherRule->mDistanceUnit = mDistanceUnit;
    otherRule->mDistanceUnitScale = mDistanceUnitScale;
    otherRule->mCost = mCost;
  }
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

  const QgsRectangle candidateBounds = candidate->outerBoundingBox();
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
      QgsMessageLog::logMessage( u"The %1 labeling rule requires GEOS 3.10+"_s.arg( name().isEmpty() ? displayType() : name() ) );
      return false;
#endif
    }
    catch ( QgsGeosException &e )
    {
      QgsDebugError( u"GEOS exception: %1"_s.arg( e.what() ) );
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
  auto res = std::make_unique< QgsLabelingEngineRuleMinimumDistanceLabelToFeature >();
  copyCommonProperties( res.get() );
  return res.release();
}

QString QgsLabelingEngineRuleMinimumDistanceLabelToFeature::id() const
{
  return u"minimumDistanceLabelToFeature"_s;
}

QString QgsLabelingEngineRuleMinimumDistanceLabelToFeature::displayType() const
{
  return QObject::tr( "Push Labels Away from Features" );
}

QString QgsLabelingEngineRuleMinimumDistanceLabelToFeature::description() const
{
  QString res = u"<b>%1</b>"_s.arg( name().isEmpty() ? displayType() : name() );
  if ( labeledLayer() && targetLayer() )
  {
    res += u"<p>"_s + QObject::tr( "Labels from <i>%1</i> must be at least %2 %3 from features in <i>%4</i>" ).arg(
             labeledLayer()->name(),
             QString::number( distance() ),
             QgsUnitTypes::toAbbreviatedString( distanceUnit() ),
             targetLayer()->name()
           ) + u"</p>"_s;
  }
  return res;
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
  auto res = std::make_unique< QgsLabelingEngineRuleMaximumDistanceLabelToFeature >();
  copyCommonProperties( res.get() );
  return res.release();
}

QString QgsLabelingEngineRuleMaximumDistanceLabelToFeature::id() const
{
  return u"maximumDistanceLabelToFeature"_s;
}

QString QgsLabelingEngineRuleMaximumDistanceLabelToFeature::displayType() const
{
  return QObject::tr( "Pull Labels Toward Features" );
}

QString QgsLabelingEngineRuleMaximumDistanceLabelToFeature::description() const
{
  QString res = u"<b>%1</b>"_s.arg( name().isEmpty() ? displayType() : name() );
  if ( labeledLayer() && targetLayer() )
  {
    res += u"<p>"_s + QObject::tr( "Labels from <i>%1</i> must be at most %2 %3 from features in <i>%4</i>" ).arg(
             labeledLayer()->name(),
             QString::number( distance() ),
             QgsUnitTypes::toAbbreviatedString( distanceUnit() ),
             targetLayer()->name()
           ) + u"</p>"_s;
  }
  return res;
}


//
// QgsLabelingEngineRuleMinimumDistanceLabelToLabel
//

QgsLabelingEngineRuleMinimumDistanceLabelToLabel::QgsLabelingEngineRuleMinimumDistanceLabelToLabel() = default;
QgsLabelingEngineRuleMinimumDistanceLabelToLabel::~QgsLabelingEngineRuleMinimumDistanceLabelToLabel() = default;

QgsLabelingEngineRuleMinimumDistanceLabelToLabel *QgsLabelingEngineRuleMinimumDistanceLabelToLabel::clone() const
{
  auto res = std::make_unique< QgsLabelingEngineRuleMinimumDistanceLabelToLabel >();
  copyCommonProperties( res.get() );
  res->mLabeledLayer = mLabeledLayer;
  res->mTargetLayer = mTargetLayer;
  res->mDistance = mDistance;
  res->mDistanceUnit = mDistanceUnit;
  res->mDistanceUnitScale = mDistanceUnitScale;
  return res.release();
}

QString QgsLabelingEngineRuleMinimumDistanceLabelToLabel::id() const
{
  return u"minimumDistanceLabelToLabel"_s;
}

QString QgsLabelingEngineRuleMinimumDistanceLabelToLabel::displayType() const
{
  return QObject::tr( "Push Labels Away from Other Labels" );
}

QString QgsLabelingEngineRuleMinimumDistanceLabelToLabel::description() const
{
  QString res = u"<b>%1</b>"_s.arg( name().isEmpty() ? displayType() : name() );
  if ( labeledLayer() && targetLayer() )
  {
    res += u"<p>"_s + QObject::tr( "Labels from <i>%1</i> must be at least %2 %3 from labels from <i>%4</i>" ).arg(
             labeledLayer()->name(),
             QString::number( distance() ),
             QgsUnitTypes::toAbbreviatedString( distanceUnit() ),
             targetLayer()->name()
           ) + u"</p>"_s;
  }
  return res;
}

bool QgsLabelingEngineRuleMinimumDistanceLabelToLabel::isAvailable() const
{
#if GEOS_VERSION_MAJOR>3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR>=10 )
  return true;
#else
  return false;
#endif
}

void QgsLabelingEngineRuleMinimumDistanceLabelToLabel::writeXml( QDomDocument &, QDomElement &element, const QgsReadWriteContext & ) const
{
  element.setAttribute( u"distance"_s, mDistance );
  element.setAttribute( u"distanceUnit"_s, QgsUnitTypes::encodeUnit( mDistanceUnit ) );
  element.setAttribute( u"distanceUnitScale"_s, QgsSymbolLayerUtils::encodeMapUnitScale( mDistanceUnitScale ) );

  if ( mLabeledLayer )
  {
    element.setAttribute( u"labeledLayer"_s, mLabeledLayer.layerId );
    element.setAttribute( u"labeledLayerName"_s, mLabeledLayer.name );
    element.setAttribute( u"labeledLayerSource"_s, mLabeledLayer.source );
    element.setAttribute( u"labeledLayerProvider"_s, mLabeledLayer.provider );
  }
  if ( mTargetLayer )
  {
    element.setAttribute( u"targetLayer"_s, mTargetLayer.layerId );
    element.setAttribute( u"targetLayerName"_s, mTargetLayer.name );
    element.setAttribute( u"targetLayerSource"_s, mTargetLayer.source );
    element.setAttribute( u"targetLayerProvider"_s, mTargetLayer.provider );
  }
}

void QgsLabelingEngineRuleMinimumDistanceLabelToLabel::readXml( const QDomElement &element, const QgsReadWriteContext & )
{
  mDistance = element.attribute( u"distance"_s, u"5"_s ).toDouble();
  mDistanceUnit = QgsUnitTypes::decodeRenderUnit( element.attribute( u"distanceUnit"_s ) );
  mDistanceUnitScale =  QgsSymbolLayerUtils::decodeMapUnitScale( element.attribute( u"distanceUnitScale"_s ) );

  {
    const QString layerId = element.attribute( u"labeledLayer"_s );
    const QString layerName = element.attribute( u"labeledLayerName"_s );
    const QString layerSource = element.attribute( u"labeledLayerSource"_s );
    const QString layerProvider = element.attribute( u"labeledLayerProvider"_s );
    mLabeledLayer = QgsMapLayerRef( layerId, layerName, layerSource, layerProvider );
  }
  {
    const QString layerId = element.attribute( u"targetLayer"_s );
    const QString layerName = element.attribute( u"targetLayerName"_s );
    const QString layerSource = element.attribute( u"targetLayerSource"_s );
    const QString layerProvider = element.attribute( u"targetLayerProvider"_s );
    mTargetLayer = QgsMapLayerRef( layerId, layerName, layerSource, layerProvider );
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
      QgsMessageLog::logMessage( u"The %1 labeling rule requires GEOS 3.10+"_s.arg( name().isEmpty() ? displayType() : name() ) );
      return false;
#endif
    }
    catch ( QgsGeosException &e )
    {
      QgsDebugError( u"GEOS exception: %1"_s.arg( e.what() ) );
    }
  }

  return false;
}

QgsMapLayer *QgsLabelingEngineRuleMinimumDistanceLabelToLabel::labeledLayer() const
{
  return mLabeledLayer.get();
}

void QgsLabelingEngineRuleMinimumDistanceLabelToLabel::setLabeledLayer( QgsMapLayer *layer )
{
  mLabeledLayer = layer;
}

QgsMapLayer *QgsLabelingEngineRuleMinimumDistanceLabelToLabel::targetLayer() const
{
  return mTargetLayer.get();
}

void QgsLabelingEngineRuleMinimumDistanceLabelToLabel::setTargetLayer( QgsMapLayer *layer )
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
  auto res = std::make_unique< QgsLabelingEngineRuleAvoidLabelOverlapWithFeature >();
  copyCommonProperties( res.get() );
  res->mLabeledLayer = mLabeledLayer;
  res->mTargetLayer = mTargetLayer;
  return res.release();
}

QString QgsLabelingEngineRuleAvoidLabelOverlapWithFeature::id() const
{
  return u"avoidLabelOverlapWithFeature"_s;
}

QString QgsLabelingEngineRuleAvoidLabelOverlapWithFeature::displayType() const
{
  return QObject::tr( "Prevent Labels Overlapping Features" );
}

QString QgsLabelingEngineRuleAvoidLabelOverlapWithFeature::description() const
{
  QString res = u"<b>%1</b>"_s.arg( name().isEmpty() ? displayType() : name() );
  if ( labeledLayer() && targetLayer() )
  {
    res += u"<p>"_s + QObject::tr( "Labels from <i>%1</i> must not overlap features from <i>%2</i>" ).arg(
             labeledLayer()->name(),
             targetLayer()->name()
           ) + u"</p>"_s;
  }
  return res;
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
    element.setAttribute( u"labeledLayer"_s, mLabeledLayer.layerId );
    element.setAttribute( u"labeledLayerName"_s, mLabeledLayer.name );
    element.setAttribute( u"labeledLayerSource"_s, mLabeledLayer.source );
    element.setAttribute( u"labeledLayerProvider"_s, mLabeledLayer.provider );
  }
  if ( mTargetLayer )
  {
    element.setAttribute( u"targetLayer"_s, mTargetLayer.layerId );
    element.setAttribute( u"targetLayerName"_s, mTargetLayer.name );
    element.setAttribute( u"targetLayerSource"_s, mTargetLayer.source );
    element.setAttribute( u"targetLayerProvider"_s, mTargetLayer.provider );
  }
}

void QgsLabelingEngineRuleAvoidLabelOverlapWithFeature::readXml( const QDomElement &element, const QgsReadWriteContext & )
{
  {
    const QString layerId = element.attribute( u"labeledLayer"_s );
    const QString layerName = element.attribute( u"labeledLayerName"_s );
    const QString layerSource = element.attribute( u"labeledLayerSource"_s );
    const QString layerProvider = element.attribute( u"labeledLayerProvider"_s );
    mLabeledLayer = QgsMapLayerRef( layerId, layerName, layerSource, layerProvider );
  }
  {
    const QString layerId = element.attribute( u"targetLayer"_s );
    const QString layerName = element.attribute( u"targetLayerName"_s );
    const QString layerSource = element.attribute( u"targetLayerSource"_s );
    const QString layerProvider = element.attribute( u"targetLayerProvider"_s );
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

  const QList<QgsFeatureId> overlapCandidates = mIndex->intersects( candidate->outerBoundingBox() );
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
    catch ( QgsGeosException &e )
    {
      QgsDebugError( u"GEOS exception: %1"_s.arg( e.what() ) );
    }
  }

  return false;
}

QgsMapLayer *QgsLabelingEngineRuleAvoidLabelOverlapWithFeature::labeledLayer() const
{
  return mLabeledLayer.get();
}

void QgsLabelingEngineRuleAvoidLabelOverlapWithFeature::setLabeledLayer( QgsMapLayer *layer )
{
  mLabeledLayer = layer;
}

QgsVectorLayer *QgsLabelingEngineRuleAvoidLabelOverlapWithFeature::targetLayer() const
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
