/***************************************************************************
    qgsaiindexingscheduler.cpp
    --------------------------
    begin                : June 2026
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

#include "qgsaiindexingscheduler.h"

#include "qgsaiworkspaceindex.h"
#include "qgsapplication.h"
#include "qgsmessagelog.h"
#include "qgstaskmanager.h"

#include <algorithm>

using namespace Qt::StringLiterals;

namespace
{
  class QgsAiWorkspaceIndexTask final : public QgsTask
  {
    public:
      explicit QgsAiWorkspaceIndexTask( QgsAiWorkspaceIndex *index )
        : QgsTask( QObject::tr( "Index AI workspace" ), QgsTask::CanCancel | QgsTask::CancelWithoutPrompt )
        , mIndex( index )
      {}

      QString errorMessage() const { return mErrorMessage; }

    protected:
      bool run() override
      {
        if ( !mIndex )
        {
          mErrorMessage = QObject::tr( "Workspace index is unavailable." );
          return false;
        }
        if ( isCanceled() )
          return false;

        // Relay the index's progress to the task so the task manager shows real
        // per-file progress. reindex() runs synchronously on this worker thread, so
        // the progress signal is delivered here; a direct connection is safe.
        const QMetaObject::Connection conn = connect( mIndex.data(), &QgsAiWorkspaceIndex::progress, this, [this]( int current, int total, const QString & ) {
          if ( total > 0 )
            setProgress( 100.0 * static_cast<double>( current ) / static_cast<double>( total ) );
        }, Qt::DirectConnection );

        QString error;
        const bool ok = mIndex->reindex( QgsAiWorkspaceIndex::DEFAULT_MAX_FILES, &error );
        disconnect( conn );
        if ( !ok )
          mErrorMessage = error;
        return ok && !isCanceled();
      }

    private:
      QPointer<QgsAiWorkspaceIndex> mIndex;
      QString mErrorMessage;
  };
} // namespace

QgsAiIndexingScheduler::QgsAiIndexingScheduler( QgsAiWorkspaceIndex *index, QObject *parent )
  : QObject( parent )
  , mIndex( index )
{
  mDebounceTimer.setSingleShot( true );
  connect( &mDebounceTimer, &QTimer::timeout, this, &QgsAiIndexingScheduler::startWorkspaceIndexing );
}

void QgsAiIndexingScheduler::setAutomaticEnabled( bool enabled )
{
  mAutomaticEnabled = enabled;
  if ( !mAutomaticEnabled )
  {
    mDebounceTimer.stop();
    cancel();
  }
}

void QgsAiIndexingScheduler::scheduleStartupIndexing( int delayMs )
{
  scheduleWorkspaceIndexing( delayMs );
}

void QgsAiIndexingScheduler::scheduleWorkspaceIndexing( int delayMs )
{
  if ( !mAutomaticEnabled || !mIndex || !mIndex->embeddingProviderAvailable() )
    return;
  mDebounceTimer.start( std::max( 0, delayMs ) );
}

void QgsAiIndexingScheduler::cancel()
{
  if ( mRunningTask )
    mRunningTask->cancel();
}

void QgsAiIndexingScheduler::startWorkspaceIndexing()
{
  if ( !mAutomaticEnabled || !mIndex || !mIndex->embeddingProviderAvailable() )
    return;
  if ( mRunningTask && mRunningTask->isActive() )
    return;

  QgsTaskManager *manager = QgsApplication::taskManager();
  if ( !manager )
  {
    QString error;
    const bool ok = mIndex->reindex( QgsAiWorkspaceIndex::DEFAULT_MAX_FILES, &error );
    if ( !ok && !error.isEmpty() )
      QgsMessageLog::logMessage( u"AI workspace indexing failed: %1"_s.arg( error ), u"AI/Index"_s, Qgis::MessageLevel::Warning, false );
    return;
  }

  QgsAiWorkspaceIndexTask *task = new QgsAiWorkspaceIndexTask( mIndex );
  mRunningTask = task;
  connect( task, &QgsTask::taskCompleted, this, [this, task]() {
    QgsMessageLog::logMessage( u"AI workspace index updated in background."_s, u"AI/Index"_s, Qgis::MessageLevel::Info, false );
    if ( mRunningTask == task )
      mRunningTask = nullptr;
  } );
  connect( task, &QgsTask::taskTerminated, this, [this, task]() {
    const QString error = task->errorMessage();
    if ( !error.isEmpty() )
      QgsMessageLog::logMessage( u"AI workspace background indexing failed: %1"_s.arg( error ), u"AI/Index"_s, Qgis::MessageLevel::Warning, false );
    if ( mRunningTask == task )
      mRunningTask = nullptr;
  } );
  // Lowest priority: background indexing must never take precedence over
  // user-initiated tasks (in QGIS higher priority numbers win, so 0 is lowest).
  manager->addTask( task, 0 );
}
