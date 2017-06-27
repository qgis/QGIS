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

void QgsGeometryLineIntersectionCheck::collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &/*messages*/, QAtomicInt *progressCounter, const QMap<QString, QgsFeatureIds> &ids ) const
{
  QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds() : ids;
  QgsGeometryCheckerUtils::LayerFeatures layerFeatures( mContext->featurePools, featureIds, mCompatibleGeometryTypes, progressCounter, true );
  for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeature : layerFeatures )
  {
    const QgsAbstractGeometry *geom = layerFeature.geometry();
    for ( int iPart = 0, nParts = geom->partCount(); iPart < nParts; ++iPart )
    {
      const QgsLineString *line = dynamic_cast<const QgsLineString *>( QgsGeometryCheckerUtils::getGeomPart( geom, iPart ) );
      if ( !line )
      {
        // Should not happen
        continue;
      }

      // Check whether the line intersects with any other lines
      QgsGeometryCheckerUtils::LayerFeatures checkFeatures( mContext->featurePools, featureIds.keys(), line->boundingBox(), {QgsWkbTypes::LineGeometry} );
      for ( const QgsGeometryCheckerUtils::LayerFeature &checkFeature : checkFeatures )
      {
        if ( checkFeature.feature().id() == layerFeature.feature().id() )
        {
          // Skip current feature
          continue;
        }
        const QgsAbstractGeometry *testGeom = checkFeature.geometry();
        for ( int jPart = 0, mParts = testGeom->partCount(); jPart < mParts; ++jPart )
        {
          const QgsLineString *testLine = dynamic_cast<const QgsLineString *>( QgsGeometryCheckerUtils::getGeomPart( testGeom, jPart ) );
          if ( !testLine )
          {
            continue;
          }
          QgsPoint inter;
          if ( QgsGeometryCheckerUtils::linesIntersect( line, testLine, mContext->tolerance, inter ) )
          {
            errors.append( new QgsGeometryCheckError( this, layerFeature, inter ) );
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

QStringList QgsGeometryLineIntersectionCheck::getResolutionMethods() const
{
  static QStringList methods = QStringList() << tr( "No action" );
  return methods;
}
