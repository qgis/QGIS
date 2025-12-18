/***************************************************************************
                         qgsprocessingtaskqueue.h
                         ------------------------
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

#ifndef QGSPROCESSINGTASKQUEUE_H
#define QGSPROCESSINGTASKQUEUE_H

#include "qgis_core.h"
#include "qgis_sip.h"

#include <QList>
#include <QObject>
#include <QString>
#include <QVariantMap>

class QgsProcessingAlgorithm;
class QgsProcessingContext;
class QgsProcessingFeedback;
class QgsProcessingAlgRunnerTask;

/**
 * \class QgsProcessingQueuedTask
 * \ingroup core
 * \brief Represents a single queued processing task.
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsProcessingQueuedTask
{
  public:

    /**
     * Constructor for QgsProcessingQueuedTask.
     * \param algorithmId ID of the algorithm to execute
     * \param parameters parameters to pass to the algorithm
     * \param description optional description for the task
     */
    QgsProcessingQueuedTask( const QString &algorithmId = QString(),
                             const QVariantMap &parameters = QVariantMap(),
                             const QString &description = QString() );

    /**
     * Returns the algorithm ID.
     */
    QString algorithmId() const { return mAlgorithmId; }

    /**
     * Returns the algorithm parameters.
     */
    QVariantMap parameters() const { return mParameters; }

    /**
     * Returns the task description.
     */
    QString description() const { return mDescription; }

  private:
    QString mAlgorithmId;
    QVariantMap mParameters;
    QString mDescription;
};

/**
 * \class QgsProcessingTaskQueue
 * \ingroup core
 * \brief Singleton manager for a queue of processing tasks to be executed sequentially.
 *
 * This allows users to queue up different processing algorithms and execute them
 * one after another, avoiding concurrent execution that could overload system resources.
 *
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsProcessingTaskQueue : public QObject SIP_NODEFAULTCTORS
{
    Q_OBJECT

  public:

    /**
     * Returns the singleton instance of the task queue.
     */
    static QgsProcessingTaskQueue *instance();

    /**
     * Adds a task to the queue.
     * \param algorithmId ID of the algorithm to execute
     * \param parameters parameters to pass to the algorithm
     * \param description optional description for the task
     */
    void addTask( const QString &algorithmId, const QVariantMap &parameters, const QString &description = QString() );

    /**
     * Removes a task at the specified index.
     * \param index index of the task to remove
     * \returns TRUE if the task was successfully removed
     */
    bool removeTask( int index );

    /**
     * Moves a task up in the queue.
     * \param index index of the task to move
     * \returns TRUE if the task was successfully moved
     */
    bool moveTaskUp( int index );

    /**
     * Moves a task down in the queue.
     * \param index index of the task to move
     * \returns TRUE if the task was successfully moved
     */
    bool moveTaskDown( int index );

    /**
     * Clears all tasks from the queue.
     */
    void clear();

    /**
     * Returns all tasks in the queue.
     */
    QList<QgsProcessingQueuedTask> tasks() const { return mQueue; }

    /**
     * Returns the number of tasks in the queue.
     */
    int count() const { return mQueue.count(); }

    /**
     * Returns TRUE if the queue is empty.
     */
    bool isEmpty() const { return mQueue.isEmpty(); }

  signals:

    /**
     * Emitted when the queue changes (tasks added, removed, or reordered).
     */
    void queueChanged();

  private:

    QgsProcessingTaskQueue() SIP_FORCE;
    ~QgsProcessingTaskQueue() override SIP_FORCE;

    QgsProcessingTaskQueue( const QgsProcessingTaskQueue &other ) = delete;
    QgsProcessingTaskQueue &operator=( const QgsProcessingTaskQueue &other ) = delete;

    QList<QgsProcessingQueuedTask> mQueue;

    static QgsProcessingTaskQueue *sInstance;

    friend class QgsApplication;
};

#endif // QGSPROCESSINGTASKQUEUE_H
