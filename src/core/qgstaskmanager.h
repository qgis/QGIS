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
#include <QFuture>

/** \ingroup core
 * \class QgsTask
 * \brief Interface class for long running background tasks. Tasks can be controlled directly,
 * or added to a QgsTaskManager for automatic management.
 * \note Added in version 2.16
 */
class CORE_EXPORT QgsTask : public QObject
{
    Q_OBJECT

  public:

    //! Status of tasks
    enum TaskStatus
    {
      Queued, /*!< Task is queued and has not begun */
      OnHold, /*!< Task is queued but on hold and will not be started */
      Running, /*!< Task is currently running */
      Complete, /*!< Task successfully completed */
      Terminated, /*!< Task was terminated or errored */
    };

    //! Task flags
    enum Flag
    {
      CanCancel = 1 << 1, //!< Task can be cancelled
      CanReportProgress = 1 << 2, //!< Task will report its progress
      AllFlags = CanCancel | CanReportProgress, //!< Task supports all flags
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    /** Constructor for QgsTask.
     * @param description text description of task
     * @param flags task flags
     */
    QgsTask( const QString& description = QString(), const Flags& flags = AllFlags );

    //! Returns the flags associated with the task.
    Flags flags() const { return mFlags; }

    //! Returns true if the task can be cancelled.
    bool canCancel() const { return mFlags & CanCancel; }

    //! Returns true if the task is active, ie it is not complete and has
    //! not been cancelled.
    bool isActive() const { return mStatus == Running; }

    //! Returns the current task status.
    TaskStatus status() const { return mStatus; }

    //! Returns the task's description.
    QString description() const { return mDescription; }

    //! Returns the task's progress (between 0.0 and 100.0)
    double progress() const { return mProgress; }

  public slots:

    //! Starts the task.
    void start();

    //! Notifies the task that it should terminate.
    //! @see isCancelled()
    void cancel();

    //! Called when the task is placed on hold. If the task in not queued
    //! (ie it is running or has finished) then calling this has no effect.
    //! @see unhold()
    void hold();

    //! Called when the task should be unheld and re-added to the queue. If the
    //! task in not currently being held then calling this has no effect.
    //! @see unhold()
    void unhold();

    //! Sets the task's current progress. If task reports the CanReportProgress flag then
    //! the derived class should call this method whenever the task wants to update its
    //! progress. Calling will automatically emit the progressChanged signal.
    //! @param progress percent of progress, from 0.0 - 100.0
    void setProgress( double progress );

    //! Sets the task as completed. Should be called when the task is complete.
    //! Calling will automatically emit the statusChanged and taskCompleted signals.
    void completed();

    //! Sets the task as stopped. Should be called whenever the task ends for any
    //! reason other than successful completion.
    //! Calling will automatically emit the statusChanged and taskStopped signals.
    void stopped();

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

    //! Will be emitted by task to indicate its commencement.
    //! @note derived classes should not emit this signal directly, it will automatically
    //! be emitted when the task begins
    void begun();

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

    //! Derived tasks must implement a run() method. This method will be called when the
    //! task commences (ie via calling start() ).
    virtual void run() = 0;

    //! Will return true if task should terminate ASAP. If the task reports the CanCancel
    //! flag, then derived classes' run() methods should periodically check this and
    //! terminate in a safe manner.
    bool isCancelled() const { return mShouldTerminate; }

  private:

    Flags mFlags;
    QString mDescription;
    TaskStatus mStatus;
    double mProgress;
    bool mShouldTerminate;

};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsTask::Flags )

//! List of QgsTask objects
typedef QList< QgsTask* > QgsTaskList;

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
     * to the manager, and the task manager will be responsible for starting
     * the task.
     * @param task task to add
     * @param dependencies list of dependent tasks. These tasks must be completed
     * before task can run. If any dependent tasks are cancelled this task will also
     * be cancelled. Dependent tasks must also be added to this task manager for proper
     * handling of dependencies.
     * @returns unique task ID
     */
    long addTask( QgsTask* task, const QgsTaskList& dependencies = QgsTaskList() );

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

    //! Instructs all tasks tracked by the manager to terminate.
    void cancelAll();

    //! Returns true if all dependencies for the specified task are satisfied
    bool dependenciesSatisified( long taskId ) const;

    //! Returns the set of task IDs on which a task is dependent
    //! @note not available in Python bindings
    QSet< long > dependencies( long taskId ) const;

    //! Will return true if the specified task has circular dependencies
    bool hasCircularDependencies( long taskId ) const;

    /** Sets a list of layers on which as task is dependent. The task will automatically
     * be cancelled if any of these layers are above to be removed.
     * @param taskId task ID
     * @param layerIds list of layer IDs
     * @see dependentLayers()
     */
    void setDependentLayers( long taskId, const QStringList& layerIds );

    /** Returns a list of layers on which as task is dependent. The task will automatically
     * be cancelled if any of these layers are above to be removed.
     * @param taskId task ID
     * @returns list of layer IDs
     * @see setDependentLayers()
     */
    QStringList dependentLayers( long taskId ) const;

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
    void layersWillBeRemoved( const QStringList& layerIds );

  private:

    static QgsTaskManager *sInstance;

    struct TaskInfo
    {
      TaskInfo( QgsTask* task = nullptr )
          : task( task )
      {}
      QgsTask* task;
      QFuture< void > future;
    };

    QMap< long, TaskInfo > mTasks;
    QMap< long, QgsTaskList > mTaskDependencies;
    QMap< long, QStringList > mLayerDependencies;

    //! Tracks the next unique task ID
    long mNextTaskId;

    bool cleanupAndDeleteTask( QgsTask* task );

    //! Process the queue of outstanding jobs and starts up any
    //! which are ready to go.
    void processQueue();

    //! Recursively cancel dependent tasks
    //! @param taskId id of terminated task to cancel any other tasks
    //! which are dependent on
    void cancelDependentTasks( long taskId );

    bool resolveDependencies( long firstTaskId, long currentTaskId, QSet< long >& results ) const;

};

#endif //QGSTASKMANAGER_H
