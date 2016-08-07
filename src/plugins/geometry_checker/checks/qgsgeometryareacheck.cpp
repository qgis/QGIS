/***************************************************************************
    qgsgeometryareacheck.cpp
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

#include "qgsgeometryengine.h"
#include "qgsgeometrycollection.h"
#include "qgsgeometryareacheck.h"
#include "../utils/qgsfeaturepool.h"

void QgsGeometryAreaCheck::collectErrors( QList<QgsGeometryCheckError*>& errors, QStringList &/*messages*/, QAtomicInt* progressCounter , const QgsFeatureIds &ids ) const
{
  const QgsFeatureIds& featureIds = ids.isEmpty() ? mFeaturePool->getFeatureIds() : ids;
  Q_FOREACH ( QgsFeatureId featureid, featureIds )
  {
    if ( progressCounter ) progressCounter->fetchAndAddRelaxed( 1 );
    QgsFeature feature;
    if ( !mFeaturePool->get( featureid, feature ) )
    {
      continue;
    }

    QgsGeometry g = feature.geometry();
    QgsAbstractGeometry* geom = g.geometry();
    if ( dynamic_cast<QgsGeometryCollection*>( geom ) )
    {
      QgsGeometryCollection* multiGeom = static_cast<QgsGeometryCollection*>( geom );
      for ( int i = 0, n = multiGeom->numGeometries(); i < n; ++i )
      {
        double value;
        if ( checkThreshold( multiGeom->geometryN( i ), value ) )
        {
          errors.append( new QgsGeometryCheckError( this, featureid, multiGeom->geometryN( i )->centroid(), QgsVertexId( i ), value, QgsGeometryCheckError::ValueArea ) );
        }
      }
    }
    else
    {
      double value;
      if ( checkThreshold( geom, value ) )
      {
        errors.append( new QgsGeometryCheckError( this, featureid, geom->centroid(), QgsVertexId( 0 ), value, QgsGeometryCheckError::ValueArea ) );
      }
    }
  }
}

void QgsGeometryAreaCheck::fixError( QgsGeometryCheckError* error, int method, int mergeAttributeIndex, Changes &changes ) const
{
  QgsFeature feature;
  if ( !mFeaturePool->get( error->featureId(), feature ) )
  {
    error->setObsolete();
    return;
  }
  QgsGeometry g = feature.geometry();
  QgsAbstractGeometry* geom = g.geometry();
  QgsVertexId vidx = error->vidx();

  // Check if polygon still exists
  if ( !vidx.isValid( geom ) )
  {
    error->setObsolete();
    return;
  }

  // Check if error still applies
  if ( dynamic_cast<QgsGeometryCollection*>( geom ) )
  {
    double value;
    if ( !checkThreshold( static_cast<QgsGeometryCollection*>( geom )->geometryN( vidx.part ), value ) )
    {
      error->setObsolete();
      return;
    }
  }
  else
  {
    double value;
    if ( !checkThreshold( geom, value ) )
    {
      error->setObsolete();
      return;
    }
  }

  // Fix with selected method
  if ( method == NoChange )
  {
    error->setFixed( method );
  }
  else if ( method == Delete )
  {
    deleteFeatureGeometryPart( feature, vidx.part, changes );
    error->setFixed( method );
  }
  else if ( method == MergeLongestEdge || method == MergeLargestArea || method == MergeIdenticalAttribute )
  {
    QString errMsg;
    if ( mergeWithNeighbor( feature,  vidx.part, method, mergeAttributeIndex, changes, errMsg ) )
    {
      error->setFixed( method );
    }
    else
    {
      error->setFixFailed( tr( "Failed to merge with neighbor: %1" ).arg( errMsg ) );
    }
  }
  else
  {
    error->setFixFailed( tr( "Unknown method" ) );
  }
}

bool QgsGeometryAreaCheck::mergeWithNeighbor( QgsFeature& feature, int partIdx, int method, int mergeAttributeIndex, Changes &changes, QString& errMsg ) const
{
  double maxVal = 0.;
  QgsFeature mergeFeature;
  int mergePartIdx = -1;
  bool matchFound = false;
  QgsGeometry g = feature.geometry();;
  QgsAbstractGeometry* geom = g.geometry();

  // Search for touching neighboring geometries
  Q_FOREACH ( QgsFeatureId testId, mFeaturePool->getIntersects( g.boundingBox() ) )
  {
    QgsFeature testFeature;
    if ( !mFeaturePool->get( testId, testFeature ) )
    {
      continue;
    }
    QgsGeometry testFeatureGeom = testFeature.geometry();
    QgsAbstractGeometry* testGeom = testFeatureGeom.geometry();
    for ( int testPartIdx = 0, nTestParts = testGeom->partCount(); testPartIdx < nTestParts; ++testPartIdx )
    {
      if ( testId == feature.id() && testPartIdx == partIdx )
      {
        continue;
      }
      double len = QgsGeometryCheckerUtils::sharedEdgeLength( QgsGeometryCheckerUtils::getGeomPart( geom, partIdx ), QgsGeometryCheckerUtils::getGeomPart( testGeom, testPartIdx ), QgsGeometryCheckPrecision::reducedTolerance() );
      if ( len > 0. )
      {
        if ( method == MergeLongestEdge || method == MergeLargestArea )
        {
          double val;
          if ( method == MergeLongestEdge )
          {
            val = len;
          }
          else
          {
            if ( dynamic_cast<QgsGeometryCollection*>( testGeom ) )
              val = static_cast<QgsGeometryCollection*>( testGeom )->geometryN( testPartIdx )->area();
            else
              val = testGeom->area();
          }
          if ( val > maxVal )
          {
            maxVal = val;
            mergeFeature = testFeature;
            mergePartIdx = testPartIdx;
          }
        }
        else if ( method == MergeIdenticalAttribute )
        {
          if ( testFeature.attribute( mergeAttributeIndex ) == feature.attribute( mergeAttributeIndex ) )
          {
            mergeFeature = testFeature;
            mergePartIdx = testPartIdx;
            matchFound = true;
            break;
          }
        }
      }
    }
    if ( matchFound )
    {
      break;
    }
  }

  if ( !matchFound && maxVal == 0. )
  {
    return method == MergeIdenticalAttribute ? true : false;
  }

  // Merge geometries
  QgsGeometry mergeFeatureGeom = mergeFeature.geometry();
  QgsAbstractGeometry* mergeGeom = mergeFeatureGeom.geometry();
  QgsGeometryEngine* geomEngine = QgsGeometryCheckerUtils::createGeomEngine( QgsGeometryCheckerUtils::getGeomPart( mergeGeom, mergePartIdx ), QgsGeometryCheckPrecision::tolerance() );
  QgsAbstractGeometry* combinedGeom = geomEngine->combine( *QgsGeometryCheckerUtils::getGeomPart( geom, partIdx ), &errMsg );
  delete geomEngine;
  if ( !combinedGeom || combinedGeom->isEmpty() )
  {
    return false;
  }

  // Replace polygon in merge geometry
  if ( mergeFeature.id() == feature.id() && mergePartIdx > partIdx )
  {
    --mergePartIdx;
  }
  replaceFeatureGeometryPart( mergeFeature, mergePartIdx, combinedGeom, changes );
  // Remove polygon from source geometry
  deleteFeatureGeometryPart( feature, partIdx, changes );

  return true;
}

const QStringList& QgsGeometryAreaCheck::getResolutionMethods() const
{
  static QStringList methods = QStringList()
                               << tr( "Merge with neighboring polygon with longest shared edge" )
                               << tr( "Merge with neighboring polygon with largest area" )
                               << tr( "Merge with neighboring polygon with identical attribute value, if any, or leave as is" )
                               << tr( "Delete feature" )
                               << tr( "No action" );
  return methods;
}
