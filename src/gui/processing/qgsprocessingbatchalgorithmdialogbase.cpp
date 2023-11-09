/***************************************************************************
                             qgsprocessingbatchalgorithmdialogbase.cpp
                             ------------------------------------
    Date                 : March 2022
    Copyright            : (C) 2022 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingbatchalgorithmdialogbase.h"
#include "qgsprocessingbatch.h"
#include "qgsproxyprogresstask.h"
#include "qgsprocessingalgorithm.h"
#include "qgsjsonutils.h"
#include "qgsprocessingalgrunnertask.h"
#include "qgsapplication.h"
#include <nlohmann/json.hpp>

///@cond NOT_STABLE
QgsProcessingBatchAlgorithmDialogBase::QgsProcessingBatchAlgorithmDialogBase( QWidget *parent, Qt::WindowFlags flags )
  : QgsProcessingAlgorithmDialogBase( parent, flags, QgsProcessingAlgorithmDialogBase::DialogMode::Batch )
{
  mButtonRunSingle = new QPushButton( tr( "Run as Single Process…" ) );
  connect( mButtonRunSingle, &QPushButton::clicked, this, &QgsProcessingBatchAlgorithmDialogBase::runAsSingle );
  buttonBox()->addButton( mButtonRunSingle, QDialogButtonBox::ResetRole ); // reset role to ensure left alignment

  connect( QgsApplication::taskManager(), &QgsTaskManager::taskTriggered, this, &QgsProcessingBatchAlgorithmDialogBase::taskTriggered );

  updateRunButtonVisibility();
}

QgsProcessingBatchAlgorithmDialogBase::~QgsProcessingBatchAlgorithmDialogBase() = default;

void QgsProcessingBatchAlgorithmDialogBase::resetAdditionalGui()
{
  mButtonRunSingle->setEnabled( true );
}

void QgsProcessingBatchAlgorithmDialogBase::blockAdditionalControlsWhileRunning()
{
  mButtonRunSingle->setEnabled( false );
}

void QgsProcessingBatchAlgorithmDialogBase::execute( const QList<QVariantMap> &parameters )
{
  mQueuedParameters = parameters;
  mCurrentStep = 0;
  mTotalSteps = mQueuedParameters.size();
  mResults.clear();
  mErrors.clear();

  mFeedback.reset( createFeedback() );

  mBatchFeedback = std::make_unique< QgsProcessingBatchFeedback >( mTotalSteps, mFeedback.get() );

  mProxyTask = new QgsProxyProgressTask( tr( "Batch Processing - %1" ).arg( algorithm()->displayName() ), true );
  connect( mProxyTask, &QgsProxyProgressTask::canceled, mBatchFeedback.get(), &QgsFeedback::cancel );
  connect( mFeedback.get(), &QgsFeedback::progressChanged, mProxyTask, &QgsProxyProgressTask::setProxyProgress );
  QgsApplication::taskManager()->addTask( mProxyTask );

  blockControlsWhileRunning();
  setExecutedAnyResult( true );
  cancelButton()->setEnabled( true );

  // Make sure the Log tab is visible before executing the algorithm
  showLog();
  repaint();

  mTotalTimer.restart();
  if ( mTotalSteps > 0 )
    executeNext();
}

bool QgsProcessingBatchAlgorithmDialogBase::isFinalized()
{
  return mQueuedParameters.empty();
}

void QgsProcessingBatchAlgorithmDialogBase::executeNext()
{
  if ( mQueuedParameters.empty() || mFeedback->isCanceled() )
  {
    allTasksComplete( false );
    return;
  }

  mBatchFeedback->setCurrentStep( mCurrentStep++ );
  setProgressText( QStringLiteral( "\n" ) + tr( "Processing algorithm %1/%2…" ).arg( mCurrentStep ).arg( mTotalSteps ) );
  setInfo( tr( "<b>Algorithm %1 starting&hellip;</b>" ).arg( algorithm()->displayName() ), false, false );

  pushInfo( tr( "Input parameters:" ) );

  // important - we create a new context for each iteration
  // this avoids holding onto resources and layers from earlier iterations,
  // and allows batch processing of many more items then is possible
  // if we hold on to these layers
  mTaskContext.reset( createContext( mBatchFeedback.get() ) );

  const QVariantMap paramsJson = algorithm()->asMap( mQueuedParameters.constFirst(), *mTaskContext ).value( QStringLiteral( "inputs" ) ).toMap();
  pushCommandInfo( QString::fromStdString( QgsJsonUtils::jsonFromVariant( paramsJson ).dump() ) );
  pushInfo( QString() );

  mCurrentParameters = algorithm()->preprocessParameters( mQueuedParameters.constFirst() );
  mQueuedParameters.pop_front();

  mCurrentStepTimer.restart();
  if ( !( algorithm()->flags() & QgsProcessingAlgorithm::FlagNoThreading ) )
  {
    QgsProcessingAlgRunnerTask *task = new QgsProcessingAlgRunnerTask( algorithm(), mCurrentParameters, *mTaskContext, mBatchFeedback.get(), QgsTask::CanCancel | QgsTask::Hidden );
    if ( task->algorithmCanceled() )
      onTaskComplete( false, {} );
    else
    {
      setCurrentTask( task );
    }
  }
  else
  {
    // have to execute in main thread, no tasks allowed
    bool ok = false;
    const QVariantMap results = algorithm()->run( mCurrentParameters, *mTaskContext, mBatchFeedback.get(), &ok );
    onTaskComplete( ok, results );
  }
}

void QgsProcessingBatchAlgorithmDialogBase::algExecuted( bool successful, const QVariantMap &results )
{
  // parent class cleanup first!
  QgsProcessingAlgorithmDialogBase::algExecuted( successful, results );
  onTaskComplete( successful, results );
}

void QgsProcessingBatchAlgorithmDialogBase::onTaskComplete( bool ok, const QVariantMap &results )
{
  if ( ok )
  {
    setInfo( tr( "Algorithm %1 correctly executed…" ).arg( algorithm()->displayName() ), false, false );
    pushInfo( tr( "Execution completed in %1 seconds" ).arg( mCurrentStepTimer.elapsed() / 1000.0, 2 ) );
    pushInfo( tr( "Results:" ) );

    pushCommandInfo( QString::fromStdString( QgsJsonUtils::jsonFromVariant( results ).dump() ) );
    pushInfo( QString() );

    mResults.append( QVariantMap(
    {
      { QStringLiteral( "parameters" ), mCurrentParameters },
      { QStringLiteral( "results" ), results }
    } ) );

    handleAlgorithmResults( algorithm(), *mTaskContext, mBatchFeedback.get(), mCurrentParameters );
    executeNext();
  }
  else if ( mBatchFeedback->isCanceled() )
  {
    setInfo( tr( "Algorithm %1 canceled…" ).arg( algorithm()->displayName() ), false, false );
    pushInfo( tr( "Execution canceled after %1 seconds" ).arg( mCurrentStepTimer.elapsed() / 1000.0, 2 ) );
    allTasksComplete( true );
  }
  else
  {
    const QStringList taskErrors = mBatchFeedback->popErrors();
    setInfo( tr( "Algorithm %1 failed…" ).arg( algorithm()->displayName() ), false, false );
    reportError( tr( "Execution failed after %1 seconds" ).arg( mCurrentStepTimer.elapsed() / 1000.0, 2 ), false );

    mErrors.append( QVariantMap(
    {
      { QStringLiteral( "parameters" ), mCurrentParameters },
      { QStringLiteral( "errors" ), taskErrors }
    } ) );
    executeNext();
  }
}

void QgsProcessingBatchAlgorithmDialogBase::taskTriggered( QgsTask *task )
{
  if ( task == mProxyTask )
  {
    show();
    raise();
    setWindowState( ( windowState() & ~Qt::WindowMinimized ) | Qt::WindowActive );
    activateWindow();
    showLog();
  }
}

void QgsProcessingBatchAlgorithmDialogBase::allTasksComplete( bool canceled )
{
  mBatchFeedback.reset();
  mFeedback.reset();
  mTaskContext.reset();
  mQueuedParameters.clear();
  if ( mProxyTask )
  {
    mProxyTask->finalize( true );
    mProxyTask = nullptr;
  }

  if ( !canceled )
  {
    pushInfo( tr( "Batch execution completed in %1 seconds" ).arg( mTotalTimer.elapsed() / 1000.0, 2 ) );
    if ( !mErrors.empty() )
    {
      reportError( tr( "%1 executions failed. See log for further details." ).arg( mErrors.size() ), true );
    }

    for ( int i = 0; i < mResults.size(); ++i )
    {
      loadHtmlResults( mResults.at( i ).value( QStringLiteral( "results" ) ).toMap(), i );
    }

    createSummaryTable( mResults, mErrors );
  }

  resetGui();
  cancelButton()->setEnabled( false );
}


///@endcond
