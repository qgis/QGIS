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
