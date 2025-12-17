/***************************************************************************
                         qgsprocessingtaskqueuedialog.cpp
                         --------------------------------
    begin                : December 2024
    copyright            : (C) 2024 by Nassim Lanckmann
    email                : nassim dot lanckmann at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingtaskqueuedialog.h"

#include "qgsapplication.h"
#include "qgsprocessingalgorithm.h"
#include "qgsprocessingalgrunnertask.h"
#include "qgsprocessingcontext.h"
#include "qgsprocessingfeedback.h"
#include "qgsprocessingregistry.h"
#include "qgsprocessingtaskqueue.h"
#include "qgsproject.h"
#include "qgstaskmanager.h"

#include <QBrush>
#include <QColor>
#include <QMessageBox>

//
// QgsProcessingTaskQueueDialog
//

QgsProcessingTaskQueueDialog::QgsProcessingTaskQueueDialog( QWidget *parent, Qt::WindowFlags flags )
  : QDialog( parent, flags )
{
  setupUi( this );

  mQueue = QgsProcessingTaskQueue::instance();

  connect( mQueue, &QgsProcessingTaskQueue::queueChanged, this, &QgsProcessingTaskQueueDialog::refresh );
  connect( mTableWidget, &QTableWidget::itemSelectionChanged, this, &QgsProcessingTaskQueueDialog::updateButtons );
  connect( mRemoveButton, &QToolButton::clicked, this, &QgsProcessingTaskQueueDialog::removeSelected );
  connect( mMoveUpButton, &QToolButton::clicked, this, &QgsProcessingTaskQueueDialog::moveUp );
  connect( mMoveDownButton, &QToolButton::clicked, this, &QgsProcessingTaskQueueDialog::moveDown );
  connect( mClearButton, &QPushButton::clicked, this, &QgsProcessingTaskQueueDialog::clearQueue );
  connect( mExecuteButton, &QPushButton::clicked, this, &QgsProcessingTaskQueueDialog::executeQueue );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );

  refresh();
}

QgsProcessingTaskQueueDialog::~QgsProcessingTaskQueueDialog()
{
  if ( mCurrentTask )
  {
    mCurrentTask->cancel();
  }
}

void QgsProcessingTaskQueueDialog::refresh()
{
  mTableWidget->setRowCount( 0 );

  const QList<QgsProcessingQueuedTask> tasks = mQueue->tasks();
  for ( int i = 0; i < tasks.count(); ++i )
  {
    const QgsProcessingQueuedTask &task = tasks.at( i );
    mTableWidget->insertRow( i );

    QTableWidgetItem *indexItem = new QTableWidgetItem( QString::number( i + 1 ) );
    indexItem->setFlags( indexItem->flags() & ~Qt::ItemIsEditable );
    mTableWidget->setItem( i, 0, indexItem );

    const QgsProcessingAlgorithm *alg = QgsApplication::processingRegistry()->algorithmById( task.algorithmId() );
    QString algName = task.algorithmId();
    if ( alg )
    {
      algName = alg->displayName();
    }

    QTableWidgetItem *algItem = new QTableWidgetItem( algName );
    algItem->setFlags( algItem->flags() & ~Qt::ItemIsEditable );
    mTableWidget->setItem( i, 1, algItem );

    QTableWidgetItem *descItem = new QTableWidgetItem( task.description() );
    descItem->setFlags( descItem->flags() & ~Qt::ItemIsEditable );
    mTableWidget->setItem( i, 2, descItem );
  }

  mTableWidget->resizeColumnsToContents();
  updateButtons();
}

void QgsProcessingTaskQueueDialog::removeSelected()
{
  const QList<QTableWidgetItem *> selected = mTableWidget->selectedItems();
  if ( selected.isEmpty() )
    return;

  const int row = selected.at( 0 )->row();
  mQueue->removeTask( row );
}

void QgsProcessingTaskQueueDialog::moveUp()
{
  const QList<QTableWidgetItem *> selected = mTableWidget->selectedItems();
  if ( selected.isEmpty() )
    return;

  const int row = selected.at( 0 )->row();
  if ( mQueue->moveTaskUp( row ) )
  {
    mTableWidget->selectRow( row - 1 );
  }
}

void QgsProcessingTaskQueueDialog::moveDown()
{
  const QList<QTableWidgetItem *> selected = mTableWidget->selectedItems();
  if ( selected.isEmpty() )
    return;

  const int row = selected.at( 0 )->row();
  if ( mQueue->moveTaskDown( row ) )
  {
    mTableWidget->selectRow( row + 1 );
  }
}

void QgsProcessingTaskQueueDialog::clearQueue()
{
  if ( QMessageBox::question( this, tr( "Clear Queue" ), tr( "Are you sure you want to clear all tasks from the queue?" ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) == QMessageBox::Yes )
  {
    mQueue->clear();
  }
}

void QgsProcessingTaskQueueDialog::executeQueue()
{
  if ( mQueue->isEmpty() )
    return;

  mTasksToExecute = mQueue->tasks();
  mCurrentTaskIndex = 0;
  mTaskResults.clear();
  mTaskErrors.clear();

  executeNextTask();
}

void QgsProcessingTaskQueueDialog::executeNextTask()
{
  if ( mCurrentTaskIndex >= mTasksToExecute.count() )
  {
    onQueueExecutionComplete();
    return;
  }

  const QgsProcessingQueuedTask &queuedTask = mTasksToExecute.at( mCurrentTaskIndex );
  const QgsProcessingAlgorithm *alg = QgsApplication::processingRegistry()->algorithmById( queuedTask.algorithmId() );

  if ( !alg )
  {
    const QString errorMsg = tr( "Algorithm '%1' not found" ).arg( queuedTask.algorithmId() );
    mTaskErrors.append( errorMsg );
    markTaskFailed( mCurrentTaskIndex );
    mCurrentTaskIndex++;
    executeNextTask();
    return;
  }

  markTaskExecuting( mCurrentTaskIndex );

  std::unique_ptr<QgsProcessingAlgorithm> algInstance( alg->create() );
  if ( !algInstance )
  {
    const QString errorMsg = tr( "Failed to create algorithm instance for '%1'" ).arg( queuedTask.algorithmId() );
    mTaskErrors.append( errorMsg );
    markTaskFailed( mCurrentTaskIndex );
    mCurrentTaskIndex++;
    executeNextTask();
    return;
  }

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );

  auto feedback = std::make_unique<QgsProcessingFeedback>();

  QgsProcessingAlgRunnerTask *task = new QgsProcessingAlgRunnerTask( algInstance.get(), queuedTask.parameters(), *context, feedback.get() );

  connect( task, &QgsProcessingAlgRunnerTask::executed, this, &QgsProcessingTaskQueueDialog::onTaskComplete );

  mCurrentTask = task;
  QgsApplication::taskManager()->addTask( task );

  ( void ) algInstance.release();
  ( void ) context.release();
  ( void ) feedback.release();
}

void QgsProcessingTaskQueueDialog::onTaskComplete( bool success, const QVariantMap &results )
{
  if ( success )
  {
    mTaskResults.append( results );
    markTaskCompleted( mCurrentTaskIndex );
  }
  else
  {
    mTaskErrors.append( tr( "Algorithm execution failed" ) );
    markTaskFailed( mCurrentTaskIndex );
  }

  mCurrentTaskIndex++;
  executeNextTask();
}

void QgsProcessingTaskQueueDialog::onQueueExecutionComplete()
{
  const int successCount = mTaskResults.count();
  const int errorCount = mTaskErrors.count();
  const int totalCount = successCount + errorCount;

  if ( errorCount > 0 )
  {
    QString errorSummary;
    const int maxErrorsToShow = 5;
    if ( errorCount <= maxErrorsToShow )
    {
      errorSummary = mTaskErrors.join( QLatin1Char( '\n' ) );
    }
    else
    {
      errorSummary = mTaskErrors.mid( 0, maxErrorsToShow ).join( QLatin1Char( '\n' ) );
      errorSummary += tr( "\n... and %1 more errors" ).arg( errorCount - maxErrorsToShow );
    }

    QMessageBox::warning( this, tr( "Queue Execution Complete with Errors" ), tr( "%1 of %2 tasks completed successfully.\n\nErrors:\n%3" ).arg( successCount ).arg( totalCount ).arg( errorSummary ) );
  }
  else
  {
    QMessageBox::information( this, tr( "Queue Execution Complete" ), tr( "All %1 tasks completed successfully." ).arg( totalCount ) );
  }
}

void QgsProcessingTaskQueueDialog::markTaskExecuting( int index )
{
  if ( index < mTableWidget->rowCount() )
  {
    for ( int col = 0; col < mTableWidget->columnCount(); ++col )
    {
      QTableWidgetItem *item = mTableWidget->item( index, col );
      if ( item )
      {
        item->setBackground( QBrush( QColor( 255, 255, 200 ) ) );
      }
    }
  }
}

void QgsProcessingTaskQueueDialog::markTaskCompleted( int index )
{
  if ( index < mTableWidget->rowCount() )
  {
    for ( int col = 0; col < mTableWidget->columnCount(); ++col )
    {
      QTableWidgetItem *item = mTableWidget->item( index, col );
      if ( item )
      {
        item->setBackground( QBrush( QColor( 200, 255, 200 ) ) );
      }
    }
  }
}

void QgsProcessingTaskQueueDialog::markTaskFailed( int index )
{
  if ( index < mTableWidget->rowCount() )
  {
    for ( int col = 0; col < mTableWidget->columnCount(); ++col )
    {
      QTableWidgetItem *item = mTableWidget->item( index, col );
      if ( item )
      {
        item->setBackground( QBrush( QColor( 255, 200, 200 ) ) );
      }
    }
  }
}

void QgsProcessingTaskQueueDialog::updateButtons()
{
  const bool hasQueue = !mQueue->isEmpty();
  const bool hasSelection = !mTableWidget->selectedItems().isEmpty();

  mExecuteButton->setEnabled( hasQueue );
  mClearButton->setEnabled( hasQueue );
  mRemoveButton->setEnabled( hasSelection );
  mMoveUpButton->setEnabled( hasSelection );
  mMoveDownButton->setEnabled( hasSelection );
}

#include "moc_qgsprocessingtaskqueuedialog.cpp"
