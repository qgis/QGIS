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

#include "qgsgeometrycheckcontext.h"
#include "qgsgeometrypointinpolygoncheck.h"
#include "qgspolygon.h"
#include "qgsgeometryengine.h"
#include "qgsgeometrycheckerror.h"

void QgsGeometryPointInPolygonCheck::collectErrors( const QMap<QString, QgsFeaturePool *> &featurePools, QList<QgsGeometryCheckError *> &errors, QStringList &messages, QgsFeedback *feedback, const LayerFeatureIds &ids ) const
{
  const QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds( featurePools ) : ids.toMap();
  const QgsGeometryCheckerUtils::LayerFeatures layerFeatures( featurePools, featureIds, compatibleGeometryTypes(), feedback, mContext, true );
  for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeature : layerFeatures )
  {
    const QgsAbstractGeometry *geom = layerFeature.geometry().constGet();
    for ( int iPart = 0, nParts = geom->partCount(); iPart < nParts; ++iPart )
    {
      const QgsPoint *point = dynamic_cast<const QgsPoint *>( QgsGeometryCheckerUtils::getGeomPart( geom, iPart ) );
      if ( !point )
      {
        // Should not happen
        continue;
      }
      int nTested = 0;
      int nInside = 0;

      // Check whether point is contained by a fully contained by a polygon
      const QgsRectangle rect( point->x() - mContext->tolerance, point->y() - mContext->tolerance,
                               point->x() + mContext->tolerance, point->y() + mContext->tolerance );
      const QgsGeometryCheckerUtils::LayerFeatures checkFeatures( featurePools, featureIds.keys(), rect, {QgsWkbTypes::PolygonGeometry}, mContext );
      for ( const QgsGeometryCheckerUtils::LayerFeature &checkFeature : checkFeatures )
      {
        ++nTested;
        const QgsAbstractGeometry *testGeom = checkFeature.geometry().constGet();
        std::unique_ptr< QgsGeometryEngine > testGeomEngine = QgsGeometryCheckerUtils::createGeomEngine( testGeom, mContext->reducedTolerance );
        if ( !testGeomEngine->isValid() )
        {
          messages.append( tr( "Point in polygon check failed for (%1): the geometry is invalid" ).arg( checkFeature.id() ) );
          continue;
        }
        if ( testGeomEngine->contains( point ) && !testGeomEngine->touches( point ) )
        {
          ++nInside;
        }
      }
      if ( nTested == 0 || nTested != nInside )
      {
        errors.append( new QgsGeometryCheckError( this, layerFeature, *point, QgsVertexId( iPart, 0, 0 ) ) );
      }
    }
  }
}

void QgsGeometryPointInPolygonCheck::fixError( const QMap<QString, QgsFeaturePool *> &featurePools, QgsGeometryCheckError *error, int method, const QMap<QString, int> & /*mergeAttributeIndices*/, Changes & /*changes*/ ) const
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

QStringList QgsGeometryPointInPolygonCheck::resolutionMethods() const
{
  static const QStringList methods = QStringList() << tr( "No action" );
  return methods;
}

QgsGeometryCheck::CheckType QgsGeometryPointInPolygonCheck::factoryCheckType()
{
  return QgsGeometryCheck::FeatureNodeCheck;
}
