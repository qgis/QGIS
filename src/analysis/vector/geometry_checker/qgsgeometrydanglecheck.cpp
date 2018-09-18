/***************************************************************************
    qgsgeometrydanglecheck.cpp
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

#include "qgsgeometrydanglecheck.h"
#include "qgslinestring.h"
#include "qgsvectorlayer.h"

void QgsGeometryDangleCheck::collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &/*messages*/, QgsFeedback *feedback, const QMap<QString, QgsFeatureIds> &ids ) const
{
  QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds() : ids;
  QgsGeometryCheckerUtils::LayerFeatures layerFeatures( mContext->featurePools, featureIds, mCompatibleGeometryTypes, feedback, mContext );
  for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeature : layerFeatures )
  {
    const QgsAbstractGeometry *geom = layerFeature.geometry().constGet();
    for ( int iPart = 0, nParts = geom->partCount(); iPart < nParts; ++iPart )
    {
      const QgsLineString *line = dynamic_cast<const QgsLineString *>( QgsGeometryCheckerUtils::getGeomPart( geom, iPart ) );
      if ( !line )
      {
        // Should not happen
        continue;
      }
      // Check that start and end node lie on a line
      int nVerts = geom->vertexCount( iPart, 0 );
      const QgsPoint &p1 = geom->vertexAt( QgsVertexId( iPart, 0, 0 ) );
      const QgsPoint &p2 = geom->vertexAt( QgsVertexId( iPart, 0, nVerts - 1 ) );

      bool p1touches = QgsGeometryCheckerUtils::pointOnLine( p1, line, mContext->tolerance, true );
      bool p2touches = QgsGeometryCheckerUtils::pointOnLine( p2, line, mContext->tolerance, true );

      if ( p1touches && p2touches )
      {
        // Both endpoints lie on line itself
        continue;
      }

      // Check whether endpoints line on another line in the layer
      QgsGeometryCheckerUtils::LayerFeatures checkFeatures( mContext->featurePools, QList<QString>() << layerFeature.layer()->id(), line->boundingBox(), {QgsWkbTypes::LineGeometry}, mContext );
      for ( const QgsGeometryCheckerUtils::LayerFeature &checkFeature : checkFeatures )
      {
        const QgsAbstractGeometry *testGeom = checkFeature.geometry().constGet();
        for ( int jPart = 0, mParts = testGeom->partCount(); jPart < mParts; ++jPart )
        {
          if ( checkFeature.feature().id() == layerFeature.feature().id() && iPart == jPart )
          {
            // Skip current feature part, it was already checked above
            continue;
          }
          const QgsLineString *testLine = dynamic_cast<const QgsLineString *>( QgsGeometryCheckerUtils::getGeomPart( testGeom, jPart ) );
          if ( !testLine )
          {
            continue;
          }
          p1touches = p1touches || QgsGeometryCheckerUtils::pointOnLine( p1, testLine, mContext->tolerance );
          p2touches = p2touches || QgsGeometryCheckerUtils::pointOnLine( p2, testLine, mContext->tolerance );
          if ( p1touches && p2touches )
          {
            break;
          }
        }
        if ( p1touches && p2touches )
        {
          break;
        }
      }
      if ( p1touches && p2touches )
      {
        continue;
      }
      if ( !p1touches )
      {
        errors.append( new QgsGeometryCheckError( this, layerFeature, p1, QgsVertexId( iPart, 0, 0 ) ) );
      }
      if ( !p2touches )
      {
        errors.append( new QgsGeometryCheckError( this, layerFeature, p2, QgsVertexId( iPart, 0, nVerts - 1 ) ) );
      }
    }
  }
}

void QgsGeometryDangleCheck::fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> & /*mergeAttributeIndices*/, Changes & /*changes*/ ) const
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

QStringList QgsGeometryDangleCheck::resolutionMethods() const
{
  static QStringList methods = QStringList() << tr( "No action" );
  return methods;
}
