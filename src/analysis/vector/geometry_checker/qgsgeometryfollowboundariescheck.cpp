/***************************************************************************
    qgsgeometryfollowboundariescheck.cpp
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

#include "qgsgeometryfollowboundariescheck.h"
#include "qgsgeometryengine.h"
#include "qgsproject.h"
#include "qgsspatialindex.h"
#include "qgsvectorlayer.h"

QgsGeometryFollowBoundariesCheck::QgsGeometryFollowBoundariesCheck( QgsGeometryCheckerContext *context, QgsVectorLayer *checkLayer )
  : QgsGeometryCheck( FeatureNodeCheck, {QgsWkbTypes::PolygonGeometry}, context )
{
  mCheckLayer = checkLayer;
  if ( mCheckLayer )
  {
    mIndex = new QgsSpatialIndex( *mCheckLayer->dataProvider() );
  }
}

QgsGeometryFollowBoundariesCheck::~QgsGeometryFollowBoundariesCheck()
{
  delete mIndex;
}

void QgsGeometryFollowBoundariesCheck::collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &/*messages*/, QAtomicInt *progressCounter, const QMap<QString, QgsFeatureIds> &ids ) const
{
  if ( !mIndex || !mCheckLayer )
  {
    return;
  }

  QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds() : ids;
  featureIds.remove( mCheckLayer->id() ); // Don't check layer against itself
  QgsGeometryCheckerUtils::LayerFeatures layerFeatures( mContext->featurePools, featureIds, mCompatibleGeometryTypes, progressCounter, mContext );
  for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeature : layerFeatures )
  {
    const QgsAbstractGeometry *geom = layerFeature.geometry();

    // The geometry to crs of the check layer
    QgsCoordinateTransform crst( layerFeature.layer()->crs(), mCheckLayer->crs(), QgsProject::instance() );
    QgsGeometry geomt( geom->clone() );
    geomt.transform( crst );

    std::unique_ptr< QgsGeometryEngine > geomEngine = QgsGeometryCheckerUtils::createGeomEngine( geomt.constGet(), mContext->tolerance );

    // Get potential reference features
    QgsRectangle searchBounds = geomt.constGet()->boundingBox();
    searchBounds.grow( mContext->tolerance );
    QgsFeatureIds refFeatureIds = mIndex->intersects( searchBounds ).toSet();

    QgsFeatureRequest refFeatureRequest = QgsFeatureRequest().setFilterFids( refFeatureIds ).setSubsetOfAttributes( QgsAttributeList() );
    QgsFeatureIterator refFeatureIt = mCheckLayer->getFeatures( refFeatureRequest );

    if ( refFeatureIds.isEmpty() )
    {
      // If no potential reference features are found, the geometry is definitely not following boundaries of reference layer features
      errors.append( new QgsGeometryCheckError( this, layerFeature, QgsPointXY( geom->centroid() ) ) );
    }
    else
    {
      // All reference features must be either contained or disjoint from tested geometry
      QgsFeature refFeature;
      while ( refFeatureIt.nextFeature( refFeature ) )
      {
        const QgsAbstractGeometry *refGeom = refFeature.geometry().constGet();
        std::unique_ptr<QgsGeometryEngine> refgeomEngine( QgsGeometryCheckerUtils::createGeomEngine( refGeom, mContext->tolerance ) );
        QgsGeometry reducedRefGeom( refgeomEngine->buffer( -mContext->tolerance, 0 ) );
        if ( !( geomEngine->contains( reducedRefGeom.constGet() ) || geomEngine->disjoint( reducedRefGeom.constGet() ) ) )
        {
          errors.append( new QgsGeometryCheckError( this, layerFeature, QgsPointXY( geom->centroid() ) ) );
          break;
        }
      }
    }
  }
}

void QgsGeometryFollowBoundariesCheck::fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> & /*mergeAttributeIndices*/, Changes & /*changes*/ ) const
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

QStringList QgsGeometryFollowBoundariesCheck::resolutionMethods() const
{
  static QStringList methods = QStringList() << tr( "No action" );
  return methods;
}
