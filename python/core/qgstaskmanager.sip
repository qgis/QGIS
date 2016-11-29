/**
 * \ingroup core
 * \class QgsTask
 * \brief Abstract base class for long running background tasks. Tasks can be controlled directly,
 * or added to a QgsTaskManager for automatic management.
 *
 * Derived classes should implement the process they want to execute in the background
 * within the run() method. This method will be called when the
 * task commences (ie via calling start() ).
 *
 * Long running tasks should periodically check the isCancelled() flag to detect if the task
 * has been cancelled via some external event. If this flag is true then the task should
 * clean up and terminate at the earliest possible convenience.
 *
 * \note Added in version 3.0
 */
class QgsTask : QObject, QRunnable
{

%TypeHeaderCode
#include <qgstaskmanager.h>
%End
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

    //! Result of running the task
    enum TaskResult
    {
      ResultSuccess, //!< Task completed successfully
      ResultFail, //!< Task was terminated within completion
      ResultPending, //!< Task is still running
    };

    //! Task flags
    enum Flag
    {
      CanCancel, //!< Task can be cancelled
      CanReportProgress, //!< Task will report its progress
      AllFlags, //!< Task supports all flags
    };
    typedef QFlags<QgsTask::Flag> Flags;

   /**
     * Constructor for QgsTask.
     * @param description text description of task
     * @param flags task flags
     */
    QgsTask( const QString& description = QString(), const Flags& flags = AllFlags );

    /**
     * Returns the flags associated with the task.
     */
    Flags flags() const;

    /**
     * Returns true if the task can be cancelled.
     */
    bool canCancel() const;

    /**
     * Returns true if the task is active, ie it is not complete and has
     * not been cancelled.
     */
    bool isActive() const;

    /**
     * Returns the current task status.
     */
    TaskStatus status() const;

    /**
     * Returns the task's description.
     */
    QString description() const;

    /**
     * Returns the task's progress (between 0.0 and 100.0)
     */
    double progress() const;

    /**
     * Starts the task. Should only be called for tasks which are not being
     * handled by a QgsTaskManager. If the task is managed by a QgsTaskManager
     * then this method should not be called directly, instead it is left to the
     * task manager to start the task when appropriate.
     */
    void run();

    /**
     * Notifies the task that it should terminate. Calling this is not guaranteed
     * to immediately end the task, rather it sets the isCancelled() flag which
     * task subclasses can check and terminate their operations at an appropriate
     * time. Any subtasks owned by this task will also be cancelled.
     * @see isCancelled()
     */
    void cancel();

    /**
     * Places the task on hold. If the task in not queued
     * (ie it is already running or has finished) then calling this has no effect.
     * Calling this method only has an effect for tasks which are managed
     * by a QgsTaskManager.
     * @see unhold()
     */
    void hold();

    /**
     * Releases the task from being held. For tasks managed by a QgsTaskManager
     * calling this will re-add them to the queue. If the
     * task in not currently being held then calling this has no effect.
     * @see hold()
     */
    void unhold();

    //! Controls how subtasks relate to their parent task
    enum SubTaskDependency
    {
      SubTaskIndependent, //!< Subtask is independent of the parent, and can run before, after or at the same time as the parent.
      ParentDependsOnSubTask, //!< Subtask must complete before parent can begin
    };

    /**
     * Adds a subtask to this task.
     *
     * Subtasks allow a single task to be created which
     * consists of multiple smaller tasks. Subtasks are not visible or indepedently
     * controllable by users. Ownership of the subtask is transferred.
     * Subtasks can have an optional list of dependant tasks, which must be completed
     * before the subtask can begin. By default subtasks are considered independent
     * of the parent task, ie they can be run either before, after, or at the same
     * time as the parent task. This behaviour can be overriden through the subTaskDependency
     * argument.
     *
     * The parent task must be added to a QgsTaskManager for subtasks to be utilised.
     * Subtasks should not be added manually to a QgsTaskManager, rather, only the parent
     * task should be added to the manager.
     *
     * Subtasks can be nested, ie a subtask can legally be a parent task itself with
     * its own set of subtasks.
     */
    void addSubTask( QgsTask* subTask /Transfer/, const QgsTaskList& dependencies = QgsTaskList(),
                     SubTaskDependency subTaskDependency = SubTaskIndependent );

  signals:

    /**
     * Will be emitted by task when its progress changes.
     * @param progress percent of progress, from 0.0 - 100.0
     * @note derived classes should not emit this signal directly, instead they should call
     * setProgress()
     */
    void progressChanged( double progress );

    /**
     * Will be emitted by task when its status changes.
     * @param status new task status
     * @note derived classes should not emit this signal directly, instead they should call
     * completed() or terminated()
     */
    void statusChanged( int status );

    /**
     * Will be emitted by task to indicate its commencement.
     * @note derived classes should not emit this signal directly, it will automatically
     * be emitted when the task begins
     */
    void begun();

    /**
     * Will be emitted by task to indicate its successful completion.
     * @note derived classes should not emit this signal directly, instead they should call
     * completed()
     */
    void taskCompleted();

    /**
     * Will be emitted by task if it has terminated for any reason
     * other then completion (eg when a task has been cancelled or encountered
     * an internal error).
     * @note derived classes should not emit this signal directly, instead they should call
     * terminated()
     */
    void taskTerminated();

  protected:

    /**
     * Performs the task's operation. This method will be called when the task commences
     * (ie via calling start() ), and subclasses should implement the operation they
     * wish to perform in the background within this method.
     *
     * A task can return a ResultSuccess and ResultFail value to indicate that the
     * task has finished and was either completed successfully or terminated before
     * completion.
     *
     * Alternatively, tasks can also return the ResultPending value
     * to indicate that the task is still operating and will manually report its
     * completion by calling completed() or terminated(). This may be useful for
     * tasks which rely on external events for completion, eg downloading a
     * file. In this case Qt slots could be created which are connected to the
     * download completion or termination and which call completed() or terminated()
     * to indicate the task has finished operations.
     * @see completed()
     * @see terminated()
     */
    virtual TaskResult _run() = 0;

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
    virtual void finished( TaskResult result );

    /**
     * Will return true if task should terminate ASAP. If the task reports the CanCancel
     * flag, then derived classes' run() methods should periodically check this and
     * terminate in a safe manner.
     */
    bool isCancelled() const;

    /**
     * Sets the task as completed. Calling this is only required for tasks which
     * returned the ResultPending value as a result of run(). This should be called
     * when the task is complete. Calling will automatically emit the statusChanged
     * and taskCompleted signals.
     */
    void completed();

    /**
     * Sets the task as terminated.  Calling this is only required for tasks which
     * returned the ResultPending value as a result of run().
     * Should be called whenever the task ends for any reason other than successful
     * completion. Calling will automatically emit the statusChanged and taskTerminated
     * signals.
     */
    void terminated();

  protected slots:

    /**
     * Sets the task's current progress. If task reports the CanReportProgress flag then
     * the derived class should call this method whenever the task wants to update its
     * progress. Calling will automatically emit the progressChanged signal.
     * @param progress percent of progress, from 0.0 - 100.0
     */
    void setProgress( double progress );

};

QFlags<QgsTask::Flag> operator|(QgsTask::Flag f1, QFlags<QgsTask::Flag> f2);

//! List of QgsTask objects
typedef QList< QgsTask* > QgsTaskList;

/** \ingroup core
 * \class QgsTaskManager
 * \brief Task manager for managing a set of long-running QgsTask tasks. This class can be created directly,
 * or accessed via a global instance.
 * \note Added in version 2.16
 */
class QgsTaskManager : QObject
{
%TypeHeaderCode
#include <qgstaskmanager.h>
%End
  public:

    /** Constructor for QgsTaskManager.
     * @param parent parent QObject
     */
    QgsTaskManager( QObject* parent /TransferThis/ = nullptr );

    virtual ~QgsTaskManager();

    /**
     * Definition of a task for inclusion within a task bundle.
     */
    struct TaskDefinition
    {
      /**
       * Constructor for TaskDefinition. Ownership of the task is transferred to the definition.
       */
      explicit TaskDefinition( QgsTask* task, QgsTaskList dependencies = QgsTaskList() );

      //! Task
      QgsTask* task;

      /**
       * List of dependencies which must be completed before task can run.
       * These tasks must be completed before task can run. If any dependent tasks are
       * cancelled this task will also be cancelled. Dependent tasks must also be added
       * to the task manager for proper handling of dependencies.
       */
      QgsTaskList dependencies;
    };

    /** Adds a task to the manager. Ownership of the task is transferred
     * to the manager, and the task manager will be responsible for starting
     * the task. The priority argument can be used to control the run queue's
     * order of execution.
     * @returns unique task ID
     */
    long addTask( QgsTask* task /Transfer/, int priority = 0 );

    /**
     * Adds a task to the manager, using a full task definition (including dependancy
     * handling). Ownership of the task is transferred to the manager, and the task
     * manager will be responsible for starting the task. The priority argument can
     * be used to control the run queue's order of execution.
     * @returns unique task ID
     */
    long addTask( const TaskDefinition& task /Transfer/, int priority = 0 );


    /** Returns the task with matching ID.
     * @param id task ID
     * @returns task if found, or nullptr
     */
    QgsTask* task( long id ) const;

    /** Returns all tasks tracked by the manager.
     */
    QList<QgsTask*> tasks() const;

    //! Returns the number of tasks tracked by the manager.
    int count() const;

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
    //QSet< long > dependencies( long taskId ) const;

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

    /** Returns a list of the active (queued or running) tasks.
     * @see countActiveTasks()
     */
    QList< QgsTask* > activeTasks() const;

    /** Returns the number of active (queued or running) tasks.
     * @see activeTasks()
     * @see countActiveTasksChanged()
     */
    int countActiveTasks() const;

  signals:

    //! Will be emitted when a task reports a progress change
    //! @param taskId ID of task
    //! @param progress percent of progress, from 0.0 - 100.0
    void progressChanged( long taskId, double progress );

    //! Will be emitted when only a single task remains to complete
    //! and that task has reported a progress change
    //! @param progress percent of progress, from 0.0 - 100.0
    void finalTaskProgressChanged( double progress );

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

    //! Emitted when all tasks are complete
    //! @see countActiveTasksChanged()
    void allTasksFinished();

    //! Emitted when the number of active tasks changes
    //! @see countActiveTasks()
    void countActiveTasksChanged( int count );

};
