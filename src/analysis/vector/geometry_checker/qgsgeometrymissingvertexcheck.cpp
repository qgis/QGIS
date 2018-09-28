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

#include "qgsfeedback.h"
#include "qgsgeometrycollection.h"
#include "qgsmultipolygon.h"
#include "qgscurvepolygon.h"
#include "qgscurve.h"
#include "qgslinestring.h"
#include "qgsgeometryengine.h"
#include "qgsgeometryutils.h"

QgsGeometryMissingVertexCheck::QgsGeometryMissingVertexCheck( const QgsGeometryCheckContext *context, const QVariantMap &geometryCheckConfiguration )
  : QgsGeometryCheck( LayerCheck,  context, geometryCheckConfiguration )

{

}

void QgsGeometryMissingVertexCheck::collectErrors( const QMap<QString, QgsFeaturePool *> &featurePools, QList<QgsGeometryCheckError *> &errors, QStringList &messages, QgsFeedback *feedback, const LayerFeatureIds &ids ) const
{
  Q_UNUSED( messages )
  if ( feedback )
    feedback->setProgress( feedback->progress() + 1.0 );

  QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds( featurePools ) : ids.toMap();

  QgsFeaturePool *featurePool = featurePools.value( featureIds.firstKey() );

  const QgsGeometryCheckerUtils::LayerFeatures layerFeatures( featurePools, featureIds, compatibleGeometryTypes(), nullptr, mContext, true );

  for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeature : layerFeatures )
  {
    const QgsAbstractGeometry *geom = layerFeature.geometry().constGet();

    if ( QgsCurvePolygon *polygon = qgsgeometry_cast<QgsCurvePolygon *>( geom ) )
    {
      processPolygon( polygon, featurePool, errors, layerFeature );
    }
    else if ( QgsGeometryCollection *collection = qgsgeometry_cast<QgsGeometryCollection *>( geom ) )
    {
      const int numGeometries = collection->numGeometries();
      for ( int i = 0; i < numGeometries; ++i )
      {
        if ( QgsCurvePolygon *polygon = qgsgeometry_cast<QgsCurvePolygon *>( collection->geometryN( i ) ) )
        {
          processPolygon( polygon, featurePool, errors, layerFeature );
        }
      }
    }
  }
}

void QgsGeometryMissingVertexCheck::fixError( const QMap<QString, QgsFeaturePool *> &featurePools, QgsGeometryCheckError *error, int method, const QMap<QString, int> & /*mergeAttributeIndices*/, Changes &changes ) const
{
  Q_UNUSED( featurePools )
  Q_UNUSED( changes )
  if ( method == NoChange )
  {
    error->setFixed( method );
  }
}

QStringList QgsGeometryMissingVertexCheck::resolutionMethods() const
{
  static QStringList methods = QStringList() << tr( "No action" );
  return methods;
}

QString QgsGeometryMissingVertexCheck::description() const
{
  return factoryDescription();
}

void QgsGeometryMissingVertexCheck::processPolygon( const QgsCurvePolygon *polygon, QgsFeaturePool *featurePool, QList<QgsGeometryCheckError *> &errors, const QgsGeometryCheckerUtils::LayerFeature &layerFeature ) const
{
  const QgsFeature &currentFeature = layerFeature.feature();
  std::unique_ptr<QgsMultiPolygon> boundaries = qgis::make_unique<QgsMultiPolygon>();

  std::unique_ptr< QgsGeometryEngine > geomEngine = QgsGeometryCheckerUtils::createGeomEngine( polygon->exteriorRing(), mContext->tolerance );
  boundaries->addGeometry( geomEngine->buffer( mContext->tolerance, 5 ) );

  const int numRings = polygon->numInteriorRings();
  for ( int i = 0; i < numRings; ++i )
  {
    geomEngine = QgsGeometryCheckerUtils::createGeomEngine( polygon->exteriorRing(), mContext->tolerance );
    boundaries->addGeometry( geomEngine->buffer( mContext->tolerance, 5 ) );
  }

  geomEngine = QgsGeometryCheckerUtils::createGeomEngine( boundaries.get(), mContext->tolerance );
  geomEngine->prepareGeometry();

  const QgsFeatureIds fids = featurePool->getIntersects( boundaries->boundingBox() );

  QgsFeature compareFeature;
  for ( QgsFeatureId fid : fids )
  {
    if ( fid == currentFeature.id() )
      continue;

    if ( featurePool->getFeature( fid, compareFeature ) )
    {
      QgsVertexIterator vertexIterator = compareFeature.geometry().vertices();
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
            for ( QgsGeometryCheckError *error : qgis::as_const( errors ) )
            {
              // Only list missing vertices once
              if ( error->featureId() == currentFeature.id() && error->location() == QgsPointXY( pt ) )
              {
                alreadyReported = true;
                break;
              }
            }
            if ( !alreadyReported )
              errors.append( new QgsGeometryCheckError( this, layerFeature, QgsPointXY( pt ) ) );
          }
        }
      }
    }
  }
}

QString QgsGeometryMissingVertexCheck::id() const
{
  return factoryId();
}

QList<QgsWkbTypes::GeometryType> QgsGeometryMissingVertexCheck::compatibleGeometryTypes() const
{
  return factoryCompatibleGeometryTypes();
}

QgsGeometryCheck::Flags QgsGeometryMissingVertexCheck::flags() const
{
  return factoryFlags();
}

QList<QgsWkbTypes::GeometryType> QgsGeometryMissingVertexCheck::factoryCompatibleGeometryTypes()
{
  return {QgsWkbTypes::PolygonGeometry};
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
  return QStringLiteral( "QgsGeometryMissingVertexCheck" );
}

QgsGeometryCheck::Flags QgsGeometryMissingVertexCheck::factoryFlags()
{
  return QgsGeometryCheck::SingleLayerTopologyCheck;
}
