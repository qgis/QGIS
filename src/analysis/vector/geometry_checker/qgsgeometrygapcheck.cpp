/***************************************************************************
    qgsgeometrygapcheck.cpp
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
#include "qgsgeometrygapcheck.h"
#include "qgsgeometrycollection.h"
#include "qgsfeaturepool.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerutils.h"
#include "qgsfeedback.h"
#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgsexpressioncontextutils.h"
#include "qgspolygon.h"
#include "qgscurve.h"
#include "qgssnappingutils.h"

QgsGeometryGapCheck::QgsGeometryGapCheck( const QgsGeometryCheckContext *context, const QVariantMap &configuration )
  : QgsGeometryCheck( context, configuration )
  ,  mGapThresholdMapUnits( configuration.value( QStringLiteral( "gapThreshold" ) ).toDouble() )
{
}

void QgsGeometryGapCheck::prepare( const QgsGeometryCheckContext *context, const QVariantMap &configuration )
{
  if ( configuration.value( QStringLiteral( "allowedGapsEnabled" ) ).toBool() )
  {
    QgsVectorLayer *layer = context->project()->mapLayer<QgsVectorLayer *>( configuration.value( "allowedGapsLayer" ).toString() );
    if ( layer )
    {
      mAllowedGapsLayer = layer;
      mAllowedGapsSource = std::make_unique<QgsVectorLayerFeatureSource>( layer );

      mAllowedGapsBuffer = configuration.value( QStringLiteral( "allowedGapsBuffer" ) ).toDouble();
    }
  }
  else
  {
    mAllowedGapsSource.reset();
  }
}

void QgsGeometryGapCheck::collectErrors( const QMap<QString, QgsFeaturePool *> &featurePools, QList<QgsGeometryCheckError *> &errors, QStringList &messages, QgsFeedback *feedback, const LayerFeatureIds &ids ) const
{
  if ( feedback )
    feedback->setProgress( feedback->progress() + 1.0 );

  std::unique_ptr<QgsAbstractGeometry> allowedGapsGeom;
  std::unique_ptr<QgsGeometryEngine> allowedGapsGeomEngine;

  if ( mAllowedGapsSource )
  {
    QVector<QgsGeometry> allowedGaps;
    QgsFeatureRequest request;
    request.setSubsetOfAttributes( QgsAttributeList() );
    QgsFeatureIterator iterator = mAllowedGapsSource->getFeatures( request );
    QgsFeature feature;

    while ( iterator.nextFeature( feature ) )
    {
      const QgsGeometry geom = feature.geometry();
      const QgsGeometry gg = geom.buffer( mAllowedGapsBuffer, 20 );
      allowedGaps.append( gg );
    }

    std::unique_ptr< QgsGeometryEngine > allowedGapsEngine = QgsGeometryCheckerUtils::createGeomEngine( nullptr, mContext->tolerance );

    // Create union of allowed gaps
    QString errMsg;
    allowedGapsGeom.reset( allowedGapsEngine->combine( allowedGaps, &errMsg ) );
    allowedGapsGeomEngine = QgsGeometryCheckerUtils::createGeomEngine( allowedGapsGeom.get(), mContext->tolerance );
    allowedGapsGeomEngine->prepareGeometry();
  }

  QVector<QgsGeometry> geomList;
  const QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds( featurePools ) : ids.toMap();
  const QgsGeometryCheckerUtils::LayerFeatures layerFeatures( featurePools, featureIds, compatibleGeometryTypes(), nullptr, mContext, true );
  for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeature : layerFeatures )
  {
    geomList.append( layerFeature.geometry() );

    if ( feedback && feedback->isCanceled() )
    {
      geomList.clear();
      break;
    }
  }

  if ( geomList.isEmpty() )
  {
    return;
  }

  std::unique_ptr< QgsGeometryEngine > geomEngine = QgsGeometryCheckerUtils::createGeomEngine( nullptr, mContext->tolerance );

  // Create union of geometry
  QString errMsg;
  const std::unique_ptr<QgsAbstractGeometry> unionGeom( geomEngine->combine( geomList, &errMsg ) );
  if ( !unionGeom )
  {
    messages.append( tr( "Gap check: %1" ).arg( errMsg ) );
    return;
  }

  // Get envelope of union
  geomEngine = QgsGeometryCheckerUtils::createGeomEngine( unionGeom.get(), mContext->tolerance );
  geomEngine->prepareGeometry();
  std::unique_ptr<QgsAbstractGeometry> envelope( geomEngine->envelope( &errMsg ) );
  if ( !envelope )
  {
    messages.append( tr( "Gap check: %1" ).arg( errMsg ) );
    return;
  }

  // Buffer envelope
  geomEngine = QgsGeometryCheckerUtils::createGeomEngine( envelope.get(), mContext->tolerance );
  geomEngine->prepareGeometry();
  QgsAbstractGeometry *bufEnvelope = geomEngine->buffer( 2, 0, Qgis::EndCapStyle::Square, Qgis::JoinStyle::Miter, 4. );  //#spellok  //#spellok
  envelope.reset( bufEnvelope );

  // Compute difference between envelope and union to obtain gap polygons
  geomEngine = QgsGeometryCheckerUtils::createGeomEngine( envelope.get(), mContext->tolerance );
  geomEngine->prepareGeometry();
  std::unique_ptr<QgsAbstractGeometry> diffGeom( geomEngine->difference( unionGeom.get(), &errMsg ) );
  if ( !diffGeom )
  {
    messages.append( tr( "Gap check: %1" ).arg( errMsg ) );
    return;
  }

  // For each gap polygon which does not lie on the boundary, get neighboring polygons and add error
  QgsGeometryPartIterator parts = diffGeom->parts();
  while ( parts.hasNext() )
  {
    const QgsAbstractGeometry *gapGeom = parts.next();
    // Skip the gap between features and boundingbox
    const double spacing = context()->tolerance;
    if ( gapGeom->boundingBox().snappedToGrid( spacing ) == envelope->boundingBox().snappedToGrid( spacing ) )
    {
      continue;
    }

    // Skip gaps above threshold
    if ( ( mGapThresholdMapUnits > 0 && gapGeom->area() > mGapThresholdMapUnits ) || gapGeom->area() < mContext->reducedTolerance )
    {
      continue;
    }

    QgsRectangle gapAreaBBox = gapGeom->boundingBox();

    // Get neighboring polygons
    QMap<QString, QgsFeatureIds> neighboringIds;
    const QgsGeometryCheckerUtils::LayerFeatures layerFeatures( featurePools, featureIds.keys(), gapAreaBBox, compatibleGeometryTypes(), mContext );
    std::unique_ptr< QgsGeometryEngine > gapGeomEngine = QgsGeometryCheckerUtils::createGeomEngine( gapGeom, mContext->tolerance );
    gapGeomEngine->prepareGeometry();
    for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeature : layerFeatures )
    {
      const QgsGeometry geom = layerFeature.geometry();
      if ( gapGeomEngine->distance( geom.constGet() ) < mContext->tolerance )
      {
        neighboringIds[layerFeature.layer()->id()].insert( layerFeature.feature().id() );
        gapAreaBBox.combineExtentWith( geom.boundingBox() );
      }
    }

    if ( neighboringIds.isEmpty() )
    {
      continue;
    }

    if ( allowedGapsGeomEngine && allowedGapsGeomEngine->contains( gapGeom ) )
    {
      continue;
    }

    // Add error
    const double area = gapGeom->area();
    const QgsRectangle gapBbox = gapGeom->boundingBox();
    errors.append( new QgsGeometryGapCheckError( this, QString(), QgsGeometry( gapGeom->clone() ), neighboringIds, area, gapBbox, gapAreaBBox ) );
  }
}

void QgsGeometryGapCheck::fixError( const QMap<QString, QgsFeaturePool *> &featurePools, QgsGeometryCheckError *error, int method, const QMap<QString, int> & /*mergeAttributeIndices*/, Changes &changes ) const
{
  const QMetaEnum metaEnum = QMetaEnum::fromType<QgsGeometryGapCheck::ResolutionMethod>();
  if ( !metaEnum.isValid() || !metaEnum.valueToKey( method ) )
  {
    error->setFixFailed( tr( "Unknown method" ) );
  }
  else
  {
    const ResolutionMethod methodValue = static_cast<ResolutionMethod>( method );
    switch ( methodValue )
    {
      case NoChange:
        error->setFixed( method );
        break;

      case MergeLongestEdge:
      {
        QString errMsg;
        if ( mergeWithNeighbor( featurePools, static_cast<QgsGeometryGapCheckError *>( error ), changes, errMsg, LongestSharedEdge ) )
        {
          error->setFixed( method );
        }
        else
        {
          error->setFixFailed( tr( "Failed to merge with neighbor: %1" ).arg( errMsg ) );
        }
        break;
      }

      case AddToAllowedGaps:
      {
        QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( mAllowedGapsLayer.data() );
        if ( layer )
        {
          if ( !layer->isEditable() && !layer->startEditing() )
          {
            error->setFixFailed( tr( "Could not start editing layer %1" ).arg( layer->name() ) );
          }
          else
          {
            const QgsFeature feature = QgsVectorLayerUtils::createFeature( layer, error->geometry() );
            QgsFeatureList features = QgsVectorLayerUtils::makeFeatureCompatible( feature, layer );
            if ( !layer->addFeatures( features ) )
            {
              error->setFixFailed( tr( "Could not add feature to layer %1" ).arg( layer->name() ) );
            }
            else
            {
              error->setFixed( method );
            }
          }
        }
        else
        {
          error->setFixFailed( tr( "Allowed gaps layer could not be resolved" ) );
        }
        break;
      }

      case CreateNewFeature:
      {
        QgsGeometryGapCheckError *gapCheckError = static_cast<QgsGeometryGapCheckError *>( error );
        QgsProject *project = QgsProject::instance();
        QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( project->mapLayer( gapCheckError->neighbors().keys().first() ) );
        if ( layer )
        {
          const QgsGeometry geometry = error->geometry();
          QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );
          QgsFeature feature = QgsVectorLayerUtils::createFeature( layer, geometry, QgsAttributeMap(), &context );
          if ( !layer->addFeature( feature ) )
          {
            error->setFixFailed( tr( "Could not add feature" ) );
          }
          else
          {
            error->setFixed( method );
          }
        }
        else
        {
          error->setFixFailed( tr( "Could not resolve target layer %1 to add feature" ).arg( error->layerId() ) );
        }
        break;
      }

      case MergeLargestArea:
      {
        QString errMsg;
        if ( mergeWithNeighbor( featurePools, static_cast<QgsGeometryGapCheckError *>( error ), changes, errMsg, LargestArea ) )
        {
          error->setFixed( method );
        }
        else
        {
          error->setFixFailed( tr( "Failed to merge with neighbor: %1" ).arg( errMsg ) );
        }
        break;
      }
    }
  }
}

bool QgsGeometryGapCheck::mergeWithNeighbor( const QMap<QString, QgsFeaturePool *> &featurePools, QgsGeometryGapCheckError *err, Changes &changes, QString &errMsg, Condition condition ) const
{
  double maxVal = 0.;
  QString mergeLayerId;
  QgsFeature mergeFeature;
  int mergePartIdx = -1;

  const QgsGeometry geometry = err->geometry();
  const QgsAbstractGeometry *errGeometry = QgsGeometryCheckerUtils::getGeomPart( geometry.constGet(), 0 );

  const auto layerIds = err->neighbors().keys();
  QList<QgsFeature> neighbours;

  // Search for touching neighboring geometries
  for ( const QString &layerId : layerIds )
  {
    QgsFeaturePool *featurePool = featurePools.value( layerId );
    std::unique_ptr<QgsAbstractGeometry> errLayerGeom( errGeometry->clone() );
    const QgsCoordinateTransform ct( featurePool->crs(), mContext->mapCrs, mContext->transformContext );
    errLayerGeom->transform( ct, Qgis::TransformDirection::Reverse );

    const auto featureIds = err->neighbors().value( layerId );

    for ( const QgsFeatureId testId : featureIds )
    {
      QgsFeature feature;
      if ( !featurePool->getFeature( testId, feature ) )
      {
        continue;
      }

      QgsGeometry transformedGeometry = feature.geometry();
      transformedGeometry.transform( ct );
      feature.setGeometry( transformedGeometry );
      neighbours.append( feature );
    }

    for ( const QgsFeature &testFeature : neighbours )
    {
      const QgsGeometry featureGeom = testFeature.geometry();
      const QgsAbstractGeometry *testGeom = featureGeom.constGet();
      for ( int iPart = 0, nParts = testGeom->partCount(); iPart < nParts; ++iPart )
      {
        double val = 0;
        switch ( condition )
        {
          case LongestSharedEdge:
            val = QgsGeometryCheckerUtils::sharedEdgeLength( errLayerGeom.get(), QgsGeometryCheckerUtils::getGeomPart( testGeom, iPart ), mContext->reducedTolerance );
            break;

          case LargestArea:
            // We might get a neighbour where we touch only a corner
            if ( QgsGeometryCheckerUtils::sharedEdgeLength( errLayerGeom.get(), QgsGeometryCheckerUtils::getGeomPart( testGeom, iPart ), mContext->reducedTolerance ) > 0 )
              val = QgsGeometryCheckerUtils::getGeomPart( testGeom, iPart )->area();
            break;
        }

        if ( val > maxVal )
        {
          maxVal = val;
          mergeFeature = testFeature;
          mergePartIdx = iPart;
          mergeLayerId = layerId;
        }
      }
    }
  }

  if ( maxVal == 0. )
  {
    return false;
  }

  // Create an index of all neighbouring vertices
  QgsSpatialIndex neighbourVerticesIndex( QgsSpatialIndex::Flag::FlagStoreFeatureGeometries );
  int id = 0;
  for ( const QgsFeature &neighbour : neighbours )
  {
    QgsVertexIterator vit = neighbour.geometry().vertices();
    while ( vit.hasNext() )
    {
      const QgsPoint pt = vit.next();
      QgsFeature f;
      f.setId( id ); // required for SpatialIndex to return the correct result
      f.setGeometry( QgsGeometry( pt.clone() ) );
      neighbourVerticesIndex.addFeature( f );
      id++;
    }
  }

  // Snap to the closest vertex
  QgsPolyline snappedRing;
  QgsVertexIterator iterator = errGeometry->vertices();
  while ( iterator.hasNext() )
  {
    const QgsPoint pt = iterator.next();
    const QgsGeometry closestGeom = neighbourVerticesIndex.geometry( neighbourVerticesIndex.nearestNeighbor( QgsPointXY( pt ) ).first() );
    if ( !closestGeom.isEmpty() )
    {
      snappedRing.append( QgsPoint( closestGeom.vertexAt( 0 ) ) );
    }
  }

  std::unique_ptr<QgsPolygon> snappedErrGeom = std::make_unique<QgsPolygon>();
  snappedErrGeom->setExteriorRing( new QgsLineString( snappedRing ) );

  // Merge geometries
  QgsFeaturePool *featurePool = featurePools[ mergeLayerId ];
  std::unique_ptr<QgsAbstractGeometry> errLayerGeom( snappedErrGeom->clone() );
  const QgsCoordinateTransform ct( featurePool->crs(), mContext->mapCrs, mContext->transformContext );
  errLayerGeom->transform( ct, Qgis::TransformDirection::Reverse );
  const QgsGeometry mergeFeatureGeom = mergeFeature.geometry();
  const QgsAbstractGeometry *mergeGeom = mergeFeatureGeom.constGet();
  std::unique_ptr< QgsGeometryEngine > geomEngine = QgsGeometryCheckerUtils::createGeomEngine( errLayerGeom.get(), 0 );
  std::unique_ptr<QgsAbstractGeometry> combinedGeom( geomEngine->combine( QgsGeometryCheckerUtils::getGeomPart( mergeGeom, mergePartIdx ), &errMsg ) );
  if ( !combinedGeom || combinedGeom->isEmpty() || !QgsWkbTypes::isSingleType( combinedGeom->wkbType() ) )
  {
    return false;
  }

  // Add merged polygon to destination geometry
  replaceFeatureGeometryPart( featurePools, mergeLayerId, mergeFeature, mergePartIdx, combinedGeom.release(), changes );

  return true;
}


QStringList QgsGeometryGapCheck::resolutionMethods() const
{
  QStringList methods = QStringList()
                        << tr( "Add gap area to neighboring polygon with longest shared edge" )
                        << tr( "No action" );
  if ( mAllowedGapsSource )
    methods << tr( "Add gap to allowed exceptions" );

  return methods;
}

QList<QgsGeometryCheckResolutionMethod> QgsGeometryGapCheck::availableResolutionMethods() const
{
  QList<QgsGeometryCheckResolutionMethod> fixes
  {
    QgsGeometryCheckResolutionMethod( MergeLongestEdge, tr( "Add to longest shared edge" ), tr( "Add the gap area to the neighbouring polygon with the longest shared edge." ), false ),
    QgsGeometryCheckResolutionMethod( CreateNewFeature, tr( "Create new feature" ), tr( "Create a new feature from the gap area." ), false ),
    QgsGeometryCheckResolutionMethod( MergeLargestArea, tr( "Add to largest neighbouring area" ), tr( "Add the gap area to the neighbouring polygon with the largest area." ), false )
  };

  if ( mAllowedGapsSource )
    fixes << QgsGeometryCheckResolutionMethod( AddToAllowedGaps, tr( "Add Gap to Allowed Exceptions" ), tr( "Create a new feature from the gap geometry on the allowed exceptions layer." ), true );

  fixes << QgsGeometryCheckResolutionMethod( NoChange, tr( "No action" ), tr( "Do not perform any action and mark this error as fixed." ), false );

  return fixes;
}

QString QgsGeometryGapCheck::description() const
{
  return factoryDescription();
}

QString QgsGeometryGapCheck::id() const
{
  return factoryId();
}

QgsGeometryCheck::Flags QgsGeometryGapCheck::flags() const
{
  return factoryFlags();
}

///@cond private
QString QgsGeometryGapCheck::factoryDescription()
{
  return tr( "Gap" );
}

QString QgsGeometryGapCheck::factoryId()
{
  return QStringLiteral( "QgsGeometryGapCheck" );
}

QgsGeometryCheck::Flags QgsGeometryGapCheck::factoryFlags()
{
  return QgsGeometryCheck::AvailableInValidation;
}

QList<QgsWkbTypes::GeometryType> QgsGeometryGapCheck::factoryCompatibleGeometryTypes()
{
  return {QgsWkbTypes::PolygonGeometry};
}

bool QgsGeometryGapCheck::factoryIsCompatible( QgsVectorLayer *layer ) SIP_SKIP
{
  return factoryCompatibleGeometryTypes().contains( layer->geometryType() );
}

QgsGeometryCheck::CheckType QgsGeometryGapCheck::factoryCheckType()
{
  return QgsGeometryCheck::LayerCheck;
}
///@endcond private

QgsRectangle QgsGeometryGapCheckError::contextBoundingBox() const
{
  return mContextBoundingBox;
}

bool QgsGeometryGapCheckError::isEqual( QgsGeometryCheckError *other ) const
{
  QgsGeometryGapCheckError *err = dynamic_cast<QgsGeometryGapCheckError *>( other );
  return err && QgsGeometryCheckerUtils::pointsFuzzyEqual( err->location(), location(), mCheck->context()->reducedTolerance ) && err->neighbors() == neighbors();
}

bool QgsGeometryGapCheckError::closeMatch( QgsGeometryCheckError *other ) const
{
  QgsGeometryGapCheckError *err = dynamic_cast<QgsGeometryGapCheckError *>( other );
  return err && err->layerId() == layerId() && err->neighbors() == neighbors();
}

void QgsGeometryGapCheckError::update( const QgsGeometryCheckError *other )
{
  QgsGeometryCheckError::update( other );
  // Static cast since this should only get called if isEqual == true
  const QgsGeometryGapCheckError *err = static_cast<const QgsGeometryGapCheckError *>( other );
  mNeighbors = err->mNeighbors;
  mGapAreaBBox = err->mGapAreaBBox;
}

bool QgsGeometryGapCheckError::handleChanges( const QgsGeometryCheck::Changes & )
{
  return true;
}

QgsRectangle QgsGeometryGapCheckError::affectedAreaBBox() const
{
  return mGapAreaBBox;
}

QMap<QString, QgsFeatureIds> QgsGeometryGapCheckError::involvedFeatures() const
{
  return mNeighbors;
}

QIcon QgsGeometryGapCheckError::icon() const
{

  if ( status() == QgsGeometryCheckError::StatusFixed )
    return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmCheckGeometry.svg" ) );
  else
    return QgsApplication::getThemeIcon( QStringLiteral( "/checks/SliverOrGap.svg" ) );
}
