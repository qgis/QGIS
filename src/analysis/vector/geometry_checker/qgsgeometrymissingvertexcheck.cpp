/***************************************************************************
    qgsgeometrymissingvertexcheck.cpp
    ---------------------
    begin                : September 2018
    copyright            : (C) 2018 Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometrymissingvertexcheck.h"

#include "qgsapplication.h"
#include "qgscurve.h"
#include "qgscurvepolygon.h"
#include "qgsfeedback.h"
#include "qgsgeometrycollection.h"
#include "qgsgeometryengine.h"
#include "qgsgeometryutils.h"
#include "qgsmultipolygon.h"

#include "moc_qgsgeometrymissingvertexcheck.cpp"

QgsGeometryMissingVertexCheck::QgsGeometryMissingVertexCheck( const QgsGeometryCheckContext *context, const QVariantMap &geometryCheckConfiguration )
  : QgsGeometryCheck( context, geometryCheckConfiguration )

{}

QgsGeometryCheck::Result QgsGeometryMissingVertexCheck::collectErrors( const QMap<QString, QgsFeaturePool *> &featurePools, QList<QgsGeometryCheckError *> &errors, QStringList &messages, QgsFeedback *feedback, const LayerFeatureIds &ids ) const
{
  Q_UNUSED( messages )
  if ( feedback )
    feedback->setProgress( feedback->progress() + 1.0 );

  QMap<QString, QSet<QVariant>> uniqueIds;
  QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds( featurePools ) : ids.toMap();
  QgsFeaturePool *featurePool = featurePools.value( featureIds.firstKey() );
  if ( !featurePool )
  {
    QgsDebugError( u"Could not retrieve feature pool for %1"_s.arg( featureIds.firstKey() ) );
    return QgsGeometryCheck::Result::Canceled;
  }
  const QgsGeometryCheckerUtils::LayerFeatures layerFeatures( featurePools, featureIds, compatibleGeometryTypes(), nullptr, mContext, true );
  for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeature : layerFeatures )
  {
    if ( feedback && feedback->isCanceled() )
    {
      return QgsGeometryCheck::Result::Canceled;
    }

    if ( context()->uniqueIdFieldIndex != -1 )
    {
      QgsGeometryCheck::Result result = checkUniqueId( layerFeature, uniqueIds );
      if ( result != QgsGeometryCheck::Result::Success )
      {
        return result;
      }
    }

    const QgsGeometry geometry = layerFeature.geometry();
    const QgsAbstractGeometry *geom = geometry.constGet();

    if ( const QgsCurvePolygon *polygon = qgsgeometry_cast<const QgsCurvePolygon *>( geom ) )
    {
      processPolygon( polygon, featurePool, errors, layerFeature, feedback );
    }
    else if ( const QgsGeometryCollection *collection = qgsgeometry_cast<const QgsGeometryCollection *>( geom ) )
    {
      const int numGeometries = collection->numGeometries();
      for ( int i = 0; i < numGeometries; ++i )
      {
        if ( const QgsCurvePolygon *polygon = qgsgeometry_cast<const QgsCurvePolygon *>( collection->geometryN( i ) ) )
        {
          processPolygon( polygon, featurePool, errors, layerFeature, feedback );
        }
      }
    }
  }
  return QgsGeometryCheck::Result::Success;
}

void QgsGeometryMissingVertexCheck::fixError( const QMap<QString, QgsFeaturePool *> &featurePools, QgsGeometryCheckError *error, int method, const QMap<QString, int> & /*mergeAttributeIndices*/, Changes &changes ) const
{
  Q_UNUSED( featurePools )
  Q_UNUSED( changes )

  QMetaEnum metaEnum = QMetaEnum::fromType<QgsGeometryMissingVertexCheck::ResolutionMethod>();
  if ( !metaEnum.isValid() || !metaEnum.valueToKey( method ) )
  {
    error->setFixFailed( tr( "Unknown method" ) );
  }
  else
  {
    ResolutionMethod methodValue = static_cast<ResolutionMethod>( method );
    switch ( methodValue )
    {
      case NoChange:
        error->setFixed( method );
        break;

      case AddMissingVertex:
      {
        QgsFeaturePool *featurePool = featurePools[error->layerId()];

        QgsFeature feature;
        featurePool->getFeature( error->featureId(), feature );

        QgsPointXY pointOnSegment; // Should be equal to location
        int vertexIndex;
        QgsGeometry geometry = feature.geometry();
        geometry.closestSegmentWithContext( error->location(), pointOnSegment, vertexIndex );
        geometry.insertVertex( QgsPoint( error->location() ), vertexIndex );
        feature.setGeometry( geometry );

        featurePool->updateFeature( feature );
        // TODO update "changes" structure

        error->setFixed( method );
      }
      break;
    }
  }
}

QStringList QgsGeometryMissingVertexCheck::resolutionMethods() const
{
  static QStringList methods = QStringList()
                               << tr( "No action" )
                               << tr( "Add missing vertex" );
  return methods;
}

QString QgsGeometryMissingVertexCheck::description() const
{
  return factoryDescription();
}

void QgsGeometryMissingVertexCheck::processPolygon( const QgsCurvePolygon *polygon, QgsFeaturePool *featurePool, QList<QgsGeometryCheckError *> &errors, const QgsGeometryCheckerUtils::LayerFeature &layerFeature, QgsFeedback *feedback ) const
{
  const QgsFeature &currentFeature = layerFeature.feature();
  auto boundaries = std::make_unique<QgsMultiPolygon>();

  std::unique_ptr<QgsGeometryEngine> geomEngine( QgsGeometry::createGeometryEngine( polygon->exteriorRing()->clone(), mContext->tolerance ) );
  boundaries->addGeometry( geomEngine->buffer( mContext->tolerance, 5 ) );

  const int numRings = polygon->numInteriorRings();
  for ( int i = 0; i < numRings; ++i )
  {
    geomEngine.reset( QgsGeometry::createGeometryEngine( polygon->interiorRing( i ), mContext->tolerance ) );
    boundaries->addGeometry( geomEngine->buffer( mContext->tolerance, 5 ) );
  }

  geomEngine.reset( QgsGeometry::createGeometryEngine( boundaries.get(), mContext->tolerance ) );
  geomEngine->prepareGeometry();

  const QgsFeatureIds fids = featurePool->getIntersects( boundaries->boundingBox() );

  QgsFeature compareFeature;
  for ( QgsFeatureId fid : fids )
  {
    if ( fid == currentFeature.id() )
      continue;

    if ( featurePool->getFeature( fid, compareFeature ) )
    {
      if ( feedback && feedback->isCanceled() )
        break;

      const QgsGeometry compareGeometry = compareFeature.geometry();
      QgsVertexIterator vertexIterator = compareGeometry.vertices();
      while ( vertexIterator.hasNext() )
      {
        const QgsPoint &pt = vertexIterator.next();
        if ( geomEngine->intersects( &pt ) )
        {
          QgsVertexId vertexId;
          QgsPoint closestVertex = QgsGeometryUtils::closestVertex( *polygon, pt, vertexId );

          if ( closestVertex.distance( pt ) > mContext->tolerance )
          {
            bool alreadyReported = false;
            for ( QgsGeometryCheckError *error : std::as_const( errors ) )
            {
              // Only list missing vertices once
              if ( error->featureId() == currentFeature.id() && error->location() == QgsPointXY( pt ) )
              {
                alreadyReported = true;
                break;
              }
            }
            if ( !alreadyReported )
            {
              auto error = std::make_unique<QgsGeometryMissingVertexCheckError>( this, layerFeature, QgsPointXY( pt ) );
              error->setAffectedAreaBBox( contextBoundingBox( polygon, vertexId, pt ) );
              QMap<QString, QgsFeatureIds> involvedFeatures;
              involvedFeatures[layerFeature.layerId()].insert( layerFeature.feature().id() );
              involvedFeatures[featurePool->layerId()].insert( fid );
              error->setInvolvedFeatures( involvedFeatures );

              errors.append( error.release() );
            }
          }
        }
      }
    }
  }
}

QgsRectangle QgsGeometryMissingVertexCheck::contextBoundingBox( const QgsCurvePolygon *polygon, const QgsVertexId &vertexId, const QgsPoint &point ) const
{
  QgsVertexId vertexBefore;
  QgsVertexId vertexAfter;

  polygon->adjacentVertices( vertexId, vertexBefore, vertexAfter );

  QgsPoint ptBefore = polygon->vertexAt( vertexBefore );
  QgsPoint ptAt = polygon->vertexAt( vertexId );
  QgsPoint ptAfter = polygon->vertexAt( vertexAfter );

  double length = std::abs( ptAt.distance( ptBefore ) ) + std::abs( ptAt.distance( ptAfter ) );

  QgsRectangle rect( point.x() - length / 2, point.y() - length / 2, point.x() + length / 2, point.y() + length / 2 );
  return rect;
}

QString QgsGeometryMissingVertexCheck::id() const
{
  return factoryId();
}

QList<Qgis::GeometryType> QgsGeometryMissingVertexCheck::compatibleGeometryTypes() const
{
  return factoryCompatibleGeometryTypes();
}

QgsGeometryCheck::Flags QgsGeometryMissingVertexCheck::flags() const
{
  return factoryFlags();
}

QgsGeometryCheck::CheckType QgsGeometryMissingVertexCheck::checkType() const
{
  return factoryCheckType();
}

///@cond private
QList<Qgis::GeometryType> QgsGeometryMissingVertexCheck::factoryCompatibleGeometryTypes()
{
  return { Qgis::GeometryType::Polygon };
}

bool QgsGeometryMissingVertexCheck::factoryIsCompatible( QgsVectorLayer *layer ) SIP_SKIP
{
  return factoryCompatibleGeometryTypes().contains( layer->geometryType() );
}

QString QgsGeometryMissingVertexCheck::factoryDescription()
{
  return tr( "Missing Vertex" );
}

QString QgsGeometryMissingVertexCheck::factoryId()
{
  return u"QgsGeometryMissingVertexCheck"_s;
}

QgsGeometryCheck::Flags QgsGeometryMissingVertexCheck::factoryFlags()
{
  return QgsGeometryCheck::AvailableInValidation;
}

QgsGeometryCheck::CheckType QgsGeometryMissingVertexCheck::factoryCheckType()
{
  return QgsGeometryCheck::LayerCheck;
}
///@endcond private

QgsGeometryMissingVertexCheckError::QgsGeometryMissingVertexCheckError( const QgsGeometryCheck *check, const QgsGeometryCheckerUtils::LayerFeature &layerFeature, const QgsPointXY &errorLocation, QgsVertexId vidx, const QVariant &value, QgsGeometryCheckError::ValueType valueType )
  : QgsGeometryCheckError( check, layerFeature, errorLocation, vidx, value, valueType )
{
}

QgsRectangle QgsGeometryMissingVertexCheckError::affectedAreaBBox() const
{
  return mAffectedAreaBBox;
}

void QgsGeometryMissingVertexCheckError::setAffectedAreaBBox( const QgsRectangle &affectedAreaBBox )
{
  mAffectedAreaBBox = affectedAreaBBox;
}

QMap<QString, QgsFeatureIds> QgsGeometryMissingVertexCheckError::involvedFeatures() const
{
  return mInvolvedFeatures;
}

void QgsGeometryMissingVertexCheckError::setInvolvedFeatures( const QMap<QString, QgsFeatureIds> &involvedFeatures )
{
  mInvolvedFeatures = involvedFeatures;
}

QIcon QgsGeometryMissingVertexCheckError::icon() const
{
  if ( status() == QgsGeometryCheckError::StatusFixed )
    return QgsApplication::getThemeIcon( u"/algorithms/mAlgorithmCheckGeometry.svg"_s );
  else
    return QgsApplication::getThemeIcon( u"/checks/MissingVertex.svg"_s );
}
