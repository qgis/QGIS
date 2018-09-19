/***************************************************************************
    qgsgeometrypointcoveredbylinecheck.cpp
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

#include "qgsgeometrypointcoveredbylinecheck.h"
#include "qgslinestring.h"
#include "qgsfeaturepool.h"

void QgsGeometryPointCoveredByLineCheck::collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &messages, QgsFeedback *feedback, const LayerFeatureIds &ids ) const
{
  Q_UNUSED( messages )

  QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds() : ids.toMap();
  QgsGeometryCheckerUtils::LayerFeatures layerFeatures( mContext->featurePools, featureIds, mCompatibleGeometryTypes, feedback, mContext, true );
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
      // Check that point lies on a line
      bool touches = false;
      QgsRectangle rect( point->x() - mContext->tolerance, point->y() - mContext->tolerance,
                         point->x() + mContext->tolerance, point->y() + mContext->tolerance );
      QgsGeometryCheckerUtils::LayerFeatures checkFeatures( mContext->featurePools, featureIds.keys(), rect, {QgsWkbTypes::LineGeometry}, mContext );
      for ( const QgsGeometryCheckerUtils::LayerFeature &checkFeature : checkFeatures )
      {
        const QgsAbstractGeometry *testGeom = checkFeature.geometry().constGet();
        for ( int jPart = 0, mParts = testGeom->partCount(); jPart < mParts; ++jPart )
        {
          const QgsLineString *testLine = dynamic_cast<const QgsLineString *>( QgsGeometryCheckerUtils::getGeomPart( testGeom, jPart ) );
          if ( !testLine )
          {
            continue;
          }
          if ( QgsGeometryCheckerUtils::pointOnLine( *point, testLine, mContext->tolerance ) )
          {
            touches = true;
            break;
          }
        }
        if ( touches )
        {
          break;
        }
      }
      if ( touches )
      {
        continue;
      }
      errors.append( new QgsGeometryCheckError( this, layerFeature, *point, QgsVertexId( iPart, 0, 0 ) ) );
    }
  }
}

void QgsGeometryPointCoveredByLineCheck::fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> & /*mergeAttributeIndices*/, Changes & /*changes*/ ) const
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

QStringList QgsGeometryPointCoveredByLineCheck::resolutionMethods() const
{
  static QStringList methods = QStringList() << tr( "No action" );
  return methods;
}
