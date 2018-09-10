/***************************************************************************
    qgsgeometrylineintersectioncheck.cpp
    ---------------------
    begin                : June 2017
    copyright            : (C) 2017 by Sandro Mani / Sourcepole AG
    email                : smani at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometrylineintersectioncheck.h"
#include "qgslinestring.h"
#include "qgsvectorlayer.h"

void QgsGeometryLineIntersectionCheck::collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &/*messages*/, QAtomicInt *progressCounter, const QMap<QString, QgsFeatureIds> &ids ) const
{
  QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds() : ids;
  QgsGeometryCheckerUtils::LayerFeatures layerFeaturesA( mContext->featurePools, featureIds, mCompatibleGeometryTypes, progressCounter, true );
  QList<QString> layerIds = featureIds.keys();
  for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeatureA : layerFeaturesA )
  {
    // Ensure each pair of layers only gets compared once: remove the current layer from the layerIds, but add it to the layerList for layerFeaturesB
    layerIds.removeOne( layerFeatureA.layer()->id() );

    const QgsAbstractGeometry *geom = layerFeatureA.geometry();
    for ( int iPart = 0, nParts = geom->partCount(); iPart < nParts; ++iPart )
    {
      const QgsLineString *line = dynamic_cast<const QgsLineString *>( QgsGeometryCheckerUtils::getGeomPart( geom, iPart ) );
      if ( !line )
      {
        // Should not happen
        continue;
      }

      // Check whether the line intersects with any other lines
      QgsGeometryCheckerUtils::LayerFeatures layerFeaturesB( mContext->featurePools, QList<QString>() << layerFeatureA.layer()->id() << layerIds, line->boundingBox(), {QgsWkbTypes::LineGeometry} );
      for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeatureB : layerFeaturesB )
      {
        // > : only report intersections within same layer once
        if ( layerFeatureA.layer()->id() == layerFeatureB.layer()->id() && layerFeatureB.feature().id() > layerFeatureA.feature().id() )
        {
          continue;
        }

        const QgsAbstractGeometry *testGeom = layerFeatureB.geometry();
        for ( int jPart = 0, mParts = testGeom->partCount(); jPart < mParts; ++jPart )
        {
          // Skip current feature part, only report intersections within same part once
          if ( layerFeatureB.feature().id() == layerFeatureA.feature().id() && iPart >= jPart )
          {
            continue;
          }
          const QgsLineString *testLine = dynamic_cast<const QgsLineString *>( QgsGeometryCheckerUtils::getGeomPart( testGeom, jPart ) );
          if ( !testLine )
          {
            continue;
          }
          const QList< QgsPoint > intersections = QgsGeometryCheckerUtils::lineIntersections( line, testLine, mContext->tolerance );
          for ( const QgsPoint &inter : intersections )
          {
            errors.append( new QgsGeometryCheckError( this, layerFeatureA, inter, QgsVertexId( iPart ), layerFeatureB.id() ) );
          }
        }
      }
    }
  }
}

void QgsGeometryLineIntersectionCheck::fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> & /*mergeAttributeIndices*/, Changes & /*changes*/ ) const
{
  if ( method == NoChange )
  {
    error->setFixed( method );
  }
  else
  {
    error->setFixFailed( tr( "Unknown method" ) );
  }
}

QStringList QgsGeometryLineIntersectionCheck::resolutionMethods() const
{
  static QStringList methods = QStringList() << tr( "No action" );
  return methods;
}
