# The following has been generated automatically from src/core/qgstaskmanager.h
QgsTask.Queued = QgsTask.TaskStatus.Queued
QgsTask.OnHold = QgsTask.TaskStatus.OnHold
QgsTask.Running = QgsTask.TaskStatus.Running
QgsTask.Complete = QgsTask.TaskStatus.Complete
QgsTask.Terminated = QgsTask.TaskStatus.Terminated
QgsTask.TaskStatus.baseClass = QgsTask
QgsTask.CanCancel = QgsTask.Flag.CanCancel
QgsTask.CancelWithoutPrompt = QgsTask.Flag.CancelWithoutPrompt
QgsTask.Hidden = QgsTask.Flag.Hidden
QgsTask.Silent = QgsTask.Flag.Silent
QgsTask.AllFlags = QgsTask.Flag.AllFlags
QgsTask.Flags = lambda flags=0: QgsTask.Flag(flags)
QgsTask.SubTaskIndependent = QgsTask.SubTaskDependency.SubTaskIndependent
QgsTask.ParentDependsOnSubTask = QgsTask.SubTaskDependency.ParentDependsOnSubTask
from enum import Enum


def _force_int(v): return int(v.value) if isinstance(v, Enum) else v


QgsTask.Flag.__bool__ = lambda flag: bool(_force_int(flag))
QgsTask.Flag.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsTask.Flag.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsTask.Flag.__or__ = lambda flag1, flag2: QgsTask.Flag(_force_int(flag1) | _force_int(flag2))
try:
    QgsTask.__attribute_docs__ = {'progressChanged': 'Will be emitted by task when its progress changes.\n\n:param progress: percent of progress, from 0.0 - 100.0\n\n.. note::\n\n   derived classes should not emit this signal directly, instead they should call\n   :py:func:`~QgsTask.setProgress`\n', 'statusChanged': 'Will be emitted by task when its status changes.\n\n:param status: new task status\n\n.. note::\n\n   derived classes should not emit this signal directly, it will automatically\n   be emitted\n', 'begun': 'Will be emitted by task to indicate its commencement.\n\n.. note::\n\n   derived classes should not emit this signal directly, it will automatically\n   be emitted when the task begins\n', 'taskCompleted': 'Will be emitted by task to indicate its successful completion.\n\n.. note::\n\n   derived classes should not emit this signal directly, it will automatically\n   be emitted\n', 'taskTerminated': 'Will be emitted by task if it has terminated for any reason\nother then completion (e.g., when a task has been canceled or encountered\nan internal error).\n\n.. note::\n\n   derived classes should not emit this signal directly, it will automatically\n   be emitted\n'}
    QgsTask.__signal_arguments__ = {'progressChanged': ['progress: float'], 'statusChanged': ['status: int']}
except (NameError, AttributeError):
    pass
try:
    QgsTaskManager.TaskDefinition.__attribute_docs__ = {'task': 'Task', 'dependentTasks': 'List of dependent tasks which must be completed before task can run. If any dependent tasks are\ncanceled this task will also be canceled. Dependent tasks must also be added\nto the task manager for proper handling of dependencies.'}
    QgsTaskManager.TaskDefinition.__doc__ = """Definition of a task for inclusion in the manager."""
except (NameError, AttributeError):
    pass
try:
    QgsTaskManager.__attribute_docs__ = {'progressChanged': 'Will be emitted when a task reports a progress change\n\n:param taskId: ID of task\n:param progress: percent of progress, from 0.0 - 100.0\n', 'finalTaskProgressChanged': 'Will be emitted when only a single task remains to complete\nand that task has reported a progress change\n\n:param progress: percent of progress, from 0.0 - 100.0\n', 'statusChanged': 'Will be emitted when a task reports a status change\n\n:param taskId: ID of task\n:param status: new task status\n', 'taskAdded': 'Emitted when a new task has been added to the manager\n\n:param taskId: ID of task\n', 'taskAboutToBeDeleted': 'Emitted when a task is about to be deleted\n\n:param taskId: ID of task\n', 'allTasksFinished': 'Emitted when all tasks are complete\n\n.. seealso:: :py:func:`countActiveTasksChanged`\n', 'countActiveTasksChanged': 'Emitted when the number of active tasks changes\n\n.. seealso:: :py:func:`countActiveTasks`\n', 'taskTriggered': 'Emitted when a ``task`` is triggered. This occurs when a user clicks on\nthe task from the QGIS GUI, and can be used to show detailed progress\nreports or re-open a related dialog.\n\n.. seealso:: :py:func:`triggerTask`\n'}
    QgsTaskManager.__signal_arguments__ = {'progressChanged': ['taskId: int', 'progress: float'], 'finalTaskProgressChanged': ['progress: float'], 'statusChanged': ['taskId: int', 'status: int'], 'taskAdded': ['taskId: int'], 'taskAboutToBeDeleted': ['taskId: int'], 'countActiveTasksChanged': ['count: int'], 'taskTriggered': ['task: QgsTask']}
except (NameError, AttributeError):
    pass
