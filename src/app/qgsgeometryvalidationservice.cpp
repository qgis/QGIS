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
#include "qgsmessagebar.h"
#include "qgsmessagebaritem.h"
#include "qgsmessagelog.h"

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

  QgsVectorLayer *layer = nullptr;

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

  if ( layer )
  {
    layer->triggerRepaint();

    emit topologyErrorUpdated( layer, error );
  }
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
      } );

      connect( vectorLayer->geometryOptions(), &QgsGeometryOptions::geometryChecksChanged, this, [this, vectorLayer]()
      {
        enableLayerChecks( vectorLayer );
      } );

      connect( vectorLayer, &QgsVectorLayer::destroyed, this, [vectorLayer, this]()
      {
        cleanupLayerChecks( vectorLayer );
        mLayerChecks.remove( vectorLayer );
      } );

      connect( vectorLayer, &QgsMapLayer::beforeResolveReferences, this, [this, vectorLayer]()
      {
        enableLayerChecks( vectorLayer );
      } );
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

  mLayerChecks[layer].singleFeatureCheckErrors.remove( fid );

  // There should be no geometry errors on a non-existent feature, right?
  emit geometryCheckCompleted( layer, fid, QList<std::shared_ptr<QgsSingleGeometryCheckError>>() );
}

void QgsGeometryValidationService::onBeforeCommitChanges( QgsVectorLayer *layer, bool stopEditing )
{
  if ( mLayerChecks[layer].topologyChecks.empty() && !layer->allowCommit() )
  {
    showMessage( tr( "Geometry errors have been found. Please fix the errors before saving the layer." ) );
  }
  if ( !mBypassChecks && !mLayerChecks[layer].topologyChecks.empty() )
  {
    if ( !layer->allowCommit() )
    {
      showMessage( tr( "Running geometry validation checks before saving…" ) );
    }

    mLayerChecks[layer].commitPending = true;

    triggerTopologyChecks( layer, stopEditing );
  }
}

void QgsGeometryValidationService::onEditingStopped( QgsVectorLayer *layer )
{
  cancelTopologyCheck( layer );
  clearTopologyChecks( layer );
}

void QgsGeometryValidationService::showMessage( const QString &message )
{
  mMessageBar->popWidget( mMessageBarItem );
  mMessageBarItem = QgsMessageBar::createMessage( tr( "Geometry Validation" ),  message );
  mMessageBarItem->setDuration( 5 );
  mMessageBar->pushItem( mMessageBarItem );
}

void QgsGeometryValidationService::cleanupLayerChecks( QgsVectorLayer *layer )
{
  if ( !mLayerChecks.contains( layer ) )
    return;

  VectorLayerCheckInformation &checkInformation = mLayerChecks[layer];

  cancelTopologyCheck( layer );
  clearTopologyChecks( layer );

  qDeleteAll( checkInformation.singleFeatureChecks );
  checkInformation.singleFeatureChecks.clear();
  qDeleteAll( checkInformation.topologyChecks );
  checkInformation.topologyChecks.clear();
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
    for ( QMetaObject::Connection connection : std::as_const( checkInformation.connections ) )
    {
      disconnect( connection );
    }
    checkInformation.connections.clear();
    return;
  }

  int precision = 0;
  if ( layer->geometryOptions()->geometryPrecision() == 0 )
    precision = 8;
  else
  {
    precision = log10( layer->geometryOptions()->geometryPrecision() ) * -1;

    if ( precision < 1 || precision > 13 )
      precision = 8;
  }

  checkInformation.context = std::make_unique<QgsGeometryCheckContext>( precision, mProject->crs(), mProject->transformContext(), mProject );

  QList<QgsGeometryCheck *> layerChecks;

  QgsGeometryCheckRegistry *checkRegistry = QgsAnalysis::geometryCheckRegistry();

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
  for ( QgsGeometryCheck *check : std::as_const( layerChecks ) )
  {
    Q_ASSERT( dynamic_cast<QgsSingleGeometryCheck *>( check ) );
    singleGeometryChecks.append( dynamic_cast<QgsSingleGeometryCheck *>( check ) );
  }

  if ( singleGeometryChecks.empty() )
  {
    mLayerChecks[layer].singleFeatureCheckErrors.clear();
    emit singleGeometryCheckCleared( layer );
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

      if ( checkConfiguration.value( QStringLiteral( "allowedGapsEnabled" ) ).toBool() )
      {
        QgsVectorLayer *gapsLayer = QgsProject::instance()->mapLayer<QgsVectorLayer *>( checkConfiguration.value( "allowedGapsLayer" ).toString() );
        if ( gapsLayer )
        {
          connect( layer, &QgsVectorLayer::editingStarted, gapsLayer, [gapsLayer] { gapsLayer->startEditing(); } );
          connect( layer, &QgsVectorLayer::beforeRollBack, gapsLayer, [gapsLayer] { gapsLayer->rollBack(); } );
          connect( layer, &QgsVectorLayer::afterCommitChanges, gapsLayer, [gapsLayer] { gapsLayer->commitChanges(); } );
        }
        else
        {
          QgsMessageLog::logMessage( tr( "Allowed gaps layer %1 configured but not loaded. Allowed gaps not working." ).arg( checkConfiguration.value( "allowedGapsLayer" ).toString() ), tr( "Geometry validation" ) );
        }
      }
    }
  }

  checkInformation.topologyChecks = topologyChecks;

  if ( checkInformation.connections.empty() )
  {
    // Connect to all modifications on a layer that can introduce a geometry or topology error
    // Also connect to the beforeCommitChanges signal, so we can trigger topology checks
    // We keep all connections around in a list, so if in the future all checks get disabled
    // we can kill those connections to be sure the layer does not even get a tiny bit of overhead.
    checkInformation.connections
        << connect( layer, &QgsVectorLayer::featureAdded, this, [this, layer]( QgsFeatureId fid )
    {
      onFeatureAdded( layer, fid );
    } );
    checkInformation.connections
        << connect( layer, &QgsVectorLayer::geometryChanged, this, [this, layer]( QgsFeatureId fid, const QgsGeometry & geometry )
    {
      onGeometryChanged( layer, fid, geometry );
    } );
    checkInformation.connections
        << connect( layer, &QgsVectorLayer::featureDeleted, this, [this, layer]( QgsFeatureId fid )
    {
      onFeatureDeleted( layer, fid );
    } );
    checkInformation.connections
        << connect( layer, &QgsVectorLayer::beforeCommitChanges, this, [this, layer]( bool stopEditing )
    {
      onBeforeCommitChanges( layer, stopEditing );
    } );
    checkInformation.connections
        << connect( layer, &QgsVectorLayer::editingStopped, this, [this, layer]()
    {
      onEditingStopped( layer );
    } );
  }
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

void QgsGeometryValidationService::clearTopologyChecks( QgsVectorLayer *layer )
{
  QList<std::shared_ptr<QgsGeometryCheckError>> &allErrors = mLayerChecks[layer].topologyCheckErrors;
  allErrors.clear();
  layer->setAllowCommit( mLayerChecks[layer].singleFeatureCheckErrors.empty() );

  emit topologyChecksCleared( layer );
}

void QgsGeometryValidationService::invalidateTopologyChecks( QgsVectorLayer *layer )
{
  cancelTopologyCheck( layer );
  layer->setAllowCommit( false );
}

void QgsGeometryValidationService::processFeature( QgsVectorLayer *layer, QgsFeatureId fid )
{
  if ( !mLayerChecks.contains( layer ) )
    return;

  const QList< QgsSingleGeometryCheck * > checks = mLayerChecks[layer].singleFeatureChecks;
  if ( checks.empty() )
    return;

  emit geometryCheckStarted( layer, fid );

  QgsGeometry geometry = layer->getGeometry( fid );

  mLayerChecks[layer].singleFeatureCheckErrors.remove( fid );

  // The errors are going to be sent out via a signal. We cannot keep ownership in here (or can we?)
  // nor can we be sure that a single slot is connected to the signal. So make it a shared_ptr.
  QList<std::shared_ptr<QgsSingleGeometryCheckError>> allErrors;
  for ( QgsSingleGeometryCheck *check : checks )
  {
    const auto errors = check->processGeometry( geometry );

    for ( auto error : errors )
      allErrors.append( std::shared_ptr<QgsSingleGeometryCheckError>( error ) );
  }

  if ( !allErrors.empty() )
    mLayerChecks[layer].singleFeatureCheckErrors.insert( fid, allErrors );

  if ( !mLayerChecks[layer].singleFeatureCheckErrors.empty() )
    layer->setAllowCommit( false );
  else if ( mLayerChecks[layer].topologyChecks.empty() )
    layer->setAllowCommit( true );

  emit geometryCheckCompleted( layer, fid, allErrors );
}

void QgsGeometryValidationService::setMessageBar( QgsMessageBar *messageBar )
{
  mMessageBar = messageBar;
}

void QgsGeometryValidationService::triggerTopologyChecks( QgsVectorLayer *layer, bool stopEditing )
{
  cancelTopologyCheck( layer );
  clearTopologyChecks( layer );
  layer->setAllowCommit( false );

  QgsFeatureIds affectedFeatureIds;
  if ( layer->editBuffer() )
  {
    affectedFeatureIds = qgis::listToSet( layer->editBuffer()->changedGeometries().keys() );
    affectedFeatureIds.unite( qgis::listToSet( layer->editBuffer()->addedFeatures().keys() ) );
  }

  const QString layerId = layer->id();
  QgsFeaturePool *featurePool = mFeaturePools.value( layerId );
  if ( !featurePool )
  {
    featurePool = new QgsVectorLayerFeaturePool( layer );
    mFeaturePools.insert( layerId, featurePool );
  }

  QList<std::shared_ptr<QgsGeometryCheckError>> &allErrors = mLayerChecks[layer].topologyCheckErrors;

  QMap<QString, QgsFeatureIds> layerIds;

  QgsFeatureRequest request = QgsFeatureRequest( affectedFeatureIds ).setNoAttributes();
  QgsFeatureIterator it = layer->getFeatures( request );
  QgsFeature feature;
  QgsRectangle area;
  while ( it.nextFeature( feature ) )
  {
    area.combineExtentWith( feature.geometry().boundingBox() );
  }

  QgsFeatureRequest areaRequest = QgsFeatureRequest().setFilterRect( area );
  QgsFeatureIds checkFeatureIds = featurePool->getFeatures( areaRequest );

  layerIds.insert( layerId, checkFeatureIds );
  QgsGeometryCheck::LayerFeatureIds layerFeatureIds( layerIds );

  const QList<QgsGeometryCheck *> checks = mLayerChecks[layer].topologyChecks;

  QHash<const QgsGeometryCheck *, QgsFeedback *> feedbacks;
  for ( QgsGeometryCheck *check : checks )
  {
    feedbacks.insert( check, new QgsFeedback() );
    check->prepare( mLayerChecks[layer].context.get(), layer->geometryOptions()->checkConfiguration( check->id() ) );
  }

  mLayerChecks[layer].topologyCheckFeedbacks = feedbacks.values();

  QFutureWatcher<void> *futureWatcher = new QFutureWatcher<void>();
  connect( futureWatcher, &QFutureWatcherBase::finished, this, [&allErrors, layer, feedbacks, futureWatcher, stopEditing, this]()
  {
    QgsReadWriteLocker errorLocker( mTopologyCheckLock, QgsReadWriteLocker::Read );
    layer->setAllowCommit( allErrors.empty() && mLayerChecks[layer].singleFeatureCheckErrors.empty() );
    errorLocker.unlock();
    qDeleteAll( feedbacks );
    futureWatcher->deleteLater();
    if ( mLayerChecks[layer].topologyCheckFutureWatcher == futureWatcher )
      mLayerChecks[layer].topologyCheckFutureWatcher = nullptr;

    if ( !allErrors.empty() || !mLayerChecks[layer].singleFeatureCheckErrors.empty() )
    {
      if ( mLayerChecks[layer].commitPending )
        showMessage( tr( "Geometry errors have been found. Please fix the errors before saving the layer." ) );
      else
        showMessage( tr( "Geometry errors have been found." ) );
    }
    if ( allErrors.empty() && mLayerChecks[layer].singleFeatureCheckErrors.empty() && mLayerChecks[layer].commitPending )
    {
      mBypassChecks = true;
      layer->commitChanges( stopEditing );
      mBypassChecks = false;
      mMessageBar->popWidget( mMessageBarItem );
      mMessageBarItem = nullptr;
    }

    mLayerChecks[layer].commitPending = false;
  } );

  QFuture<void> future = QtConcurrent::map( checks, [&allErrors, layerFeatureIds, layer, layerId, feedbacks, affectedFeatureIds, this]( const QgsGeometryCheck * check )
  {
    // Watch out with the layer pointer in here. We are running in a thread, so we do not want to actually use it
    // except for using its address to report the error.
    QList<QgsGeometryCheckError *> errors;
    QStringList messages; // Do we really need these?
    QgsFeedback *feedback = feedbacks.value( check );

    check->collectErrors( mFeaturePools, errors, messages, feedback, layerFeatureIds );
    QgsReadWriteLocker errorLocker( mTopologyCheckLock, QgsReadWriteLocker::Write );

    QList<std::shared_ptr<QgsGeometryCheckError> > sharedErrors;
    for ( QgsGeometryCheckError *err : errors )
    {
      std::shared_ptr<QgsGeometryCheckError> error( err );
      // Check if the error happened in one of the edited / checked features
      // Errors which are happen to be in the same area "by chance" are ignored.
      const auto involvedFeatures = error->involvedFeatures();

      bool errorAffectsEditedFeature = true;
      if ( !involvedFeatures.isEmpty() )
      {
        errorAffectsEditedFeature = false;
        const auto involvedFids = involvedFeatures.value( layerId );
        for ( const QgsFeatureId id : involvedFids )
        {
          if ( affectedFeatureIds.contains( id ) )
          {
            errorAffectsEditedFeature = true;
            break;
          }
        }
      }

      if ( errorAffectsEditedFeature )
        sharedErrors.append( error );
    }

    allErrors.append( sharedErrors );
    if ( !feedback->isCanceled() )
      emit topologyChecksUpdated( layer, sharedErrors );

    errorLocker.unlock();
  } );

  futureWatcher->setFuture( future );

  mLayerChecks[layer].topologyCheckFutureWatcher = futureWatcher;
}
