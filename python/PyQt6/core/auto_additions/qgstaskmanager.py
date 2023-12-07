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
