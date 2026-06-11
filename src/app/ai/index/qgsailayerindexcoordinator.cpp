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
      QgsAiLayerIndexTask( QgsAiWorkspaceIndex *index, const QList<QgsAiWorkspaceIndex::WorkspaceLayerSnapshot> &snapshots )
        : QgsTask( QObject::tr( "Index AI layers" ), QgsTask::CanCancel | QgsTask::CancelWithoutPrompt )
        , mIndex( index )
        , mSnapshots( snapshots )
      {}

      QList<LayerIndexResult> results() const { return mResults; }
      QStringList remainingLayerIds() const { return mRemainingLayerIds; }
      QString errorMessage() const { return mErrorMessage; }

    protected:
      bool run() override
      {
        if ( !mIndex )
        {
          mErrorMessage = QObject::tr( "Workspace index is unavailable." );
          return false;
        }

        for ( int i = 0; i < mSnapshots.size(); ++i )
        {
          if ( isCanceled() )
          {
            appendRemainingLayerIds( i );
            mIndex->closeDatabaseConnectionForCurrentThread();
            return false;
          }

          const QgsAiWorkspaceIndex::WorkspaceLayerSnapshot &snapshot = mSnapshots.at( i );
          const QString layerId = snapshot.scopedLayerId;
          QString error;
          const bool ok = mIndex->reindexLayerSnapshot( snapshot, &error );
          mResults.append( { layerId, ok, error } );
          if ( !ok && !mIndex->embeddingProviderAvailable() )
          {
            appendRemainingLayerIds( i + 1 );
            mErrorMessage = error;
            mIndex->closeDatabaseConnectionForCurrentThread();
            return false;
          }
        }

        mIndex->closeDatabaseConnectionForCurrentThread();
        return !isCanceled();
      }

    private:
      void appendRemainingLayerIds( int startIndex )
      {
        for ( int i = startIndex; i < mSnapshots.size(); ++i )
        {
          const QString layerId = mSnapshots.at( i ).scopedLayerId;
          if ( !layerId.isEmpty() )
            mRemainingLayerIds.append( layerId );
        }
      }

      QPointer<QgsAiWorkspaceIndex> mIndex;
      QList<QgsAiWorkspaceIndex::WorkspaceLayerSnapshot> mSnapshots;
      QList<LayerIndexResult> mResults;
      QStringList mRemainingLayerIds;
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
    disconnectProjectSignals();
  }
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
  if ( mRunningTask && mRunningTask->isActive() )
    return;

  const QList<QString> toProcess( mDirtyLayers.constBegin(), mDirtyLayers.constEnd() );
  mDirtyLayers.clear();

  QList<QgsAiWorkspaceIndex::WorkspaceLayerSnapshot> snapshots;
  snapshots.reserve( toProcess.size() );
  for ( const QString &layerId : toProcess )
  {
    QString err;
    QgsAiWorkspaceIndex::WorkspaceLayerSnapshot snapshot;
    if ( !mIndex->createWorkspaceLayerSnapshotForLayer( layerId, snapshot, &err ) )
    {
      emit reindexStarted( layerId );
      emit reindexFinished( layerId, false, err );
      QgsMessageLog::logMessage( u"Layer index: snapshot(%1) failed: %2"_s.arg( layerId, err ), u"AI/Index"_s, Qgis::MessageLevel::Warning, false );
      continue;
    }
    snapshots.append( snapshot );
  }

  if ( snapshots.isEmpty() )
    return;

  QgsTaskManager *taskManager = QgsApplication::taskManager();
  if ( !taskManager )
  {
    for ( int i = 0; i < snapshots.size(); ++i )
    {
      const QString layerId = snapshots.at( i ).scopedLayerId;
      emit reindexStarted( layerId );
      QString err;
      const bool ok = mIndex->reindexLayerSnapshot( snapshots.at( i ), &err );
      emit reindexFinished( layerId, ok, err );
      if ( !ok && !mIndex->embeddingProviderAvailable() )
      {
        for ( int j = i + 1; j < snapshots.size(); ++j )
          mDirtyLayers.insert( snapshots.at( j ).scopedLayerId );
        break;
      }
      if ( !ok )
        QgsMessageLog::logMessage( u"Layer index: reindexLayer(%1) failed: %2"_s.arg( layerId, err ), u"AI/Index"_s, Qgis::MessageLevel::Warning, false );
    }
    return;
  }

  for ( const QgsAiWorkspaceIndex::WorkspaceLayerSnapshot &snapshot : std::as_const( snapshots ) )
    emit reindexStarted( snapshot.scopedLayerId );

  QgsAiLayerIndexTask *task = new QgsAiLayerIndexTask( mIndex, snapshots );
  mRunningTask = task;

  auto finishTask = [this, task]( bool terminated ) {
    for ( const LayerIndexResult &result : task->results() )
    {
      if ( !result.success )
        QgsMessageLog::logMessage( u"Layer index: reindexLayer(%1) failed: %2"_s.arg( result.layerId, result.errorMessage ), u"AI/Index"_s, Qgis::MessageLevel::Warning, false );
      emit reindexFinished( result.layerId, result.success, result.errorMessage );
    }

    if ( mEnabled )
    {
      for ( const QString &layerId : task->remainingLayerIds() )
        mDirtyLayers.insert( layerId );
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
