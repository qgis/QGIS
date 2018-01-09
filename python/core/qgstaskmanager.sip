/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgstaskmanager.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






typedef QList< QgsTask * > QgsTaskList;

class QgsTask : QObject
{
%Docstring
 Abstract base class for long running background tasks. Tasks can be controlled directly,
or added to a QgsTaskManager for automatic management.

Derived classes should implement the process they want to execute in the background
within the run() method. This method will be called when the
task commences (ie via calling run() ).

Long running tasks should periodically check the isCanceled() flag to detect if the task
has been canceled via some external event. If this flag is true then the task should
clean up and terminate at the earliest possible convenience.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgstaskmanager.h"
%End
  public:

    enum TaskStatus
    {
      Queued,
      OnHold,
      Running,
      Complete,
      Terminated,
    };

    enum Flag
    {
      CanCancel,
      AllFlags,
    };
    typedef QFlags<QgsTask::Flag> Flags;


    QgsTask( const QString &description = QString(), const Flags &flags = AllFlags );
%Docstring
Constructor for QgsTask.

:param description: text description of task
:param flags: task flags
%End

    ~QgsTask();

    Flags flags() const;
%Docstring
Returns the flags associated with the task.
%End

    bool canCancel() const;
%Docstring
Returns true if the task can be canceled.
%End

    bool isActive() const;
%Docstring
Returns true if the task is active, ie it is not complete and has
not been canceled.
%End

    TaskStatus status() const;
%Docstring
Returns the current task status.
%End

    QString description() const;
%Docstring
Returns the task's description.
%End

    double progress() const;
%Docstring
Returns the task's progress (between 0.0 and 100.0)
%End

    virtual void cancel();
%Docstring
Notifies the task that it should terminate. Calling this is not guaranteed
to immediately end the task, rather it sets the isCanceled() flag which
task subclasses can check and terminate their operations at an appropriate
time. Any subtasks owned by this task will also be canceled.
Derived classes must ensure that the base class implementation is called
from any overridden version.

.. seealso:: :py:func:`isCanceled`
%End

    void hold();
%Docstring
Places the task on hold. If the task in not queued
(ie it is already running or has finished) then calling this has no effect.
Calling this method only has an effect for tasks which are managed
by a QgsTaskManager.

.. seealso:: :py:func:`unhold`
%End

    void unhold();
%Docstring
Releases the task from being held. For tasks managed by a QgsTaskManager
calling this will re-add them to the queue. If the
task in not currently being held then calling this has no effect.

.. seealso:: :py:func:`hold`
%End

    enum SubTaskDependency
    {
      SubTaskIndependent,
      ParentDependsOnSubTask,
    };

    void addSubTask( QgsTask *subTask /Transfer/, const QgsTaskList &dependencies = QgsTaskList(),
                     SubTaskDependency subTaskDependency = SubTaskIndependent );
%Docstring
Adds a subtask to this task.

Subtasks allow a single task to be created which
consists of multiple smaller tasks. Subtasks are not visible or indepedently
controllable by users. Ownership of the subtask is transferred.
Subtasks can have an optional list of dependent tasks, which must be completed
before the subtask can begin. By default subtasks are considered independent
of the parent task, ie they can be run either before, after, or at the same
time as the parent task. This behavior can be overridden through the subTaskDependency
argument. Note that subtasks should NEVER be dependent on their parent task, and violating
this constraint will prevent the task from completing successfully.

The parent task must be added to a QgsTaskManager for subtasks to be utilized.
Subtasks should not be added manually to a QgsTaskManager, rather, only the parent
task should be added to the manager.

Subtasks can be nested, ie a subtask can legally be a parent task itself with
its own set of subtasks.
%End

    void setDependentLayers( const QList<QgsMapLayer *> &dependentLayers );
%Docstring
Sets a list of layers on which the task depends. The task will automatically
be canceled if any of these layers are about to be removed.

.. seealso:: :py:func:`dependentLayerIds`
%End

    QList< QgsMapLayer * > dependentLayers() const;
%Docstring
Returns the list of layers on which the task depends. The task will automatically
be canceled if any of these layers are about to be removed.

.. seealso:: :py:func:`setDependentLayers`
%End

    bool waitForFinished( unsigned long timeout = 30000 );
%Docstring
Blocks the current thread until the task finishes or a maximum of ``timeout`` milliseconds.
If ``timeout`` is ``0`` the thread will be blocked forever.
In case of a timeout, the task will still be running.
In case the task already is finished, the method will return immediately while
returning ``true``.

The result will be false if the wait timed out and true in any other case.
%End

  signals:

    void progressChanged( double progress );
%Docstring
Will be emitted by task when its progress changes.

:param progress: percent of progress, from 0.0 - 100.0

.. note::

   derived classes should not emit this signal directly, instead they should call
setProgress()
%End

    void statusChanged( int status );
%Docstring
Will be emitted by task when its status changes.

:param status: new task status

.. note::

   derived classes should not emit this signal directly, it will automatically
be emitted
%End

    void begun();
%Docstring
Will be emitted by task to indicate its commencement.

.. note::

   derived classes should not emit this signal directly, it will automatically
be emitted when the task begins
%End

    void taskCompleted();
%Docstring
Will be emitted by task to indicate its successful completion.

.. note::

   derived classes should not emit this signal directly, it will automatically
be emitted
%End

    void taskTerminated();
%Docstring
Will be emitted by task if it has terminated for any reason
other then completion (e.g., when a task has been canceled or encountered
an internal error).

.. note::

   derived classes should not emit this signal directly, it will automatically
be emitted
%End

  protected:

    virtual bool run() = 0;
%Docstring
Performs the task's operation. This method will be called when the task commences
(ie via calling start() ), and subclasses should implement the operation they
wish to perform in the background within this method.

A task must return a boolean value to indicate whether the
task was completed successfully or terminated before completion.
%End

    virtual void finished( bool result );
%Docstring
If the task is managed by a QgsTaskManager, this will be called after the
task has finished (whether through successful completion or via early
termination). The result argument reflects whether
the task was successfully completed or not. This method is always called
from the main thread, so it is safe to create widgets and perform other
operations which require the main thread. However, the GUI will be blocked
for the duration of this method so tasks should avoid performing any
lengthy operations here.
%End

    bool isCanceled() const;
%Docstring
Will return true if task should terminate ASAP. If the task reports the CanCancel
flag, then derived classes' run() methods should periodically check this and
terminate in a safe manner.
%End

  protected slots:

    void setProgress( double progress );
%Docstring
Sets the task's current progress. The derived class should call this method whenever
the task wants to update its progress. Calling will automatically emit the progressChanged signal.

:param progress: percent of progress, from 0.0 - 100.0
%End

};


QFlags<QgsTask::Flag> operator|(QgsTask::Flag f1, QFlags<QgsTask::Flag> f2);


class QgsTaskManager : QObject
{
%Docstring
 Task manager for managing a set of long-running QgsTask tasks. This class can be created directly,
or accessed via :py:func:`QgsApplication.taskManager()`

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgstaskmanager.h"
%End
  public:

    QgsTaskManager( QObject *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsTaskManager.

:param parent: parent QObject
%End

    ~QgsTaskManager();

    struct TaskDefinition
    {

      explicit TaskDefinition( QgsTask *task, QgsTaskList dependentTasks = QgsTaskList() );
%Docstring
Constructor for TaskDefinition. Ownership of the task is not transferred to the definition,
but will be transferred to a QgsTaskManager.
%End

      QgsTask *task;

      QgsTaskList dependentTasks;

    };

    long addTask( QgsTask *task /Transfer/, int priority = 0 );
%Docstring
Adds a task to the manager. Ownership of the task is transferred
to the manager, and the task manager will be responsible for starting
the task. The priority argument can be used to control the run queue's
order of execution, with larger numbers
taking precedence over lower priority numbers.

:return: unique task ID
%End

    long addTask( const TaskDefinition &task /Transfer/, int priority = 0 );
%Docstring
Adds a task to the manager, using a full task definition (including dependency
handling). Ownership of the task is transferred to the manager, and the task
manager will be responsible for starting the task. The priority argument can
be used to control the run queue's order of execution, with larger numbers
taking precedence over lower priority numbers.

:return: unique task ID
%End

    QgsTask *task( long id ) const;
%Docstring
Returns the task with matching ID.

:param id: task ID

:return: task if found, or None
%End

    QList<QgsTask *> tasks() const;
%Docstring
Returns all tasks tracked by the manager.
%End

    int count() const;
%Docstring
Returns the number of tasks tracked by the manager.
%End

    long taskId( QgsTask *task ) const;
%Docstring
Returns the unique task ID corresponding to a task managed by the class.

:param task: task to find

:return: task ID, or -1 if task not found
%End

    void cancelAll();
%Docstring
Instructs all tasks tracked by the manager to terminate. Individual tasks may take some time
to cancel, or may totally ignore this instruction. Calling this does not block
but will instead signal the tasks to cancel and then return immediately.
%End

    bool dependenciesSatisfied( long taskId ) const;
%Docstring
Returns true if all dependencies for the specified task are satisfied
%End


    QList< QgsMapLayer * > dependentLayers( long taskId ) const;
%Docstring
Returns a list of layers on which as task is dependent. The task will automatically
be canceled if any of these layers are above to be removed.

:param taskId: task ID

:return: list of layers

.. seealso:: :py:func:`tasksDependentOnLayer`
%End

    QList< QgsTask * > tasksDependentOnLayer( QgsMapLayer *layer ) const;
%Docstring
Returns a list of tasks which depend on a layer.

.. seealso:: :py:func:`dependentLayers`
%End

    QList< QgsTask * > activeTasks() const;
%Docstring
Returns a list of the active (queued or running) tasks.

.. seealso:: :py:func:`countActiveTasks`
%End

    int countActiveTasks() const;
%Docstring
Returns the number of active (queued or running) tasks.

.. seealso:: :py:func:`activeTasks`

.. seealso:: :py:func:`countActiveTasksChanged`
%End

  signals:

    void progressChanged( long taskId, double progress );
%Docstring
Will be emitted when a task reports a progress change

:param taskId: ID of task
:param progress: percent of progress, from 0.0 - 100.0
%End

    void finalTaskProgressChanged( double progress );
%Docstring
Will be emitted when only a single task remains to complete
and that task has reported a progress change

:param progress: percent of progress, from 0.0 - 100.0
%End

    void statusChanged( long taskId, int status );
%Docstring
Will be emitted when a task reports a status change

:param taskId: ID of task
:param status: new task status
%End

    void taskAdded( long taskId );
%Docstring
Emitted when a new task has been added to the manager

:param taskId: ID of task
%End

    void taskAboutToBeDeleted( long taskId );
%Docstring
Emitted when a task is about to be deleted

:param taskId: ID of task
%End

    void allTasksFinished();
%Docstring
Emitted when all tasks are complete

.. seealso:: :py:func:`countActiveTasksChanged`
%End

    void countActiveTasksChanged( int count );
%Docstring
Emitted when the number of active tasks changes

.. seealso:: :py:func:`countActiveTasks`
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgstaskmanager.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
