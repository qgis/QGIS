# The following has been generated automatically from src/core/processing/qgsprocessingcontext.h
QgsProcessingContext.Unused = QgsProcessingContext.Flag.Unused
QgsProcessingContext.Flags = lambda flags=0: QgsProcessingContext.Flag(flags)
QgsProcessingContext.DefaultLevel = QgsProcessingContext.LogLevel.DefaultLevel
QgsProcessingContext.Verbose = QgsProcessingContext.LogLevel.Verbose
QgsProcessingContext.ModelDebug = QgsProcessingContext.LogLevel.ModelDebug
# monkey patching scoped based enum
QgsProcessingContext.ProcessArgumentFlag.IncludeProjectPath.__doc__ = "Include the associated project path argument"
QgsProcessingContext.ProcessArgumentFlag.__doc__ = "Flags controlling the results given by :py:func:`~QgsProcessingContext.asQgisProcessArguments`.\n\n.. versionadded:: 3.24\n\n" + '* ``IncludeProjectPath``: ' + QgsProcessingContext.ProcessArgumentFlag.IncludeProjectPath.__doc__
# --
QgsProcessingContext.ProcessArgumentFlags = lambda flags=0: QgsProcessingContext.ProcessArgumentFlag(flags)
