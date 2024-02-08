# The following has been generated automatically from src/core/processing/qgsprocessingcontext.h
QgsProcessingContext.Unused = QgsProcessingContext.Flag.Unused
QgsProcessingContext.Flags = lambda flags=0: QgsProcessingContext.Flag(flags)
# monkey patching scoped based enum
QgsProcessingContext.ProcessArgumentFlag.IncludeProjectPath.__doc__ = "Include the associated project path argument"
QgsProcessingContext.ProcessArgumentFlag.__doc__ = "Flags controlling the results given by :py:func:`~QgsProcessingContext.asQgisProcessArguments`.\n\n.. versionadded:: 3.24\n\n" + '* ``IncludeProjectPath``: ' + QgsProcessingContext.ProcessArgumentFlag.IncludeProjectPath.__doc__
# --
QgsProcessingContext.ProcessArgumentFlags = lambda flags=0: QgsProcessingContext.ProcessArgumentFlag(flags)
def _force_int(v): return v if isinstance(v, int) else int(v.value)


QgsProcessingContext.Flag.__bool__ = lambda flag: bool(_force_int(flag))
QgsProcessingContext.Flag.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsProcessingContext.Flag.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsProcessingContext.Flag.__or__ = lambda flag1, flag2: QgsProcessingContext.Flag(_force_int(flag1) | _force_int(flag2))
