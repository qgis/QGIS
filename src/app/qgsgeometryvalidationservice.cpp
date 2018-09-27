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
#include "qgsgeometryoptions.h"
#include "qgsanalysis.h"
#include "qgsgeometrycheckregistry.h"
#include "qgsgeometrycheckfactory.h"

QgsGeometryValidationService::QgsGeometryValidationService( QgsProject *project )
  : mProject( project )
{
  connect( project, &QgsProject::layersAdded, this, &QgsGeometryValidationService::onLayersAdded );
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
      connect( vectorLayer, &QgsVectorLayer::beforeCommitChanges, this, [this, vectorLayer]()
      {
        onBeforeCommitChanges( vectorLayer );
      } );

      enableLayerChecks( vectorLayer );
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

void QgsGeometryValidationService::onBeforeCommitChanges( QgsVectorLayer *layer )
{
  if ( !mTopologyChecksOk.value( layer ) )
  {
    triggerTopologyChecks( layer );
  }
}

void QgsGeometryValidationService::enableLayerChecks( QgsVectorLayer *layer )
{
  // TODO: finish all ongoing checks
  qDeleteAll( mSingleFeatureChecks.value( layer ) );

  // TODO: ownership and lifetime of the context!!
  auto context = new QgsGeometryCheckContext( 1, mProject->crs(), mProject->transformContext() );
  QList<QgsGeometryCheck *> layerChecks;

  QgsGeometryCheckRegistry *checkRegistry = QgsAnalysis::instance()->geometryCheckRegistry();

  const QStringList activeChecks = layer->geometryOptions()->geometryChecks();

  const QList<QgsGeometryCheckFactory *> singleCheckFactories = checkRegistry->geometryCheckFactories( layer, QgsGeometryCheck::SingleGeometryCheck );

  for ( QgsGeometryCheckFactory *factory : singleCheckFactories )
  {
    const QString checkId = factory->id();
    if ( activeChecks.contains( checkId ) )
    {
      const QVariantMap checkConfiguration = layer->geometryOptions()->checkConfiguration( checkId );
      layerChecks.append( factory->createGeometryCheck( context, checkConfiguration ) );
    }
  }

  QList<QgsSingleGeometryCheck *> singleGeometryChecks;
  for ( QgsGeometryCheck *check : qgis::as_const( layerChecks ) )
  {
    Q_ASSERT( dynamic_cast<QgsSingleGeometryCheck *>( check ) );
    singleGeometryChecks.append( dynamic_cast<QgsSingleGeometryCheck *>( check ) );
  }

  mSingleFeatureChecks.insert( layer, singleGeometryChecks );

#if 0
  const QList<QgsGeometryCheckFactory *> topologyCheckFactories = checkRegistry->geometryCheckFactories( layer, QgsGeometryCheck::SingleLayerTopologyCheck );

  for ( const QString &check : activeChecks )
  {
    checkRegistry->geometryCheckFactories( layer, QgsGeometryCheck::SingleLayerTopologyCheck );
  }
#endif
}

void QgsGeometryValidationService::cancelChecks( QgsVectorLayer *layer, QgsFeatureId fid )
{

}

void QgsGeometryValidationService::processFeature( QgsVectorLayer *layer, QgsFeatureId fid )
{
  emit geometryCheckStarted( layer, fid );

  QgsFeature feature = layer->getFeature( fid );

  const auto &checks = mSingleFeatureChecks.value( layer );

  // The errors are going to be sent out via a signal. We cannot keep ownership in here (or can we?)
  // nor can we be sure that a single slot is connected to the signal. So make it a shared_ptr.
  QList<std::shared_ptr<QgsSingleGeometryCheckError>> allErrors;
  for ( QgsSingleGeometryCheck *check : checks )
  {
    const auto errors = check->processGeometry( feature.geometry() );

    for ( auto error : errors )
      allErrors.append( std::shared_ptr<QgsSingleGeometryCheckError>( error ) );
  }

  emit geometryCheckCompleted( layer, fid, allErrors );
}

void QgsGeometryValidationService::triggerTopologyChecks( QgsVectorLayer *layer )
{

}
