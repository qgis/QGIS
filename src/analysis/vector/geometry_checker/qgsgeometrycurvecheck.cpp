/***************************************************************************
    qgsgeometrycurvecheck.cpp
    ---------------------
    begin                : December 2023
    copyright            : (C) 2023 by Loïc Bartoletti
    email                : loic dot bartoletti at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsabstractgeometry.h"
#include "qgsgeometrycurvecheck.h"
#include "qgsfeaturepool.h"
#include "qgsgeometrycheckerror.h"

void QgsGeometryCurveCheck::collectErrors( const QMap<QString, QgsFeaturePool *> &featurePools, QList<QgsGeometryCheckError *> &errors, QStringList &messages, QgsFeedback *feedback, const LayerFeatureIds &ids ) const
{
  Q_UNUSED( messages )
  const QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds( featurePools ) : ids.toMap();
  const QgsGeometryCheckerUtils::LayerFeatures layerFeatures( featurePools, featureIds, compatibleGeometryTypes(), feedback, mContext );
  for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeature : layerFeatures )
  {
    const QgsAbstractGeometry *geom = layerFeature.geometry().constGet();
    for ( int iPart = 0, nParts = geom->partCount(); iPart < nParts; ++iPart )
    {
      const QgsAbstractGeometry *part = QgsGeometryCheckerUtils::getGeomPart( geom, iPart );
      if ( part->hasCurvedSegments() )
      {
        errors.append( new QgsGeometryCheckError( this, layerFeature, part->centroid(), QgsVertexId( iPart ) ) );
      }
    }
  }
}

void QgsGeometryCurveCheck::fixError( const QMap<QString, QgsFeaturePool *> &featurePools, QgsGeometryCheckError *error, int method, const QMap<QString, int> & /*mergeAttributeIndices*/, Changes &changes ) const
{
  QgsFeaturePool *featurePool = featurePools[ error->layerId() ];
  QgsFeature feature;
  if ( !featurePool->getFeature( error->featureId(), feature ) )
  {
    error->setObsolete();
    return;
  }
  const QgsGeometry featureGeom = feature.geometry();
  const QgsAbstractGeometry *geom = featureGeom.constGet();
  const QgsVertexId vidx = error->vidx();

  // Check if ring still exists
  if ( !vidx.isValid( geom ) )
  {
    error->setObsolete();
    return;
  }

  // Fix error
  if ( method == NoChange )
  {
    error->setFixed( method );
  }
  else if ( method == Delete )
  {
    deleteFeatureGeometryPart( featurePools, error->layerId(), feature, vidx.part, changes );
    error->setFixed( method );
  }
  else if ( method == StraightCurves )
  {
    replaceFeatureGeometryPart( featurePools, error->layerId(), feature, vidx.part, geom->segmentize(), changes );
    error->setFixed( method );
  }
  else
  {
    error->setFixFailed( tr( "Unknown method" ) );
  }
}

QStringList QgsGeometryCurveCheck::resolutionMethods() const
{
  static const QStringList methods = QStringList() << tr( "Convert to straight line segments" ) << tr( "Delete feature" ) << tr( "No action" );
  return methods;
}
