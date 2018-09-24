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

#include "qgsfeaturepool.h"
#include "qgsgeometrycollection.h"
#include "qgscurvepolygon.h"
#include "qgscurve.h"
#include "qgslinestring.h"
#include "qgsgeometryengine.h"
#include "qgsgeometryutils.h"

void QgsGeometryMissingVertexCheck::collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &messages, QAtomicInt *progressCounter, const QMap<QString, QgsFeatureIds> &ids ) const
{
  Q_UNUSED( messages );
  if ( progressCounter )
    progressCounter->fetchAndAddRelaxed( 1 );

  QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds() : ids;

  QgsFeaturePool *featurePool = mContext->featurePools.value( featureIds.firstKey() );

  const QgsGeometryCheckerUtils::LayerFeatures layerFeatures( mContext->featurePools, featureIds, mCompatibleGeometryTypes, nullptr, mContext, true );

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

void QgsGeometryMissingVertexCheck::fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes &changes ) const
{
  Q_UNUSED( mergeAttributeIndices );
  Q_UNUSED( changes );
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

void QgsGeometryMissingVertexCheck::processPolygon( const QgsCurvePolygon *polygon, QgsFeaturePool *featurePool, QList<QgsGeometryCheckError *> &errors, const QgsGeometryCheckerUtils::LayerFeature &layerFeature ) const
{
  std::unique_ptr<QgsGeometryCollection> boundaries = qgis::make_unique<QgsGeometryCollection>();

  std::unique_ptr< QgsGeometryEngine > geomEngine = QgsGeometryCheckerUtils::createGeomEngine( polygon->exteriorRing(), mContext->tolerance );
  boundaries->addGeometry( geomEngine->buffer( mContext->tolerance, 5 ) );

  const int numRings = polygon->numInteriorRings();
  for ( int i = 0; i < numRings; ++i )
  {
    geomEngine = QgsGeometryCheckerUtils::createGeomEngine( polygon->exteriorRing(), mContext->tolerance );
    boundaries->addGeometry( geomEngine->buffer( mContext->tolerance, 5 ) );
  }

  geomEngine = QgsGeometryCheckerUtils::createGeomEngine( boundaries.get(), mContext->tolerance );

  const QgsFeatureIds fids = featurePool->getIntersects( boundaries->boundingBox() );

  QgsFeature feature;
  for ( QgsFeatureId fid : fids )
  {
    if ( fid == layerFeature.feature().id() )
      continue;

    if ( featurePool->getFeature( fid, feature ) )
    {
      QgsVertexIterator vertexIterator = feature.geometry().vertices();
      while ( vertexIterator.hasNext() )
      {
        const QgsPoint &pt = vertexIterator.next();
        if ( geomEngine->intersects( &pt ) )
        {
          QgsVertexId vertexId;
          QgsPoint closestVertex = QgsGeometryUtils::closestVertex( *polygon, pt, vertexId );

          if ( closestVertex.distance( pt ) > mContext->tolerance )
          {
            errors.append( new QgsGeometryCheckError( this, layerFeature, QgsPointXY( pt ) ) );
          }
        }
      }
    }
  }
}
