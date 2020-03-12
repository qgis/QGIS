# The following has been generated automatically from src/core/qgsproject.h
# monkey patching scoped based enum
QgsProject.FlagDontResolveLayers = QgsProject.ReadFlag.FlagDontResolveLayers
QgsProject.ReadFlag.FlagDontResolveLayers.__doc__ = "Don't resolve layer paths (i.e. don't load any layer content). Dramatically improves project read time if the actual data from the layers is not required."
QgsProject.FlagDontLoadLayouts = QgsProject.ReadFlag.FlagDontLoadLayouts
QgsProject.ReadFlag.FlagDontLoadLayouts.__doc__ = "Don't load print layouts. Improves project read time if layouts are not required, and allows projects to be safely read in background threads (since print layouts are not thread safe)."
QgsProject.ReadFlag.__doc__ = 'Flags which control project read behavior.\n\n.. versionadded:: 3.10\n\n' + '* ``FlagDontResolveLayers``: ' + QgsProject.ReadFlag.FlagDontResolveLayers.__doc__ + '\n' + '* ``FlagDontLoadLayouts``: ' + QgsProject.ReadFlag.FlagDontLoadLayouts.__doc__
# --
# monkey patching scoped based enum
QgsProject.FileFormat.Qgz.__doc__ = "Archive file format, supports auxiliary data"
QgsProject.FileFormat.Qgs.__doc__ = "Project saved in a clear text, does not support auxiliary data"
QgsProject.FileFormat.__doc__ = 'Flags which control project read behavior.\n\n.. versionadded:: 3.12\n\n' + '* ``Qgz``: ' + QgsProject.FileFormat.Qgz.__doc__ + '\n' + '* ``Qgs``: ' + QgsProject.FileFormat.Qgs.__doc__
# --
QgsProject.FileFormat.baseClass = QgsProject
