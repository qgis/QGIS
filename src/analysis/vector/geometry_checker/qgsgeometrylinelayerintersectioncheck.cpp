/***************************************************************************
    qgsgeometrylinelayerintersectioncheck.cpp
    ---------------------
    begin                : September 2017
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

#include "qgsgeometrycheckcontext.h"
#include "qgsgeometrylinelayerintersectioncheck.h"
#include "qgspolygon.h"
#include "qgslinestring.h"
#include "qgsfeaturepool.h"
#include "qgsgeometrycheckerror.h"

void QgsGeometryLineLayerIntersectionCheck::collectErrors( const QMap<QString, QgsFeaturePool *> &featurePools, QList<QgsGeometryCheckError *> &errors, QStringList &messages, QgsFeedback *feedback, const LayerFeatureIds &ids ) const
{
  Q_UNUSED( messages )

  QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds( featurePools ) : ids.toMap();
  featureIds.remove( mCheckLayer ); // Don't check layer against itself
  const QgsGeometryCheckerUtils::LayerFeatures layerFeatures( featurePools, featureIds, compatibleGeometryTypes(), feedback, mContext, true );
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

      // Check whether the line intersects with any other features of the specified layer
      const QgsGeometryCheckerUtils::LayerFeatures checkFeatures( featurePools, QStringList() << mCheckLayer, line->boundingBox(), {QgsWkbTypes::LineGeometry, QgsWkbTypes::PolygonGeometry}, mContext );
      for ( const QgsGeometryCheckerUtils::LayerFeature &checkFeature : checkFeatures )
      {
        const QgsAbstractGeometry *testGeom = checkFeature.geometry().constGet();
        for ( int jPart = 0, mParts = testGeom->partCount(); jPart < mParts; ++jPart )
        {
          const QgsAbstractGeometry *part = QgsGeometryCheckerUtils::getGeomPart( testGeom, jPart );
          if ( const QgsLineString *testLine = dynamic_cast<const QgsLineString *>( part ) )
          {
            const QList< QgsPoint > intersections = QgsGeometryCheckerUtils::lineIntersections( line, testLine, mContext->tolerance );
            for ( const QgsPoint &inter : intersections )
            {
              errors.append( new QgsGeometryCheckError( this, layerFeature, inter, QgsVertexId( iPart ), checkFeature.id() ) );
            }
          }
          else if ( const QgsPolygon *polygon = dynamic_cast<const QgsPolygon *>( part ) )
          {
            const QList< const QgsLineString * > rings = QgsGeometryCheckerUtils::polygonRings( polygon );
            for ( const QgsLineString *ring : rings )
            {
              const QList< QgsPoint > intersections = QgsGeometryCheckerUtils::lineIntersections( line, ring, mContext->tolerance );
              for ( const QgsPoint &inter : intersections )
              {
                errors.append( new QgsGeometryCheckError( this, layerFeature, inter, QgsVertexId( iPart ), checkFeature.id() ) );
              }
            }
          }
        }
      }
    }
  }
}

void QgsGeometryLineLayerIntersectionCheck::fixError( const QMap<QString, QgsFeaturePool *> &featurePools, QgsGeometryCheckError *error, int method, const QMap<QString, int> & /*mergeAttributeIndices*/, Changes & /*changes*/ ) const
{
  Q_UNUSED( featurePools )

  if ( method == NoChange )
  {
    error->setFixed( method );
  }
  else
  {
    error->setFixFailed( tr( "Unknown method" ) );
  }
}

QStringList QgsGeometryLineLayerIntersectionCheck::resolutionMethods() const
{
  static const QStringList methods = QStringList() << tr( "No action" );
  return methods;
}

QgsGeometryCheck::CheckType QgsGeometryLineLayerIntersectionCheck::factoryCheckType()
{
  return QgsGeometryCheck::FeatureNodeCheck;
}
