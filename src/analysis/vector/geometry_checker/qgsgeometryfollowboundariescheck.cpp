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

#include "qgsgeometrycheckcontext.h"
#include "qgsgeometryfollowboundariescheck.h"
#include "qgsgeometryengine.h"
#include "qgsproject.h"
#include "qgsspatialindex.h"
#include "qgsvectorlayer.h"
#include "qgsgeometrycheckerror.h"

QgsGeometryFollowBoundariesCheck::QgsGeometryFollowBoundariesCheck( QgsGeometryCheckContext *context, const QVariantMap &configuration, QgsVectorLayer *checkLayer )
  : QgsGeometryCheck( context, configuration )
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

void QgsGeometryFollowBoundariesCheck::collectErrors( const QMap<QString, QgsFeaturePool *> &featurePools, QList<QgsGeometryCheckError *> &errors, QStringList &messages, QgsFeedback *feedback, const LayerFeatureIds &ids ) const
{
  Q_UNUSED( messages )

  if ( !mIndex || !mCheckLayer )
  {
    return;
  }

  QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds( featurePools ) : ids.toMap();
  featureIds.remove( mCheckLayer->id() ); // Don't check layer against itself
  const QgsGeometryCheckerUtils::LayerFeatures layerFeatures( featurePools, featureIds, compatibleGeometryTypes(), feedback, mContext );
  for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeature : layerFeatures )
  {
    const QgsAbstractGeometry *geom = layerFeature.geometry().constGet();

    // The geometry to crs of the check layer
    const QgsCoordinateTransform crst( layerFeature.layer()->crs(), mCheckLayer->crs(), QgsProject::instance() );
    QgsGeometry geomt( geom->clone() );
    geomt.transform( crst );

    std::unique_ptr< QgsGeometryEngine > geomEngine = QgsGeometryCheckerUtils::createGeomEngine( geomt.constGet(), mContext->tolerance );

    // Get potential reference features
    QgsRectangle searchBounds = geomt.constGet()->boundingBox();
    searchBounds.grow( mContext->tolerance );
    const QgsFeatureIds refFeatureIds = qgis::listToSet( mIndex->intersects( searchBounds ) );

    const QgsFeatureRequest refFeatureRequest = QgsFeatureRequest().setFilterFids( refFeatureIds ).setNoAttributes();
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
        const QgsGeometry reducedRefGeom( refgeomEngine->buffer( -mContext->tolerance, 0 ) );
        if ( !( geomEngine->contains( reducedRefGeom.constGet() ) || geomEngine->disjoint( reducedRefGeom.constGet() ) ) )
        {
          errors.append( new QgsGeometryCheckError( this, layerFeature, QgsPointXY( geom->centroid() ) ) );
          break;
        }
      }
    }
  }
}

void QgsGeometryFollowBoundariesCheck::fixError( const QMap<QString, QgsFeaturePool *> &featurePools, QgsGeometryCheckError *error, int method, const QMap<QString, int> & /*mergeAttributeIndices*/, Changes & /*changes*/ ) const
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

QStringList QgsGeometryFollowBoundariesCheck::resolutionMethods() const
{
  static const QStringList methods = QStringList() << tr( "No action" );
  return methods;
}

QgsGeometryCheck::CheckType QgsGeometryFollowBoundariesCheck::factoryCheckType()
{
  return QgsGeometryCheck::FeatureNodeCheck;
}
