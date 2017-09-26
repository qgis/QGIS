/***************************************************************************
    qgsgeometrypointinpolygoncheck.cpp
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

#include "qgsgeometrypointinpolygoncheck.h"
#include "qgspolygon.h"
#include "qgsgeometryengine.h"

void QgsGeometryPointInPolygonCheck::collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &/*messages*/, QAtomicInt *progressCounter, const QMap<QString, QgsFeatureIds> &ids ) const
{
  QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds() : ids;
  QgsGeometryCheckerUtils::LayerFeatures layerFeatures( mContext->featurePools, featureIds, mCompatibleGeometryTypes, progressCounter, true );
  for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeature : layerFeatures )
  {
    const QgsAbstractGeometry *geom = layerFeature.geometry();
    for ( int iPart = 0, nParts = geom->partCount(); iPart < nParts; ++iPart )
    {
      const QgsPoint *point = dynamic_cast<const QgsPoint *>( QgsGeometryCheckerUtils::getGeomPart( geom, iPart ) );
      if ( !point )
      {
        // Should not happen
        continue;
      }

      // Check whether point is contained by a fully contained by a polygon
      bool contained = false;
      QgsRectangle rect( point->x() - mContext->tolerance, point->y() - mContext->tolerance,
                         point->x() + mContext->tolerance, point->y() + mContext->tolerance );
      QgsGeometryCheckerUtils::LayerFeatures checkFeatures( mContext->featurePools, featureIds.keys(), rect, {QgsWkbTypes::PolygonGeometry} );
      for ( const QgsGeometryCheckerUtils::LayerFeature &checkFeature : checkFeatures )
      {
        const QgsAbstractGeometry *testGeom = checkFeature.geometry();
        for ( int jPart = 0, mParts = testGeom->partCount(); jPart < mParts; ++jPart )
        {
          const QgsPolygonV2 *testPoly = dynamic_cast<const QgsPolygonV2 *>( QgsGeometryCheckerUtils::getGeomPart( testGeom, jPart ) );
          if ( !testPoly )
          {
            continue;
          }
          QSharedPointer<QgsGeometryEngine> testGeomEngine = QgsGeometryCheckerUtils::createGeomEngine( testPoly, mContext->tolerance );
          if ( testGeomEngine->contains( point ) )
          {
            // Check whether point does not lie on a ring boundary
            bool touchesBoundary = false;
            if ( dynamic_cast<const QgsLineString *>( testPoly->exteriorRing() ) &&
                 QgsGeometryCheckerUtils::pointOnLine( *point, static_cast<const QgsLineString *>( testPoly->exteriorRing() ), mContext->tolerance ) )
            {
              touchesBoundary = true;
            }
            else
            {
              for ( int jRing = 1, mRings = testPoly->ringCount( jPart ); jRing < mRings; ++jRing )
              {
                if ( dynamic_cast<const QgsLineString *>( testPoly->interiorRing( jRing - 1 ) ) &&
                     QgsGeometryCheckerUtils::pointOnLine( *point, static_cast<const QgsLineString *>( testPoly->interiorRing( jRing - 1 ) ), mContext->tolerance ) )
                {
                  touchesBoundary = true;
                  break;
                }
              }
            }
            if ( !touchesBoundary )
            {
              // Ok, point is contained by a polygon and does not touch its boundaries
              contained = true;
              break;
            }
          }
        }
        if ( contained )
        {
          break;
        }
      }
      if ( !contained )
      {
        errors.append( new QgsGeometryCheckError( this, layerFeature, *point, QgsVertexId( iPart, 0, 0 ) ) );
      }
    }
  }
}

void QgsGeometryPointInPolygonCheck::fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> & /*mergeAttributeIndices*/, Changes & /*changes*/ ) const
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

QStringList QgsGeometryPointInPolygonCheck::getResolutionMethods() const
{
  static QStringList methods = QStringList() << tr( "No action" );
  return methods;
}
