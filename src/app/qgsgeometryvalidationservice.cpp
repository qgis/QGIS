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
#include "qgsvectorlayereditbuffer.h"
#include "qgsvectorlayerfeaturepool.h"
#include "qgsfeedback.h"
#include "qgsreadwritelocker.h"

#include <QtConcurrent>
#include <QFutureWatcher>

QgsGeometryValidationService::QgsGeometryValidationService( QgsProject *project )
  : mProject( project )
{
  connect( project, &QgsProject::layersAdded, this, &QgsGeometryValidationService::onLayersAdded );
}

bool QgsGeometryValidationService::validationActive( QgsVectorLayer *layer, QgsFeatureId feature ) const
{
  return false;
}

void QgsGeometryValidationService::fixError( const QgsGeometryCheckError *error, int method )
{
  QgsGeometryCheck::Changes changes;
  QgsGeometryCheckError *nonconsterr = const_cast<QgsGeometryCheckError *>( error );
  error->check()->fixError( mFeaturePools, nonconsterr, method, QMap<QString, int>(), changes );
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
  if ( !mLayerCheckStates[layer].topologyChecks.empty() )
  {
    // TODO: Cancel topology checks
    layer->setAllowCommit( false );
  }
  processFeature( layer, fid );
}

void QgsGeometryValidationService::onGeometryChanged( QgsVectorLayer *layer, QgsFeatureId fid, const QgsGeometry &geometry )
{
  if ( !mLayerCheckStates[layer].topologyChecks.empty() )
  {
    // TODO: Cancel topology checks
    layer->setAllowCommit( false );
  }
  Q_UNUSED( geometry )

  cancelChecks( layer, fid );
  processFeature( layer, fid );
}

void QgsGeometryValidationService::onFeatureDeleted( QgsVectorLayer *layer, QgsFeatureId fid )
{
  if ( !mLayerCheckStates[layer].topologyChecks.empty() )
  {
    // TODO: Cancel topology checks
    layer->setAllowCommit( false );
  }

  cancelChecks( layer, fid );
}

void QgsGeometryValidationService::onBeforeCommitChanges( QgsVectorLayer *layer )
{
  if ( !mLayerCheckStates[layer].topologyChecks.empty() ) // TODO && topologyChecks not fulfilled
  {
    if ( !layer->allowCommit() )
    {
      emit warning( tr( "Can not save yet, we'll need to run some topology checks on your dataset first..." ) );
    }
    triggerTopologyChecks( layer );
  }
}

void QgsGeometryValidationService::enableLayerChecks( QgsVectorLayer *layer )
{
  // TODO: finish all ongoing checks
  qDeleteAll( mLayerCheckStates[layer].singleFeatureChecks );
  qDeleteAll( mLayerCheckStates[layer].topologyChecks );

  // TODO: ownership and lifetime of the context!!
  auto context = new QgsGeometryCheckContext( 8, mProject->crs(), mProject->transformContext() );
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

  mLayerCheckStates[layer].singleFeatureChecks = singleGeometryChecks;

  // Topology checks
  QList<QgsGeometryCheck *> topologyChecks;
  const QList<QgsGeometryCheckFactory *> topologyCheckFactories = checkRegistry->geometryCheckFactories( layer, QgsGeometryCheck::SingleLayerTopologyCheck );

  for ( QgsGeometryCheckFactory *factory : topologyCheckFactories )
  {
    const QString checkId = factory->id();
    if ( activeChecks.contains( checkId ) )
    {
      const QVariantMap checkConfiguration = layer->geometryOptions()->checkConfiguration( checkId );
      topologyChecks.append( factory->createGeometryCheck( context, checkConfiguration ) );
    }
  }

  mLayerCheckStates[layer].topologyChecks = topologyChecks;
}

void QgsGeometryValidationService::cancelChecks( QgsVectorLayer *layer, QgsFeatureId fid )
{

}

void QgsGeometryValidationService::processFeature( QgsVectorLayer *layer, QgsFeatureId fid )
{
  emit geometryCheckStarted( layer, fid );

  QgsFeature feature = layer->getFeature( fid );

  const auto &checks = mLayerCheckStates[layer].singleFeatureChecks;

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
  emit topologyChecksCleared( layer );

  QFutureWatcher<void> *futureWatcher = mLayerCheckStates[layer].topologyCheckFutureWatcher;
  if ( futureWatcher )
  {
    // Make sure no more checks are started first
    futureWatcher->cancel();

    // Tell all checks to stop asap
    const auto feedbacks = mLayerCheckStates[layer].topologyCheckFeedbacks;
    for ( QgsFeedback *feedback : feedbacks )
    {
      if ( feedback )
        feedback->cancel();
    }

    // The future watcher will take care of deleting
    mLayerCheckStates[layer].topologyCheckFeedbacks.clear();
  }

  QgsFeatureIds affectedFeatureIds = layer->editBuffer()->changedGeometries().keys().toSet();
  affectedFeatureIds.unite( layer->editBuffer()->addedFeatures().keys().toSet() );

  // TODO: ownership of these objects...
  QgsVectorLayerFeaturePool *featurePool = new QgsVectorLayerFeaturePool( layer );
  QList<QgsGeometryCheckError *> &allErrors = mLayerCheckStates[layer].topologyCheckErrors;
  QMap<QString, QgsFeatureIds> layerIds;

  QgsFeatureRequest request = QgsFeatureRequest( affectedFeatureIds ).setSubsetOfAttributes( QgsAttributeList() );
  QgsFeatureIterator it = layer->getFeatures( request );
  QgsFeature feature;
  QgsRectangle area;
  while ( it.nextFeature( feature ) )
  {
    area.combineExtentWith( feature.geometry().boundingBox() );
  }

  QgsFeatureRequest areaRequest = QgsFeatureRequest().setFilterRect( area );
  QgsFeatureIds checkFeatureIds = featurePool->getFeatures( areaRequest );

  layerIds.insert( layer->id(), checkFeatureIds );
  QgsGeometryCheck::LayerFeatureIds layerFeatureIds( layerIds );

  if ( !mFeaturePools.contains( layer->id() ) )
  {
    mFeaturePools.insert( layer->id(), featurePool );
  }

  const QList<QgsGeometryCheck *> checks = mLayerCheckStates[layer].topologyChecks;

  QMap<const QgsGeometryCheck *, QgsFeedback *> feedbacks;
  for ( QgsGeometryCheck *check : checks )
    feedbacks.insert( check, new QgsFeedback() );

  mLayerCheckStates[layer].topologyCheckFeedbacks = feedbacks.values();

  QFuture<void> future = QtConcurrent::map( checks, [&allErrors, layerFeatureIds, layer, feedbacks, this]( const QgsGeometryCheck * check )
  {
    // Watch out with the layer pointer in here. We are running in a thread, so we do not want to actually use it
    // except for using its address to report the error.
    QList<QgsGeometryCheckError *> errors;
    QStringList messages; // Do we really need these?
    QgsFeedback *feedback = feedbacks.value( check );

    check->collectErrors( mFeaturePools, errors, messages, feedback, layerFeatureIds );
    QgsReadWriteLocker errorLocker( mTopologyCheckLock, QgsReadWriteLocker::Write );
    allErrors.append( errors );

    QList<std::shared_ptr<QgsGeometryCheckError> > sharedErrors;
    for ( QgsGeometryCheckError *error : errors )
    {
      sharedErrors.append( std::shared_ptr<QgsGeometryCheckError>( error ) );
    }
    if ( !feedback->isCanceled() )
      emit topologyChecksUpdated( layer, sharedErrors );

    errorLocker.unlock();
  } );

  futureWatcher = new QFutureWatcher<void>();
  futureWatcher->setFuture( future );

  connect( futureWatcher, &QFutureWatcherBase::finished, this, [&allErrors, layer, feedbacks, futureWatcher, this]()
  {
    QgsReadWriteLocker errorLocker( mTopologyCheckLock, QgsReadWriteLocker::Read );
    layer->setAllowCommit( allErrors.empty() );
    errorLocker.unlock();
    qDeleteAll( feedbacks.values() );
    futureWatcher->deleteLater();
    if ( mLayerCheckStates[layer].topologyCheckFutureWatcher == futureWatcher )
      mLayerCheckStates[layer].topologyCheckFutureWatcher = nullptr;
  } );

  mLayerCheckStates[layer].topologyCheckFutureWatcher = futureWatcher;
}
