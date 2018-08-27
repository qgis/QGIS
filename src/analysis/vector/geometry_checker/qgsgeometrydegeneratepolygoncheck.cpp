/***************************************************************************
    qgsgeometrydegeneratepolygoncheck.cpp
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

#include "qgsgeometrydegeneratepolygoncheck.h"
#include "qgsfeaturepool.h"

void QgsGeometryDegeneratePolygonCheck::collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &/*messages*/, QAtomicInt *progressCounter, const QMap<QString, QgsFeatureIds> &ids ) const
{
  QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds() : ids;
  QgsGeometryCheckerUtils::LayerFeatures layerFeatures( mContext->featurePools, featureIds, mCompatibleGeometryTypes, progressCounter );
  for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeature : layerFeatures )
  {
    const QgsAbstractGeometry *geom = layerFeature.geometry();
    for ( int iPart = 0, nParts = geom->partCount(); iPart < nParts; ++iPart )
    {
      for ( int iRing = 0, nRings = geom->ringCount( iPart ); iRing < nRings; ++iRing )
      {
        if ( QgsGeometryCheckerUtils::polyLineSize( geom, iPart, iRing ) < 3 )
        {
          QgsVertexId vidx( iPart, iRing );
          errors.append( new QgsGeometryCheckError( this, layerFeature, geom->vertexAt( vidx ), vidx ) );
        }
      }
    }
  }
}

void QgsGeometryDegeneratePolygonCheck::fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> & /*mergeAttributeIndices*/, Changes &changes ) const
{
  QgsFeaturePool *featurePool = mContext->featurePools[ error->layerId() ];
  QgsFeature feature;
  if ( !featurePool->get( error->featureId(), feature ) )
  {
    error->setObsolete();
    return;
  }
  QgsGeometry featureGeometry = feature.geometry();
  const QgsAbstractGeometry *geom = featureGeometry.constGet();
  QgsVertexId vidx = error->vidx();

  // Check if ring still exists
  if ( !vidx.isValid( geom ) )
  {
    error->setObsolete();
    return;
  }

  // Check if error still applies
  if ( QgsGeometryCheckerUtils::polyLineSize( geom, vidx.part, vidx.ring ) >= 3 )
  {
    error->setObsolete();
    return;
  }

  // Fix error
  if ( method == NoChange )
  {
    error->setFixed( method );
  }
  else if ( method == DeleteRing )
  {
    deleteFeatureGeometryRing( error->layerId(), feature, vidx.part, vidx.ring, changes );
    error->setFixed( method );
  }
  else
  {
    error->setFixFailed( tr( "Unknown method" ) );
  }
}

QStringList QgsGeometryDegeneratePolygonCheck::resolutionMethods() const
{
  static QStringList methods = QStringList() << tr( "Delete feature" ) << tr( "No action" );
  return methods;
}
