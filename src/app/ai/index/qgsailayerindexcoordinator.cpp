/***************************************************************************
    qgsailayerindexcoordinator.cpp
    ---------------------
    begin                : May 2026
    copyright            : (C) 2026 by Francesco Mazzi
    email                : francemazzi at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsailayerindexcoordinator.h"

#include <algorithm>

#include "qgsaiworkspaceindex.h"
#include "qgsmaplayer.h"
#include "qgsmessagelog.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"

#include <QString>

#include "moc_qgsailayerindexcoordinator.cpp"

using namespace Qt::StringLiterals;

QgsAiLayerIndexCoordinator::QgsAiLayerIndexCoordinator( QgsAiWorkspaceIndex *index, QObject *parent )
  : QObject( parent )
  , mIndex( index )
  , mProject( QgsProject::instance() )
{
  mDebounceTimer.setSingleShot( true );
  connect( &mDebounceTimer, &QTimer::timeout, this, &QgsAiLayerIndexCoordinator::flushDirty );
}

void QgsAiLayerIndexCoordinator::setEnabled( bool enabled )
{
  if ( mEnabled == enabled )
    return;
  mEnabled = enabled;
  if ( mEnabled )
    connectProjectSignals();
  else
    disconnectProjectSignals();
}

void QgsAiLayerIndexCoordinator::setDebounceMs( int ms )
{
  mDebounceMs = std::max( 0, ms );
}

void QgsAiLayerIndexCoordinator::setProject( QgsProject *project )
{
  if ( mProject == project )
    return;
  const bool wasEnabled = mEnabled;
  if ( wasEnabled )
    disconnectProjectSignals();
  mProject = project ? project : QgsProject::instance();
  if ( wasEnabled )
    connectProjectSignals();
}

void QgsAiLayerIndexCoordinator::connectProjectSignals()
{
  if ( !mProject )
    return;
  connect( mProject, &QgsProject::layerWasAdded, this, &QgsAiLayerIndexCoordinator::onLayerAdded );
  connect( mProject, qOverload<const QString &>( &QgsProject::layerWillBeRemoved ), this, &QgsAiLayerIndexCoordinator::onLayerWillBeRemoved );
  // Wire and enqueue already-loaded layers too so the current project is indexed
  // as soon as automatic layer indexing is enabled.
  const QMap<QString, QgsMapLayer *> existing = mProject->mapLayers();
  for ( auto it = existing.constBegin(); it != existing.constEnd(); ++it )
    connectLayerSignals( it.value() );
  scheduleAllLayers();
}

void QgsAiLayerIndexCoordinator::disconnectProjectSignals()
{
  if ( !mProject )
    return;
  disconnect( mProject, nullptr, this, nullptr );
  const QMap<QString, QgsMapLayer *> existing = mProject->mapLayers();
  for ( auto it = existing.constBegin(); it != existing.constEnd(); ++it )
  {
    if ( it.value() )
      disconnect( it.value(), nullptr, this, nullptr );
  }
}

void QgsAiLayerIndexCoordinator::connectLayerSignals( QgsMapLayer *layer )
{
  if ( !layer )
    return;

  connect( layer, &QgsMapLayer::layerModified, this, &QgsAiLayerIndexCoordinator::onLayerChanged, Qt::UniqueConnection );
  connect( layer, &QgsMapLayer::dataChanged, this, &QgsAiLayerIndexCoordinator::onLayerChanged, Qt::UniqueConnection );
  connect( layer, &QgsMapLayer::dataSourceChanged, this, &QgsAiLayerIndexCoordinator::onLayerChanged, Qt::UniqueConnection );
  connect( layer, &QgsMapLayer::editingStopped, this, &QgsAiLayerIndexCoordinator::onLayerChanged, Qt::UniqueConnection );

  if ( QgsVectorLayer *v = qobject_cast<QgsVectorLayer *>( layer ) )
  {
    connect( v, &QgsVectorLayer::committedAttributesDeleted, this, &QgsAiLayerIndexCoordinator::onVectorLayerCommitted, Qt::UniqueConnection );
    connect( v, &QgsVectorLayer::committedAttributesAdded, this, &QgsAiLayerIndexCoordinator::onVectorLayerCommitted, Qt::UniqueConnection );
    connect( v, &QgsVectorLayer::committedFeaturesAdded, this, &QgsAiLayerIndexCoordinator::onVectorLayerCommitted, Qt::UniqueConnection );
    connect( v, &QgsVectorLayer::committedFeaturesRemoved, this, &QgsAiLayerIndexCoordinator::onVectorLayerCommitted, Qt::UniqueConnection );
    connect( v, &QgsVectorLayer::committedAttributeValuesChanges, this, &QgsAiLayerIndexCoordinator::onVectorLayerCommitted, Qt::UniqueConnection );
    connect( v, &QgsVectorLayer::committedGeometriesChanges, this, &QgsAiLayerIndexCoordinator::onVectorLayerCommitted, Qt::UniqueConnection );
  }
}

void QgsAiLayerIndexCoordinator::onLayerAdded( QgsMapLayer *layer )
{
  if ( !layer )
    return;
  connectLayerSignals( layer );
  scheduleDirty( layer->id() );
}

void QgsAiLayerIndexCoordinator::onLayerWillBeRemoved( const QString &layerId )
{
  // Remove immediately from the index AND drop any pending dirty entry.
  mDirtyLayers.remove( layerId );
  if ( !mIndex )
    return;
  QString err;
  if ( !mIndex->removeLayer( layerId, &err ) )
    QgsMessageLog::logMessage( u"Layer index: removeLayer(%1) failed: %2"_s.arg( layerId, err ), u"AI/Index"_s, Qgis::MessageLevel::Warning, false );
}

void QgsAiLayerIndexCoordinator::onLayerChanged()
{
  QgsMapLayer *layer = qobject_cast<QgsMapLayer *>( sender() );
  if ( !layer )
    return;
  scheduleDirty( layer->id() );
}

void QgsAiLayerIndexCoordinator::onVectorLayerCommitted( const QString &layerId )
{
  scheduleDirty( layerId );
}

void QgsAiLayerIndexCoordinator::scheduleAllLayers()
{
  if ( !mProject )
    return;

  const QMap<QString, QgsMapLayer *> existing = mProject->mapLayers();
  for ( auto it = existing.constBegin(); it != existing.constEnd(); ++it )
  {
    if ( it.value() )
      scheduleDirty( it.value()->id() );
  }
}

void QgsAiLayerIndexCoordinator::scheduleDirty( const QString &layerId )
{
  if ( layerId.isEmpty() )
    return;
  if ( !mIndex || !mIndex->embeddingProviderAvailable() )
    return;
  mDirtyLayers.insert( layerId );
  mDebounceTimer.start( mDebounceMs );
}

void QgsAiLayerIndexCoordinator::flushDirty()
{
  if ( !mIndex || !mEnabled || !mIndex->embeddingProviderAvailable() )
  {
    mDirtyLayers.clear();
    return;
  }

  const QList<QString> toProcess( mDirtyLayers.constBegin(), mDirtyLayers.constEnd() );
  mDirtyLayers.clear();

  for ( int i = 0; i < toProcess.size(); ++i )
  {
    const QString &layerId = toProcess.at( i );
    emit reindexStarted( layerId );
    QString err;
    const bool ok = mIndex->reindexLayer( layerId, &err );
    if ( !ok && !mIndex->embeddingProviderAvailable() )
    {
      // The embedding provider became unavailable mid-batch (e.g. a remote 401/403
      // tripped the circuit breaker). Stop now instead of logging one warning per
      // remaining layer; re-queue the unprocessed layers so they retry once the
      // provider is available again.
      for ( int j = i; j < toProcess.size(); ++j )
        mDirtyLayers.insert( toProcess.at( j ) );
      emit reindexFinished( layerId, ok, err );
      break;
    }
    if ( !ok )
      QgsMessageLog::logMessage( u"Layer index: reindexLayer(%1) failed: %2"_s.arg( layerId, err ), u"AI/Index"_s, Qgis::MessageLevel::Warning, false );
    emit reindexFinished( layerId, ok, err );
  }
}
