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

void QgsGeometryValidationService::fixError( QgsGeometryCheckError *error, int method )
{
  QgsGeometryCheck::Changes changes;
  error->check()->fixError( mFeaturePools, error, method, QMap<QString, int>(), changes );
  error->setFixed( method );

  QgsFeaturePool *featurePool = mFeaturePools.value( error->layerId() );

  QgsVectorLayer *layer;

  if ( featurePool )
    layer = featurePool->layer();
  else
  {
    // Some checks don't tell us on which layer they are because they are able to do cross-layer checks.
    // E.g. the gap check will report in such a way

    for ( auto layerCheck = mLayerChecks.constBegin(); layerCheck != mLayerChecks.constEnd(); ++layerCheck )
    {
      const QList<std::shared_ptr<QgsGeometryCheckError>> &topologyCheckErrors = layerCheck.value().topologyCheckErrors;
      for ( const auto &checkError : topologyCheckErrors )
      {
        if ( checkError.get() == error )
        {
          layer = layerCheck.key();
          break;
        }
      }
    }
  }

  layer->triggerRepaint();

  emit topologyErrorUpdated( layer, error );
}

void QgsGeometryValidationService::onLayersAdded( const QList<QgsMapLayer *> &layers )
{
  for ( QgsMapLayer *layer : layers )
  {
    QgsVectorLayer *vectorLayer = qobject_cast<QgsVectorLayer *>( layer );
    if ( vectorLayer )
    {
      connect( vectorLayer->geometryOptions(), &QgsGeometryOptions::checkConfigurationChanged, this, [this, vectorLayer]()
      {
        enableLayerChecks( vectorLayer );
      }, Qt::UniqueConnection );

      connect( vectorLayer, &QgsVectorLayer::destroyed, this, [vectorLayer, this]()
      {
        cleanupLayerChecks( vectorLayer );
        mLayerChecks.remove( vectorLayer );
      } );

      enableLayerChecks( vectorLayer );
    }
  }
}

void QgsGeometryValidationService::onFeatureAdded( QgsVectorLayer *layer, QgsFeatureId fid )
{
  if ( !mLayerChecks[layer].topologyChecks.empty() )
  {
    invalidateTopologyChecks( layer );
  }
  processFeature( layer, fid );
}

void QgsGeometryValidationService::onGeometryChanged( QgsVectorLayer *layer, QgsFeatureId fid, const QgsGeometry &geometry )
{
  Q_UNUSED( geometry )
  // It would be nice to use the geometry here for the tests.
  // But:
  //  1. other codepaths to the checks also have no geometry (feature added / feature deleted)
  //  2. and looking it up from the edit buffer (in memory) is really fast.
  // so in short: it's still a good idea, but not as important as on first thought.

  if ( !mLayerChecks[layer].topologyChecks.empty() )
  {
    invalidateTopologyChecks( layer );
  }

  processFeature( layer, fid );
}

void QgsGeometryValidationService::onFeatureDeleted( QgsVectorLayer *layer, QgsFeatureId fid )
{
  if ( !mLayerChecks[layer].topologyChecks.empty() )
  {
    invalidateTopologyChecks( layer );
  }

  // There should be no geometry errors on an inexistent feature, right?
  emit geometryCheckCompleted( layer, fid, QList<std::shared_ptr<QgsSingleGeometryCheckError>>() );
}

void QgsGeometryValidationService::onBeforeCommitChanges( QgsVectorLayer *layer )
{
  if ( !mLayerChecks[layer].topologyChecks.empty() ) // TODO && topologyChecks not fulfilled
  {
    if ( !layer->allowCommit() )
    {
      emit warning( tr( "Can not save yet, we'll need to run some topology checks on your dataset first..." ) );
    }
    triggerTopologyChecks( layer );
  }
}

void QgsGeometryValidationService::cleanupLayerChecks( QgsVectorLayer *layer )
{
  if ( !mLayerChecks.contains( layer ) )
    return;

  VectorLayerCheckInformation &checkInformation = mLayerChecks[layer];

  cancelTopologyCheck( layer );

  qDeleteAll( checkInformation.singleFeatureChecks );
  qDeleteAll( checkInformation.topologyChecks );
  checkInformation.context.reset();
}

void QgsGeometryValidationService::enableLayerChecks( QgsVectorLayer *layer )
{
  if ( layer->geometryOptions()->geometryChecks().empty() && !mLayerChecks.contains( layer ) )
    return;

  VectorLayerCheckInformation &checkInformation = mLayerChecks[layer];

  cleanupLayerChecks( layer );

  if ( layer->geometryOptions()->geometryChecks().empty() )
  {
    for ( QMetaObject::Connection connection : qgis::as_const( checkInformation.connections ) )
    {
      disconnect( connection );
    }
    checkInformation.connections.clear();
    return;
  }

  int precision = log10( layer->geometryOptions()->geometryPrecision() ) * -1;
  if ( precision == 0 )
    precision = 8;
  checkInformation.context = qgis::make_unique<QgsGeometryCheckContext>( precision, mProject->crs(), mProject->transformContext() );

  QList<QgsGeometryCheck *> layerChecks;

  QgsGeometryCheckRegistry *checkRegistry = QgsAnalysis::instance()->geometryCheckRegistry();

  const QStringList activeChecks = layer->geometryOptions()->geometryChecks();

  const QList<QgsGeometryCheckFactory *> singleCheckFactories = checkRegistry->geometryCheckFactories( layer, QgsGeometryCheck::FeatureNodeCheck, QgsGeometryCheck::AvailableInValidation );

  for ( QgsGeometryCheckFactory *factory : singleCheckFactories )
  {
    const QString checkId = factory->id();
    if ( activeChecks.contains( checkId ) )
    {
      const QVariantMap checkConfiguration = layer->geometryOptions()->checkConfiguration( checkId );
      layerChecks.append( factory->createGeometryCheck( checkInformation.context.get(), checkConfiguration ) );
    }
  }

  QList<QgsSingleGeometryCheck *> singleGeometryChecks;
  for ( QgsGeometryCheck *check : qgis::as_const( layerChecks ) )
  {
    Q_ASSERT( dynamic_cast<QgsSingleGeometryCheck *>( check ) );
    singleGeometryChecks.append( dynamic_cast<QgsSingleGeometryCheck *>( check ) );
  }

  checkInformation.singleFeatureChecks = singleGeometryChecks;

  // Topology checks
  QList<QgsGeometryCheck *> topologyChecks;
  const QList<QgsGeometryCheckFactory *> topologyCheckFactories = checkRegistry->geometryCheckFactories( layer, QgsGeometryCheck::LayerCheck, QgsGeometryCheck::AvailableInValidation );

  for ( QgsGeometryCheckFactory *factory : topologyCheckFactories )
  {
    const QString checkId = factory->id();
    if ( activeChecks.contains( checkId ) )
    {
      const QVariantMap checkConfiguration = layer->geometryOptions()->checkConfiguration( checkId );
      topologyChecks.append( factory->createGeometryCheck( checkInformation.context.get(), checkConfiguration ) );
    }
  }

  checkInformation.topologyChecks = topologyChecks;

  // Connect to all modifications on a layer that can introduce a geometry or topology error
  // Also connect to the beforeCommitChanges signal, so we can trigger topology checks
  // We keep all connections around in a list, so if in the future all checks get disabled
  // we can kill those connections to be sure the layer does not even get a tiny bit of overhead.
  checkInformation.connections
      << connect( layer, &QgsVectorLayer::featureAdded, this, [this, layer]( QgsFeatureId fid )
  {
    onFeatureAdded( layer, fid );
  }, Qt::UniqueConnection );
  checkInformation.connections
      << connect( layer, &QgsVectorLayer::geometryChanged, this, [this, layer]( QgsFeatureId fid, const QgsGeometry & geometry )
  {
    onGeometryChanged( layer, fid, geometry );
  }, Qt::UniqueConnection );
  checkInformation.connections
      << connect( layer, &QgsVectorLayer::featureDeleted, this, [this, layer]( QgsFeatureId fid )
  {
    onFeatureDeleted( layer, fid );
  }, Qt::UniqueConnection );
  checkInformation.connections
      << connect( layer, &QgsVectorLayer::beforeCommitChanges, this, [this, layer]()
  {
    onBeforeCommitChanges( layer );
  }, Qt::UniqueConnection );
}

void QgsGeometryValidationService::cancelTopologyCheck( QgsVectorLayer *layer )
{
  QFutureWatcher<void> *futureWatcher = mLayerChecks[layer].topologyCheckFutureWatcher;
  if ( futureWatcher )
  {
    // Make sure no more checks are started first
    futureWatcher->cancel();

    // Tell all checks to stop asap
    const auto feedbacks = mLayerChecks[layer].topologyCheckFeedbacks;
    for ( QgsFeedback *feedback : feedbacks )
    {
      if ( feedback )
        feedback->cancel();
    }

    futureWatcher->waitForFinished();
    mLayerChecks[layer].topologyCheckFutureWatcher = nullptr;
  }
}

void QgsGeometryValidationService::invalidateTopologyChecks( QgsVectorLayer *layer )
{
  cancelTopologyCheck( layer );
  layer->setAllowCommit( false );
}

void QgsGeometryValidationService::processFeature( QgsVectorLayer *layer, QgsFeatureId fid )
{
  emit geometryCheckStarted( layer, fid );

  QgsFeature feature = layer->getFeature( fid );

  const auto &checks = mLayerChecks[layer].singleFeatureChecks;

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
  cancelTopologyCheck( layer );

  QgsFeatureIds affectedFeatureIds;
  if ( layer->editBuffer() )
  {
    affectedFeatureIds = layer->editBuffer()->changedGeometries().keys().toSet();
    affectedFeatureIds.unite( layer->editBuffer()->addedFeatures().keys().toSet() );
  }

  QgsFeaturePool *featurePool = mFeaturePools.value( layer->id() );
  if ( !featurePool )
  {
    featurePool = new QgsVectorLayerFeaturePool( layer );
    mFeaturePools.insert( layer->id(), featurePool );
  }

  QList<std::shared_ptr<QgsGeometryCheckError>> &allErrors = mLayerChecks[layer].topologyCheckErrors;
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

  const QList<QgsGeometryCheck *> checks = mLayerChecks[layer].topologyChecks;

  QMap<const QgsGeometryCheck *, QgsFeedback *> feedbacks;
  for ( QgsGeometryCheck *check : checks )
    feedbacks.insert( check, new QgsFeedback() );

  mLayerChecks[layer].topologyCheckFeedbacks = feedbacks.values();

  QFuture<void> future = QtConcurrent::map( checks, [&allErrors, layerFeatureIds, layer, feedbacks, this]( const QgsGeometryCheck * check )
  {
    // Watch out with the layer pointer in here. We are running in a thread, so we do not want to actually use it
    // except for using its address to report the error.
    QList<QgsGeometryCheckError *> errors;
    QStringList messages; // Do we really need these?
    QgsFeedback *feedback = feedbacks.value( check );

    check->collectErrors( mFeaturePools, errors, messages, feedback, layerFeatureIds );
    QgsReadWriteLocker errorLocker( mTopologyCheckLock, QgsReadWriteLocker::Write );

    QList<std::shared_ptr<QgsGeometryCheckError> > sharedErrors;
    for ( QgsGeometryCheckError *error : errors )
    {
      sharedErrors.append( std::shared_ptr<QgsGeometryCheckError>( error ) );
    }

    allErrors.append( sharedErrors );
    if ( !feedback->isCanceled() )
      emit topologyChecksUpdated( layer, sharedErrors );

    errorLocker.unlock();
  } );

  QFutureWatcher<void> *futureWatcher = new QFutureWatcher<void>();
  futureWatcher->setFuture( future );

  connect( futureWatcher, &QFutureWatcherBase::finished, this, [&allErrors, layer, feedbacks, futureWatcher, this]()
  {
    QgsReadWriteLocker errorLocker( mTopologyCheckLock, QgsReadWriteLocker::Read );
    layer->setAllowCommit( allErrors.empty() );
    errorLocker.unlock();
    qDeleteAll( feedbacks.values() );
    futureWatcher->deleteLater();
    if ( mLayerChecks[layer].topologyCheckFutureWatcher == futureWatcher )
      mLayerChecks[layer].topologyCheckFutureWatcher = nullptr;
  } );

  mLayerChecks[layer].topologyCheckFutureWatcher = futureWatcher;
}
