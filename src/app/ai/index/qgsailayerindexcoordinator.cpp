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
#include <utility>

#include "qgsaiworkspaceindex.h"
#include "qgsapplication.h"
#include "qgsmaplayer.h"
#include "qgsmessagelog.h"
#include "qgsproject.h"
#include "qgstaskmanager.h"
#include "qgsvectorlayer.h"

#include <QPointer>
#include <QString>

#include "moc_qgsailayerindexcoordinator.cpp"

using namespace Qt::StringLiterals;

namespace
{
  struct LayerIndexResult
  {
      QString layerId;
      bool success = false;
      QString errorMessage;
  };

  class QgsAiLayerIndexTask final : public QgsTask
  {
    public:
      QgsAiLayerIndexTask( QgsAiWorkspaceIndex *index, QgsAiWorkspaceIndex::WorkspaceLayerSnapshot snapshot )
        : QgsTask( QObject::tr( "Index AI layers" ), QgsTask::CanCancel | QgsTask::CancelWithoutPrompt )
        , mIndex( index )
        , mSnapshot( std::move( snapshot ) )
      {}

      LayerIndexResult result() const { return mResult; }
      QString errorMessage() const { return mErrorMessage; }

    protected:
      bool run() override
      {
        if ( !mIndex )
        {
          mErrorMessage = QObject::tr( "Workspace index is unavailable." );
          mResult = { mSnapshot.scopedLayerId, false, mErrorMessage };
          return false;
        }

        if ( isCanceled() )
        {
          mIndex->closeDatabaseConnectionForCurrentThread();
          return false;
        }

        QString reindexError;
        const bool ok = mIndex->reindexLayerSnapshot( mSnapshot, &reindexError );
        mResult = { mSnapshot.scopedLayerId, ok, reindexError };
        if ( !ok && !mIndex->embeddingProviderAvailable() )
          mErrorMessage = reindexError;

        mIndex->closeDatabaseConnectionForCurrentThread();
        return ok && !isCanceled();
      }

    private:
      QPointer<QgsAiWorkspaceIndex> mIndex;
      QgsAiWorkspaceIndex::WorkspaceLayerSnapshot mSnapshot;
      LayerIndexResult mResult;
      QString mErrorMessage;
  };
} // namespace

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
  {
    if ( mRunningTask )
      mRunningTask->cancel();
    mDirtyLayers.clear();
    mUseBulkDebounce = false;
    disconnectProjectSignals();
  }
}

void QgsAiLayerIndexCoordinator::setDebounceMs( int ms )
{
  mDebounceMs = std::max( 0, ms );
}

void QgsAiLayerIndexCoordinator::setBulkDebounceMs( int ms )
{
  mBulkDebounceMs = std::max( 0, ms );
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

  mUseBulkDebounce = true;
  const QMap<QString, QgsMapLayer *> existing = mProject->mapLayers();
  for ( auto it = existing.constBegin(); it != existing.constEnd(); ++it )
  {
    if ( it.value() )
      mDirtyLayers.insert( it.value()->id() );
  }
  startDebounceTimer();
}

void QgsAiLayerIndexCoordinator::scheduleDirty( const QString &layerId )
{
  if ( layerId.isEmpty() )
    return;
  if ( !mIndex || !mIndex->embeddingProviderAvailable() )
    return;
  mDirtyLayers.insert( layerId );
  startDebounceTimer();
}

void QgsAiLayerIndexCoordinator::startDebounceTimer()
{
  const int ms = mUseBulkDebounce ? mBulkDebounceMs : mDebounceMs;
  mDebounceTimer.start( ms );
}

void QgsAiLayerIndexCoordinator::flushDirty()
{
  mUseBulkDebounce = false;

  if ( !mIndex || !mEnabled || !mIndex->embeddingProviderAvailable() )
  {
    mDirtyLayers.clear();
    return;
  }
  if ( mRunningTask && mRunningTask->isActive() )
    return;
  if ( mDirtyLayers.isEmpty() )
    return;

  const QString layerId = *mDirtyLayers.constBegin();
  mDirtyLayers.remove( layerId );

  emit reindexStarted( layerId );

  // Build one layer snapshot on the main thread (QgsMapLayer is not thread-safe).
  // Embedding and SQLite writes run in QgsAiLayerIndexTask on a worker thread.
  QgsAiWorkspaceIndex::WorkspaceLayerSnapshot snapshot;
  QString snapshotError;
  if ( !mIndex->createWorkspaceLayerSnapshotForLayer( layerId, snapshot, &snapshotError ) )
  {
    emit reindexFinished( layerId, false, snapshotError );
    QgsMessageLog::logMessage( u"Layer index: snapshot(%1) failed: %2"_s.arg( layerId, snapshotError ), u"AI/Index"_s, Qgis::MessageLevel::Warning, false );
    if ( mEnabled && !mDirtyLayers.isEmpty() && mIndex && mIndex->embeddingProviderAvailable() )
      mDebounceTimer.start( 0 );
    return;
  }

  QgsTaskManager *taskManager = QgsApplication::taskManager();
  if ( !taskManager )
  {
    QString err;
    const bool ok = mIndex->reindexLayerSnapshot( snapshot, &err );
    emit reindexFinished( layerId, ok, err );
    if ( !ok )
      QgsMessageLog::logMessage( u"Layer index: reindexLayer(%1) failed: %2"_s.arg( layerId, err ), u"AI/Index"_s, Qgis::MessageLevel::Warning, false );

    if ( mEnabled && !mDirtyLayers.isEmpty() && mIndex && mIndex->embeddingProviderAvailable() )
      mDebounceTimer.start( 0 );
    return;
  }

  QgsAiLayerIndexTask *task = new QgsAiLayerIndexTask( mIndex, std::move( snapshot ) );
  mRunningTask = task;

  auto finishTask = [this, task]( bool terminated ) {
    const LayerIndexResult result = task->result();
    if ( !result.layerId.isEmpty() )
    {
      if ( !result.success )
        QgsMessageLog::logMessage( u"Layer index: reindexLayer(%1) failed: %2"_s.arg( result.layerId, result.errorMessage ), u"AI/Index"_s, Qgis::MessageLevel::Warning, false );
      emit reindexFinished( result.layerId, result.success, result.errorMessage );
    }

    if ( terminated && !task->errorMessage().isEmpty() )
      QgsMessageLog::logMessage( u"Layer index background task failed: %1"_s.arg( task->errorMessage() ), u"AI/Index"_s, Qgis::MessageLevel::Warning, false );

    if ( mRunningTask == task )
      mRunningTask = nullptr;

    if ( mEnabled && !mDirtyLayers.isEmpty() && mIndex && mIndex->embeddingProviderAvailable() )
      mDebounceTimer.start( 0 );
  };

  connect( task, &QgsTask::taskCompleted, this, [finishTask]() { finishTask( false ); } );
  connect( task, &QgsTask::taskTerminated, this, [finishTask]() { finishTask( true ); } );
  taskManager->addTask( task, 0 );
}
