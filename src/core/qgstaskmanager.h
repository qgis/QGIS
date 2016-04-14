/***************************************************************************
                          qgstaskmanager.h
                          ----------------
    begin                : April 2016
    copyright            : (C) 2016 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTASKMANAGER_H
#define QGSTASKMANAGER_H

#include <QObject>
#include <QMap>
#include <QAbstractItemModel>

/** \ingroup core
 * \class QgsTask
 * \brief Interface class for long running background tasks which will be handled by a QgsTaskManager
 * \note Added in version 2.16
 */
class CORE_EXPORT QgsTask : public QObject
{
    Q_OBJECT

  public:

    //! Status of tasks
    enum TaskStatus
    {
      Running, /*!< Task is currently running */
      Complete, /*!< Task successfully completed */
      Terminated, /*!< Task was terminated or errored */
      // Paused,
      // Queued,
    };

    /** Constructor for QgsTask.
     * @param description text description of task
     */
    QgsTask( const QString& description = QString() );

    //! Will be called when task has been terminated, either through
    //! user interaction or other reason (eg application exit)
    //! @note derived classes must ensure they call the base method
    virtual void terminate()
    {
      stopped();
    }

    //! Returns true if the task is active, ie it is not complete and has
    //! not been terminated.
    bool isActive() const { return mStatus == Running; }

    //! Returns the current task status.
    TaskStatus status() const { return mStatus; }

    //! Returns the task's description.
    QString description() const { return mDescription; }

    //! Returns the task's progress (between 0.0 and 100.0)
    double progress() const { return mProgress; }

  signals:

    //! Will be emitted by task when its progress changes
    //! @param progress percent of progress, from 0.0 - 100.0
    //! @note derived classes should not emit this signal directly, instead they should call
    //! setProgress()
    void progressChanged( double progress );

    //! Will be emitted by task when its status changes
    //! @param status new task status
    //! @note derived classes should not emit this signal directly, instead they should call
    //! completed() or stopped()
    void statusChanged( int status );

    //! Will be emitted by task to indicate its completion.
    //! @note derived classes should not emit this signal directly, instead they should call
    //! completed()
    void taskCompleted();

    //! Will be emitted by task if it has terminated for any reason
    //! other then completion.
    //! @note derived classes should not emit this signal directly, instead they should call
    //! stopped()//!
    void taskStopped();

  protected:

    //! Sets the task's current progress. Should be called whenever the
    //! task wants to update it's progress. Calling will automatically emit the progressChanged
    //! signal.
    //! @param progress percent of progress, from 0.0 - 100.0
    void setProgress( double progress );

    //! Sets the task as completed. Should be called when the task is complete.
    //! Calling will automatically emit the statusChanged and taskCompleted signals.
    void completed();

    //! Sets the task as stopped. Should be called whenever the task ends for any
    //! reason other than successful completion.
    //! Calling will automatically emit the statusChanged and taskStopped signals.
    void stopped();

  private:

    QString mDescription;
    TaskStatus mStatus;
    double mProgress;

};

/** \ingroup core
 * \class QgsTaskManager
 * \brief Task manager for managing a set of long-running QgsTask tasks. This class can be created directly,
 * or accessed via a global instance.
 * \note Added in version 2.16
 */
class CORE_EXPORT QgsTaskManager : public QObject
{
    Q_OBJECT

  public:

    /** Returns the global task manager instance pointer, creating the object on the first call.
     */
    static QgsTaskManager * instance();

    /** Constructor for QgsTaskManager.
     * @param parent parent QObject
     */
    QgsTaskManager( QObject* parent = nullptr );

    virtual ~QgsTaskManager();

    /** Adds a task to the manager. Ownership of the task is transferred
     * to the manager.
     * @param task task to add
     * @returns unique task ID
     */
    long addTask( QgsTask* task );

    /** Deletes the specified task, first terminating it if it is currently
     * running.
     * @param id task ID
     * @returns true if task was found and deleted
     */
    bool deleteTask( long id );

    /** Deletes the specified task, first terminating it if it is currently
     * running.
     * @param task task to delete
     * @returns true if task was contained in manager and deleted
     */
    bool deleteTask( QgsTask* task );

    /** Returns the task with matching ID.
     * @param id task ID
     * @returns task if found, or nullptr
     */
    QgsTask* task( long id ) const;

    /** Returns all tasks tracked by the manager.
     */
    QList<QgsTask*> tasks() const;

    //! Returns the number of tasks tracked by the manager.
    int count() const { return mTasks.count(); }

    /** Returns the unique task ID corresponding to a task managed by the class.
     * @param task task to find
     * @returns task ID, or -1 if task not found
     */
    long taskId( QgsTask* task ) const;

  signals:

    //! Will be emitted when a task reports a progress change
    //! @param taskId ID of task
    //! @param progress percent of progress, from 0.0 - 100.0
    void progressChanged( long taskId, double progress );

    //! Will be emitted when a task reports a status change
    //! @param taskId ID of task
    //! @param status new task status
    void statusChanged( long taskId, int status );

    //! Emitted when a new task has been added to the manager
    //! @param taskId ID of task
    void taskAdded( long taskId );

    //! Emitted when a task is about to be deleted
    //! @param taskId ID of task
    void taskAboutToBeDeleted( long taskId );

  private slots:

    void taskProgressChanged( double progress );
    void taskStatusChanged( int status );

  private:

    static QgsTaskManager *sInstance;
    QMap< long, QgsTask* > mTasks;

    //! Tracks the next unique task ID
    long mNextTaskId;

    bool cleanupAndDeleteTask( QgsTask* task );

};

#endif //QGSTASKMANAGER_H
