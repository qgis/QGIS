/***************************************************************************
                         qgsprocessingtaskqueuedialog.h
                         ------------------------------
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

#ifndef QGSPROCESSINGTASKQUEUEDIALOG_H
#define QGSPROCESSINGTASKQUEUEDIALOG_H

#include "ui_qgsprocessingtaskqueuedialogbase.h"

#include "qgis_gui.h"
#include "qgis_sip.h"

#include <QDialog>
#include <QPointer>

class QgsProcessingTaskQueue;
class QgsProcessingAlgRunnerTask;
class QgsProcessingContext;
class QgsProcessingFeedback;

/**
 * \ingroup gui
 * \brief Dialog for managing the processing task queue.
 * \note This is not considered stable API and may change in future QGIS versions.
 * \since QGIS 4.0
 */
class GUI_EXPORT QgsProcessingTaskQueueDialog : public QDialog, private Ui::QgsProcessingTaskQueueDialogBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsProcessingTaskQueueDialog.
     * \param parent parent widget
     * \param flags window flags
     */
    QgsProcessingTaskQueueDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags flags = Qt::WindowFlags() );

    /**
     * Destructor for QgsProcessingTaskQueueDialog.
     */
    ~QgsProcessingTaskQueueDialog() override;

#ifndef SIP_RUN
  private slots:

    void refresh();
    void removeSelected();
    void moveUp();
    void moveDown();
    void clearQueue();
    void executeQueue();
    void updateButtons();

  private:
    void executeNextTask();
    void onTaskComplete( bool success, const QVariantMap &results );
    void onQueueExecutionComplete();
    void markTaskExecuting( int index );
    void markTaskCompleted( int index );
    void markTaskFailed( int index );

    QgsProcessingTaskQueue *mQueue = nullptr;
    QList<QgsProcessingQueuedTask> mTasksToExecute;
    int mCurrentTaskIndex = 0;
    QList<QVariantMap> mTaskResults;
    QStringList mTaskErrors;
    QPointer<QgsProcessingAlgRunnerTask> mCurrentTask;
#endif
};

#endif // QGSPROCESSINGTASKQUEUEDIALOG_H
