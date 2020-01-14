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
#include "qgis_sip.h"
#include <QMap>
#include <QFuture>
#include <QReadWriteLock>

#include "qgis_core.h"
#include "qgsmaplayer.h"

class QgsTask;
class QgsTaskRunnableWrapper;

//! List of QgsTask objects
typedef QList< QgsTask * > QgsTaskList;

/**
 * \ingroup core
 * \class QgsTask
 * \brief Abstract base class for long running background tasks. Tasks can be controlled directly,
 * or added to a QgsTaskManager for automatic management.
 *
 * Derived classes should implement the process they want to execute in the background
 * within the run() method. This method will be called when the
 * task commences (ie via calling run() ).
 *
 * Long running tasks should periodically check the isCanceled() flag to detect if the task
 * has been canceled via some external event. If this flag is TRUE then the task should
 * clean up and terminate at the earliest possible convenience.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsTask : public QObject
{
    Q_OBJECT

  public:

    //! Status of tasks
    enum TaskStatus
    {
      Queued, //!< Task is queued and has not begun
      OnHold, //!< Task is queued but on hold and will not be started
      Running, //!< Task is currently running
      Complete, //!< Task successfully completed
      Terminated, //!< Task was terminated or errored
    };

    //! Task flags
    enum Flag
    {
      CanCancel = 1 << 1, //!< Task can be canceled
      AllFlags = CanCancel, //!< Task supports all flags
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    /**
     * Constructor for QgsTask.
     * \param description text description of task
     * \param flags task flags
     */
    QgsTask( const QString &description = QString(), QgsTask::Flags flags = AllFlags );

    ~QgsTask() override;

    /**
     * Returns the flags associated with the task.
     */
    Flags flags() const { return mFlags; }

    /**
     * Sets the task's \a description. This must be called before adding the task to a QgsTaskManager,
     * changing the description after queuing the task has no effect.
     * \since QGIS 3.10
     */
    void setDescription( const QString &description );

    /**
     * Returns TRUE if the task can be canceled.
     */
    bool canCancel() const { return mFlags & CanCancel; }

    /**
     * Returns TRUE if the task is active, ie it is not complete and has
     * not been canceled.
     */
    bool isActive() const { return mOverallStatus == Running; }

    /**
     * Returns the current task status.
     */
    TaskStatus status() const { return mOverallStatus; }

    /**
     * Returns the task's description.
     */
    QString description() const { return mDescription; }

    /**
     * Returns the task's progress (between 0.0 and 100.0)
     */
    double progress() const { return mTotalProgress; }

    /**
     * Returns the elapsed time since the task commenced, in milliseconds.
     *
     * The value is undefined for tasks which have not begun.
     *
     * \since QGIS 3.4
     */
    qint64 elapsedTime() const;

    /**
     * Notifies the task that it should terminate. Calling this is not guaranteed
     * to immediately end the task, rather it sets the isCanceled() flag which
     * task subclasses can check and terminate their operations at an appropriate
     * time. Any subtasks owned by this task will also be canceled.
     * Derived classes must ensure that the base class implementation is called
     * from any overridden version.
     * \see isCanceled()
     */
    virtual void cancel();

    /**
     * Places the task on hold. If the task in not queued
     * (ie it is already running or has finished) then calling this has no effect.
     * Calling this method only has an effect for tasks which are managed
     * by a QgsTaskManager.
     * \see unhold()
     */
    void hold();

    /**
     * Releases the task from being held. For tasks managed by a QgsTaskManager
     * calling this will re-add them to the queue. If the
     * task in not currently being held then calling this has no effect.
     * \see hold()
     */
    void unhold();

    //! Controls how subtasks relate to their parent task
    enum SubTaskDependency
    {
      SubTaskIndependent = 0, //!< Subtask is independent of the parent, and can run before, after or at the same time as the parent.
      ParentDependsOnSubTask, //!< Subtask must complete before parent can begin
    };

    /**
     * Adds a subtask to this task.
     *
     * Subtasks allow a single task to be created which
     * consists of multiple smaller tasks. Subtasks are not visible or indepedently
     * controllable by users. Ownership of the subtask is transferred.
     * Subtasks can have an optional list of dependent tasks, which must be completed
     * before the subtask can begin. By default subtasks are considered independent
     * of the parent task, ie they can be run either before, after, or at the same
     * time as the parent task. This behavior can be overridden through the subTaskDependency
     * argument. Note that subtasks should NEVER be dependent on their parent task, and violating
     * this constraint will prevent the task from completing successfully.
     *
     * The parent task must be added to a QgsTaskManager for subtasks to be utilized.
     * Subtasks should not be added manually to a QgsTaskManager, rather, only the parent
     * task should be added to the manager.
     *
     * Subtasks can be nested, ie a subtask can legally be a parent task itself with
     * its own set of subtasks.
     */
    void addSubTask( QgsTask *subTask SIP_TRANSFER, const QgsTaskList &dependencies = QgsTaskList(),
                     SubTaskDependency subTaskDependency = SubTaskIndependent );

    /**
     * Sets a list of layers on which the task depends. The task will automatically
     * be canceled if any of these layers are about to be removed.
     * \see dependentLayers()
     */
    void setDependentLayers( const QList<QgsMapLayer *> &dependentLayers );

    /**
     * Returns the list of layers on which the task depends. The task will automatically
     * be canceled if any of these layers are about to be removed.
     * \see setDependentLayers()
     */
    QList< QgsMapLayer * > dependentLayers() const;

    /**
     * Blocks the current thread until the task finishes or a maximum of \a timeout milliseconds.
     * If \a timeout is ``0`` the thread will be blocked forever.
     * In case of a timeout, the task will still be running.
     * In case the task already is finished, the method will return immediately while
     * returning ``TRUE``.
     *
     * The result will be FALSE if the wait timed out and TRUE in any other case.
     */
    bool waitForFinished( int timeout = 30000 );

  signals:

    /**
     * Will be emitted by task when its progress changes.
     * \param progress percent of progress, from 0.0 - 100.0
     * \note derived classes should not emit this signal directly, instead they should call
     * setProgress()
     */
    void progressChanged( double progress );

    /**
     * Will be emitted by task when its status changes.
     * \param status new task status
     * \note derived classes should not emit this signal directly, it will automatically
     * be emitted
     */
    void statusChanged( int status );

    /**
     * Will be emitted by task to indicate its commencement.
     * \note derived classes should not emit this signal directly, it will automatically
     * be emitted when the task begins
     */
    void begun();

    /**
     * Will be emitted by task to indicate its successful completion.
     * \note derived classes should not emit this signal directly, it will automatically
     * be emitted
     */
    void taskCompleted();

    /**
     * Will be emitted by task if it has terminated for any reason
     * other then completion (e.g., when a task has been canceled or encountered
     * an internal error).
     * \note derived classes should not emit this signal directly, it will automatically
     * be emitted
     */
    void taskTerminated();

  protected:

    /**
     * Performs the task's operation. This method will be called when the task commences
     * (ie via calling start() ), and subclasses should implement the operation they
     * wish to perform in the background within this method.
     *
     * A task must return a boolean value to indicate whether the
     * task was completed successfully or terminated before completion.
     */
    virtual bool run() = 0;

    /**
     * If the task is managed by a QgsTaskManager, this will be called after the
     * task has finished (whether through successful completion or via early
     * termination). The result argument reflects whether
     * the task was successfully completed or not. This method is always called
     * from the main thread, so it is safe to create widgets and perform other
     * operations which require the main thread. However, the GUI will be blocked
     * for the duration of this method so tasks should avoid performing any
     * lengthy operations here.
     */
    virtual void finished( bool result ) { Q_UNUSED( result ) }

    /**
     * Will return TRUE if task should terminate ASAP. If the task reports the CanCancel
     * flag, then derived classes' run() methods should periodically check this and
     * terminate in a safe manner.
     */
    bool isCanceled() const { return mShouldTerminate; }

  protected slots:

    /**
     * Sets the task's current progress. The derived class should call this method whenever
     * the task wants to update its progress. Calling will automatically emit the progressChanged signal.
     * \param progress percent of progress, from 0.0 - 100.0
     */
    void setProgress( double progress );

  private slots:
    void subTaskStatusChanged( int status );

  private:

    Flags mFlags;
    QString mDescription;
    //! Status of this (parent) task alone
    TaskStatus mStatus = Queued;
    //! Status of this task and all subtasks
    TaskStatus mOverallStatus = Queued;

    /**
     * This mutex remains locked from initialization until the task finishes,
     * it's used as a trigger for waitForFinished.
     */
    QMutex mNotFinishedMutex;

    /**
     * This semaphore remains locked from task creation until the task actually start,
     * it's used in waitForFinished to actually wait the task to be started.
     */
    QSemaphore mNotStartedMutex;

    //! Progress of this (parent) task alone
    double mProgress = 0.0;
    //! Overall progress of this task and all subtasks
    double mTotalProgress = 0.0;
    bool mShouldTerminate = false;
    int mStartCount = 0;

    struct SubTask
    {
      SubTask( QgsTask *task, const QgsTaskList &dependencies, SubTaskDependency dependency )
        : task( task )
        , dependencies( dependencies )
        , dependency( dependency )
      {}
      QgsTask *task = nullptr;
      QgsTaskList dependencies;
      SubTaskDependency dependency;
    };
    QList< SubTask > mSubTasks;

    QgsWeakMapLayerPointerList mDependentLayers;

    QElapsedTimer mElapsedTime;


    /**
     * Starts the task. Should not be public as only QgsTaskManagers can initiate tasks.
     */
    void start();

    /**
     * Called when the task has completed successfully.
     */
    void completed();

    /**
     * Called when the task has failed, as either a result of an internal failure or via cancellation.
     */
    void terminated();


    void processSubTasksForHold();

    friend class QgsTaskManager;
    friend class QgsTaskRunnableWrapper;
    friend class TestQgsTaskManager;

  private slots:

    void processSubTasksForCompletion();

    void processSubTasksForTermination();

};


Q_DECLARE_OPERATORS_FOR_FLAGS( QgsTask::Flags )

/**
 * \ingroup core
 * \class QgsTaskManager
 * \brief Task manager for managing a set of long-running QgsTask tasks. This class can be created directly,
 * or accessed via QgsApplication::taskManager().
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsTaskManager : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsTaskManager.
     * \param parent parent QObject
     */
    QgsTaskManager( QObject *parent SIP_TRANSFERTHIS = nullptr );

    ~QgsTaskManager() override;

    /**
     * Definition of a task for inclusion in the manager.
     */
    struct TaskDefinition
    {

      /**
       * Constructor for TaskDefinition. Ownership of the task is not transferred to the definition,
       * but will be transferred to a QgsTaskManager.
       */
      explicit TaskDefinition( QgsTask *task, const QgsTaskList &dependentTasks = QgsTaskList() )
        : task( task )
        , dependentTasks( dependentTasks )
      {}

      //! Task
      QgsTask *task = nullptr;

      /**
       * List of dependent tasks which must be completed before task can run. If any dependent tasks are
       * canceled this task will also be canceled. Dependent tasks must also be added
       * to the task manager for proper handling of dependencies.
       */
      QgsTaskList dependentTasks;

    };

    /**
     * Adds a task to the manager. Ownership of the task is transferred
     * to the manager, and the task manager will be responsible for starting
     * the task. The priority argument can be used to control the run queue's
     * order of execution, with larger numbers
     * taking precedence over lower priority numbers.
     * \returns unique task ID, or 0 if task could not be added
     */
    long addTask( QgsTask *task SIP_TRANSFER, int priority = 0 );

    /**
     * Adds a task to the manager, using a full task definition (including dependency
     * handling). Ownership of the task is transferred to the manager, and the task
     * manager will be responsible for starting the task. The priority argument can
     * be used to control the run queue's order of execution, with larger numbers
     * taking precedence over lower priority numbers.
     * \returns unique task ID, or 0 if task could not be added
     */
    long addTask( const TaskDefinition &task SIP_TRANSFER, int priority = 0 );

    /**
     * Returns the task with matching ID.
     * \param id task ID
     * \returns task if found, or NULLPTR
     */
    QgsTask *task( long id ) const;

    /**
     * Returns all tasks tracked by the manager.
     */
    QList<QgsTask *> tasks() const;

    //! Returns the number of tasks tracked by the manager.
    int count() const;

    /**
     * Returns the unique task ID corresponding to a task managed by the class.
     * \param task task to find
     * \returns task ID, or -1 if task not found
     */
    long taskId( QgsTask *task ) const;

    /**
     * Instructs all tasks tracked by the manager to terminate. Individual tasks may take some time
     * to cancel, or may totally ignore this instruction. Calling this does not block
     * but will instead signal the tasks to cancel and then return immediately.
     */
    void cancelAll();

    //! Returns TRUE if all dependencies for the specified task are satisfied
    bool dependenciesSatisfied( long taskId ) const;

    /**
     * Returns the set of task IDs on which a task is dependent
     * \note not available in Python bindings
     */
    QSet< long > dependencies( long taskId ) const SIP_SKIP;

    /**
     * Returns a list of layers on which as task is dependent. The task will automatically
     * be canceled if any of these layers are about to be removed.
     * \param taskId task ID
     * \returns list of layers
     * \see tasksDependentOnLayer()
     */
    QList< QgsMapLayer * > dependentLayers( long taskId ) const;

    /**
     * Returns a list of tasks which depend on a layer.
     * \see dependentLayers()
     */
    QList< QgsTask * > tasksDependentOnLayer( QgsMapLayer *layer ) const;

    /**
     * Returns a list of the active (queued or running) tasks.
     * \see countActiveTasks()
     */
    QList< QgsTask * > activeTasks() const;

    /**
     * Returns the number of active (queued or running) tasks.
     * \see activeTasks()
     * \see countActiveTasksChanged()
     */
    int countActiveTasks() const;

  public slots:

    /**
     * Triggers a task, e.g. as a result of a GUI interaction.
     * \see taskTriggered()
     */
    void triggerTask( QgsTask *task );

  signals:

    /**
     * Will be emitted when a task reports a progress change
     * \param taskId ID of task
     * \param progress percent of progress, from 0.0 - 100.0
     */
    void progressChanged( long taskId, double progress );

    /**
     * Will be emitted when only a single task remains to complete
     * and that task has reported a progress change
     * \param progress percent of progress, from 0.0 - 100.0
     */
    void finalTaskProgressChanged( double progress );

    /**
     * Will be emitted when a task reports a status change
     * \param taskId ID of task
     * \param status new task status
     */
    void statusChanged( long taskId, int status );

    /**
     * Emitted when a new task has been added to the manager
     * \param taskId ID of task
     */
    void taskAdded( long taskId );

    /**
     * Emitted when a task is about to be deleted
     * \param taskId ID of task
     */
    void taskAboutToBeDeleted( long taskId );

    /**
     * Emitted when all tasks are complete
     * \see countActiveTasksChanged()
     */
    void allTasksFinished();

    /**
     * Emitted when the number of active tasks changes
     * \see countActiveTasks()
     */
    void countActiveTasksChanged( int count );

    /**
     * Emitted when a \a task is triggered. This occurs when a user clicks on
     * the task from the QGIS GUI, and can be used to show detailed progress
     * reports or re-open a related dialog.
     * \see triggerTask()
     */
    void taskTriggered( QgsTask *task );

  private slots:

    void taskProgressChanged( double progress );
    void taskStatusChanged( int status );
    void layersWillBeRemoved( const QList<QgsMapLayer *> &layers );

  private:

    struct TaskInfo
    {
      TaskInfo( QgsTask *task = nullptr, int priority = 0 );
      void createRunnable();
      QgsTask *task = nullptr;
      QAtomicInt added;
      int priority;
      QgsTaskRunnableWrapper *runnable = nullptr;
    };

    mutable QMutex *mTaskMutex;

    QMap< long, TaskInfo > mTasks;
    QMap< long, QgsTaskList > mTaskDependencies;
    QMap< long, QgsWeakMapLayerPointerList > mLayerDependencies;

    //! Tracks the next unique task ID
    long mNextTaskId = 1;

    //! List of active (queued or running) tasks. Includes subtasks.
    QSet< QgsTask * > mActiveTasks;
    //! List of parent tasks
    QSet< QgsTask * > mParentTasks;
    //! List of subtasks
    QSet< QgsTask * > mSubTasks;

    QSet< QgsTask * > mPendingDeletion;

    long addTaskPrivate( QgsTask *task,
                         QgsTaskList dependencies,
                         bool isSubTask,
                         int priority );

    bool cleanupAndDeleteTask( QgsTask *task );

    /**
     * Process the queue of outstanding jobs and starts up any
     * which are ready to go.
     */
    void processQueue();

    /**
     * Recursively cancel dependent tasks
     * \param taskId id of terminated task to cancel any other tasks
     * which are dependent on
     */
    void cancelDependentTasks( long taskId );

    bool resolveDependencies( long firstTaskId, long currentTaskId, QSet< long > &results ) const;

    //! Will return TRUE if the specified task has circular dependencies
    bool hasCircularDependencies( long taskId ) const;

    friend class TestQgsTaskManager;
};

#endif //QGSTASKMANAGER_H
