/***************************************************************************
    qgsgeometryoverlapcheck.cpp
    ---------------------
    begin                : September 2015
    copyright            : (C) 2014 by Sandro Mani / Sourcepole AG
    email                : smani at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometrycheckcontext.h"
#include "qgsgeometryengine.h"
#include "qgsgeometryoverlapcheck.h"
#include "qgsfeaturepool.h"
#include "qgsvectorlayer.h"
#include "qgsfeedback.h"
#include "qgsapplication.h"

QgsGeometryOverlapCheck::QgsGeometryOverlapCheck( const QgsGeometryCheckContext *context, const QVariantMap &configuration )
  : QgsGeometryCheck( context, configuration )
  , mOverlapThresholdMapUnits( configurationValue<double>( QStringLiteral( "maxOverlapArea" ) ) )

{

}

void QgsGeometryOverlapCheck::collectErrors( const QMap<QString, QgsFeaturePool *> &featurePools, QList<QgsGeometryCheckError *> &errors, QStringList &messages, QgsFeedback *feedback, const LayerFeatureIds &ids ) const
{
  const QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds( featurePools ) : ids.toMap();
  const QgsGeometryCheckerUtils::LayerFeatures layerFeaturesA( featurePools, featureIds, compatibleGeometryTypes(), feedback, mContext, true );
  QList<QString> layerIds = featureIds.keys();
  for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeatureA : layerFeaturesA )
  {
    if ( feedback && feedback->isCanceled() )
      break;

    // Ensure each pair of layers only gets compared once: remove the current layer from the layerIds, but add it to the layerList for layerFeaturesB
    layerIds.removeOne( layerFeatureA.layer()->id() );

    const QgsGeometry geomA = layerFeatureA.geometry();
    const QgsRectangle bboxA = geomA.boundingBox();
    std::unique_ptr< QgsGeometryEngine > geomEngineA = QgsGeometryCheckerUtils::createGeomEngine( geomA.constGet(), mContext->tolerance );
    geomEngineA->prepareGeometry();
    if ( !geomEngineA->isValid() )
    {
      messages.append( tr( "Overlap check failed for (%1): the geometry is invalid" ).arg( layerFeatureA.id() ) );
      continue;
    }

    const QgsGeometryCheckerUtils::LayerFeatures layerFeaturesB( featurePools, QList<QString>() << layerFeatureA.layer()->id() << layerIds, bboxA, compatibleGeometryTypes(), mContext );
    for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeatureB : layerFeaturesB )
    {
      if ( feedback && feedback->isCanceled() )
        break;

      // > : only report overlaps within same layer once
      if ( layerFeatureA.layerId() == layerFeatureB.layerId() && layerFeatureB.feature().id() >= layerFeatureA.feature().id() )
      {
        continue;
      }

      QString errMsg;
      const QgsGeometry geometryB = layerFeatureB.geometry();
      const QgsAbstractGeometry *geomB = geometryB.constGet();
      if ( geomEngineA->overlaps( geomB, &errMsg ) )
      {
        std::unique_ptr<QgsAbstractGeometry> interGeom( geomEngineA->intersection( geomB ) );
        if ( interGeom && !interGeom->isEmpty() )
        {
          QgsGeometryCheckerUtils::filter1DTypes( interGeom.get() );
          for ( int iPart = 0, nParts = interGeom->partCount(); iPart < nParts; ++iPart )
          {
            QgsAbstractGeometry *interPart = QgsGeometryCheckerUtils::getGeomPart( interGeom.get(), iPart );
            const double area = interPart->area();
            if ( area > mContext->reducedTolerance && ( area < mOverlapThresholdMapUnits || mOverlapThresholdMapUnits == 0.0 ) )
            {
              errors.append( new QgsGeometryOverlapCheckError( this, layerFeatureA, QgsGeometry( interPart->clone() ), interPart->centroid(), area, layerFeatureB ) );
            }
          }
        }
        else if ( !errMsg.isEmpty() )
        {
          messages.append( tr( "Overlap check between features %1 and %2 %3" ).arg( layerFeatureA.id(), layerFeatureB.id(), errMsg ) );
        }
      }
    }
  }
}

void QgsGeometryOverlapCheck::fixError( const QMap<QString, QgsFeaturePool *> &featurePools, QgsGeometryCheckError *error, int method, const QMap<QString, int> & /*mergeAttributeIndices*/, Changes &changes ) const
{
  QString errMsg;
  QgsGeometryOverlapCheckError *overlapError = static_cast<QgsGeometryOverlapCheckError *>( error );

  QgsFeaturePool *featurePoolA = featurePools[ overlapError->layerId() ];
  QgsFeaturePool *featurePoolB = featurePools[ overlapError->overlappedFeature().layerId() ];
  QgsFeature featureA;
  QgsFeature featureB;
  if ( !featurePoolA->getFeature( overlapError->featureId(), featureA ) ||
       !featurePoolB->getFeature( overlapError->overlappedFeature().featureId(), featureB ) )
  {
    error->setObsolete();
    return;
  }

  // Check if error still applies
  const QgsGeometryCheckerUtils::LayerFeature layerFeatureA( featurePoolA, featureA, mContext, true );
  const QgsGeometryCheckerUtils::LayerFeature layerFeatureB( featurePoolB, featureB, mContext, true );
  const QgsGeometry geometryA = layerFeatureA.geometry();
  std::unique_ptr< QgsGeometryEngine > geomEngineA = QgsGeometryCheckerUtils::createGeomEngine( geometryA.constGet(), mContext->tolerance );
  geomEngineA->prepareGeometry();

  const QgsGeometry geometryB = layerFeatureB.geometry();
  if ( !geomEngineA->overlaps( geometryB.constGet() ) )
  {
    error->setObsolete();
    return;
  }
  std::unique_ptr< QgsAbstractGeometry > interGeom( geomEngineA->intersection( geometryB.constGet(), &errMsg ) );
  if ( !interGeom )
  {
    error->setFixFailed( tr( "Failed to compute intersection between overlapping features: %1" ).arg( errMsg ) );
    return;
  }

  // Search which overlap part this error parametrizes (using fuzzy-matching of the area and centroid...)
  QgsAbstractGeometry *interPart = nullptr;
  for ( int iPart = 0, nParts = interGeom->partCount(); iPart < nParts; ++iPart )
  {
    QgsAbstractGeometry *part = QgsGeometryCheckerUtils::getGeomPart( interGeom.get(), iPart );
    if ( std::fabs( part->area() - overlapError->value().toDouble() ) < mContext->reducedTolerance &&
         QgsGeometryCheckerUtils::pointsFuzzyEqual( part->centroid(), overlapError->location(), mContext->reducedTolerance ) )
    {
      interPart = part;
      break;
    }
  }
  if ( !interPart || interPart->isEmpty() )
  {
    error->setObsolete();
    return;
  }

  // Fix error
  if ( method == NoChange )
  {
    error->setFixed( method );
  }
  else if ( method == Subtract )
  {
    std::unique_ptr< QgsGeometryEngine > geomEngineDiffA = QgsGeometryCheckerUtils::createGeomEngine( geometryA.constGet(), 0 );
    std::unique_ptr< QgsAbstractGeometry > diff1( geomEngineDiffA->difference( interPart, &errMsg ) );
    if ( !diff1 || diff1->isEmpty() )
    {
      diff1.reset();
    }
    else
    {
      QgsGeometryCheckerUtils::filter1DTypes( diff1.get() );
    }
    std::unique_ptr< QgsGeometryEngine > geomEngineDiffB = QgsGeometryCheckerUtils::createGeomEngine( geometryB.constGet(), 0 );
    std::unique_ptr< QgsAbstractGeometry > diff2( geomEngineDiffB->difference( interPart, &errMsg ) );
    if ( !diff2 || diff2->isEmpty() )
    {
      diff2.reset();
    }
    else
    {
      QgsGeometryCheckerUtils::filter1DTypes( diff2.get() );
    }
    const double shared1 = diff1 ? QgsGeometryCheckerUtils::sharedEdgeLength( diff1.get(), interPart, mContext->reducedTolerance ) : 0;
    const double shared2 = diff2 ? QgsGeometryCheckerUtils::sharedEdgeLength( diff2.get(), interPart, mContext->reducedTolerance ) : 0;
    if ( !diff1 || !diff2 || shared1 == 0. || shared2 == 0. )
    {
      error->setFixFailed( tr( "Could not find shared edges between intersection and overlapping features" ) );
    }
    else
    {
      if ( shared1 < shared2 )
      {
        const QgsCoordinateTransform ct( featurePoolA->crs(), mContext->mapCrs, mContext->transformContext );
        diff1->transform( ct, Qgis::TransformDirection::Reverse );
        featureA.setGeometry( QgsGeometry( std::move( diff1 ) ) );

        changes[error->layerId()][featureA.id()].append( Change( ChangeFeature, ChangeChanged ) );
        featurePoolA->updateFeature( featureA );
      }
      else
      {
        const QgsCoordinateTransform ct( featurePoolB->crs(), mContext->mapCrs, mContext->transformContext );
        diff2->transform( ct, Qgis::TransformDirection::Reverse );
        featureB.setGeometry( QgsGeometry( std::move( diff2 ) ) );

        changes[overlapError->overlappedFeature().layerId()][featureB.id()].append( Change( ChangeFeature, ChangeChanged ) );
        featurePoolB->updateFeature( featureB );
      }

      error->setFixed( method );
    }
  }
  else
  {
    error->setFixFailed( tr( "Unknown method" ) );
  }
}

QStringList QgsGeometryOverlapCheck::resolutionMethods() const
{
  static const QStringList methods = QStringList()
                                     << tr( "Remove overlapping area from neighboring polygon with shortest shared edge" )
                                     << tr( "No action" );
  return methods;
}

QString QgsGeometryOverlapCheck::description() const
{
  return factoryDescription();
}

QString QgsGeometryOverlapCheck::id() const
{
  return factoryId();
}

QgsGeometryCheck::Flags QgsGeometryOverlapCheck::flags() const
{
  return factoryFlags();
}

///@cond private
QString QgsGeometryOverlapCheck::factoryDescription()
{
  return tr( "Overlap" );
}

QgsGeometryCheck::CheckType QgsGeometryOverlapCheck::factoryCheckType()
{
  return QgsGeometryCheck::LayerCheck;
}

QString QgsGeometryOverlapCheck::factoryId()
{
  return QStringLiteral( "QgsGeometryOverlapCheck" );
}

QgsGeometryCheck::Flags QgsGeometryOverlapCheck::factoryFlags()
{
  return QgsGeometryCheck::AvailableInValidation;
}

QList<QgsWkbTypes::GeometryType> QgsGeometryOverlapCheck::factoryCompatibleGeometryTypes()
{
  return {QgsWkbTypes::PolygonGeometry};
}

bool QgsGeometryOverlapCheck::factoryIsCompatible( QgsVectorLayer *layer ) SIP_SKIP
{
  return factoryCompatibleGeometryTypes().contains( layer->geometryType() );
}

///@endcond private
QgsGeometryOverlapCheckError::QgsGeometryOverlapCheckError( const QgsGeometryCheck *check, const QgsGeometryCheckerUtils::LayerFeature &layerFeature, const QgsGeometry &geometry, const QgsPointXY &errorLocation, const QVariant &value, const QgsGeometryCheckerUtils::LayerFeature &overlappedFeature )
  : QgsGeometryCheckError( check, layerFeature.layer()->id(), layerFeature.feature().id(), geometry, errorLocation, QgsVertexId(), value, ValueArea )
  , mOverlappedFeature( OverlappedFeature( overlappedFeature.layer(), overlappedFeature.feature().id() ) )
{

}

bool QgsGeometryOverlapCheckError::isEqual( QgsGeometryCheckError *other ) const
{
  QgsGeometryOverlapCheckError *err = dynamic_cast<QgsGeometryOverlapCheckError *>( other );
  return err &&
         other->layerId() == layerId() &&
         other->featureId() == featureId() &&
         err->overlappedFeature() == overlappedFeature() &&
         QgsGeometryCheckerUtils::pointsFuzzyEqual( location(), other->location(), mCheck->context()->reducedTolerance ) &&
         std::fabs( value().toDouble() - other->value().toDouble() ) < mCheck->context()->reducedTolerance;
}

bool QgsGeometryOverlapCheckError::closeMatch( QgsGeometryCheckError *other ) const
{
  QgsGeometryOverlapCheckError *err = dynamic_cast<QgsGeometryOverlapCheckError *>( other );
  return err && other->layerId() == layerId() && other->featureId() == featureId() && err->overlappedFeature() == overlappedFeature();
}

bool QgsGeometryOverlapCheckError::handleChanges( const QgsGeometryCheck::Changes &changes )
{
  if ( !QgsGeometryCheckError::handleChanges( changes ) )
  {
    return false;
  }
  if ( changes.value( mOverlappedFeature.layerId() ).contains( mOverlappedFeature.featureId() ) )
  {
    return false;
  }
  return true;
}

QString QgsGeometryOverlapCheckError::description() const
{
  return QCoreApplication::translate( "QgsGeometryTypeCheckError", "Overlap with %1 at feature %2" ).arg( mOverlappedFeature.layerName(), QString::number( mOverlappedFeature.featureId() ) );
}

QMap<QString, QgsFeatureIds> QgsGeometryOverlapCheckError::involvedFeatures() const
{
  QMap<QString, QgsFeatureIds> features;
  features[layerId()].insert( featureId() );
  features[mOverlappedFeature.layerId()].insert( mOverlappedFeature.featureId() );
  return features;
}

QIcon QgsGeometryOverlapCheckError::icon() const
{

  if ( status() == QgsGeometryCheckError::StatusFixed )
    return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmCheckGeometry.svg" ) );
  else
    return QgsApplication::getThemeIcon( QStringLiteral( "/checks/Overlap.svg" ) );
}
