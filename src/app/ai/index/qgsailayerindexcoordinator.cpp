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

#include "qgsaiworkspaceindex.h"
#include "qgsmaplayer.h"
#include "qgsmessagelog.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"

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
  // Wire signals on already-loaded layers too so we react to subsequent edits.
  const QMap<QString, QgsMapLayer *> existing = mProject->mapLayers();
  for ( auto it = existing.constBegin(); it != existing.constEnd(); ++it )
    connectLayerSignals( it.value() );
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
  if ( QgsVectorLayer *v = qobject_cast<QgsVectorLayer *>( layer ) )
    connect( v, &QgsVectorLayer::editingStopped, this, &QgsAiLayerIndexCoordinator::onEditingStopped, Qt::UniqueConnection );
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

void QgsAiLayerIndexCoordinator::onEditingStopped()
{
  QgsVectorLayer *v = qobject_cast<QgsVectorLayer *>( sender() );
  if ( !v )
    return;
  scheduleDirty( v->id() );
}

void QgsAiLayerIndexCoordinator::scheduleDirty( const QString &layerId )
{
  if ( layerId.isEmpty() )
    return;
  mDirtyLayers.insert( layerId );
  mDebounceTimer.start( mDebounceMs );
}

void QgsAiLayerIndexCoordinator::flushDirty()
{
  if ( !mIndex || !mEnabled )
  {
    mDirtyLayers.clear();
    return;
  }

  const QSet<QString> toProcess = mDirtyLayers;
  mDirtyLayers.clear();

  for ( const QString &layerId : toProcess )
  {
    emit reindexStarted( layerId );
    QString err;
    const bool ok = mIndex->reindexLayer( layerId, &err );
    if ( !ok )
      QgsMessageLog::logMessage( u"Layer index: reindexLayer(%1) failed: %2"_s.arg( layerId, err ), u"AI/Index"_s, Qgis::MessageLevel::Warning, false );
    emit reindexFinished( layerId, ok, err );
  }
}
