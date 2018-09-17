/***************************************************************************
                      qgsgeometryvalidationservice.cpp
                     --------------------------------------
Date                 : 7.9.2018
Copyright            : (C) 2018 by Matthias Kuhn
email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsgeometryvalidationservice.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"

// TODO: Replace with registry
#include "qgsisvalidgeometrycheck.h"

QgsGeometryValidationService::QgsGeometryValidationService( QgsProject *project )
{
  connect( project, &QgsProject::layersAdded, this, &QgsGeometryValidationService::onLayersAdded );
  // TODO: should not provide a nullptr context
  mIsValidGeometryCheck = new QgsIsValidGeometryCheck( nullptr );
}

QgsGeometryValidationService::~QgsGeometryValidationService()
{
}

bool QgsGeometryValidationService::validationActive( QgsVectorLayer *layer, QgsFeatureId feature ) const
{
  return false;
}

void QgsGeometryValidationService::onLayersAdded( const QList<QgsMapLayer *> &layers )
{
  for ( QgsMapLayer *layer : layers )
  {
    QgsVectorLayer *vectorLayer = qobject_cast<QgsVectorLayer *>( layer );
    if ( vectorLayer )
    {
      connect( vectorLayer, &QgsVectorLayer::featureAdded, this, [this, vectorLayer]( QgsFeatureId fid )
      {
        onFeatureAdded( vectorLayer, fid );
      } );
      connect( vectorLayer, &QgsVectorLayer::geometryChanged, this, [this, vectorLayer]( QgsFeatureId fid, const QgsGeometry & geometry )
      {
        onGeometryChanged( vectorLayer, fid, geometry );
      } );
      connect( vectorLayer, &QgsVectorLayer::featureDeleted, this, [this, vectorLayer]( QgsFeatureId fid )
      {
        onFeatureDeleted( vectorLayer, fid );
      } );
    }
  }
}

void QgsGeometryValidationService::onFeatureAdded( QgsVectorLayer *layer, QgsFeatureId fid )
{
  processFeature( layer, fid );
}

void QgsGeometryValidationService::onGeometryChanged( QgsVectorLayer *layer, QgsFeatureId fid, const QgsGeometry &geometry )
{
  Q_UNUSED( geometry )

  cancelChecks( layer, fid );
  processFeature( layer, fid );
}

void QgsGeometryValidationService::onFeatureDeleted( QgsVectorLayer *layer, QgsFeatureId fid )
{
  cancelChecks( layer, fid );
}

void QgsGeometryValidationService::cancelChecks( QgsVectorLayer *layer, QgsFeatureId fid )
{

}

void QgsGeometryValidationService::processFeature( QgsVectorLayer *layer, QgsFeatureId fid )
{
  emit geometryCheckStarted( layer, fid );

  QgsFeature feature = layer->getFeature( fid );
  // TODO: this is a bit hardcore
  const auto errors = mIsValidGeometryCheck->processGeometry( feature.geometry() );
  // emit geometryCheckCompleted( layer, fid, errors );
}
