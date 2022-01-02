# The following has been generated automatically from src/core/project/qgsproject.h
# monkey patching scoped based enum
QgsProject.FlagDontResolveLayers = QgsProject.ReadFlag.FlagDontResolveLayers
QgsProject.FlagDontResolveLayers.is_monkey_patched = True
QgsProject.ReadFlag.FlagDontResolveLayers.__doc__ = "Don't resolve layer paths (i.e. don't load any layer content). Dramatically improves project read time if the actual data from the layers is not required."
QgsProject.FlagDontLoadLayouts = QgsProject.ReadFlag.FlagDontLoadLayouts
QgsProject.FlagDontLoadLayouts.is_monkey_patched = True
QgsProject.ReadFlag.FlagDontLoadLayouts.__doc__ = "Don't load print layouts. Improves project read time if layouts are not required, and allows projects to be safely read in background threads (since print layouts are not thread safe)."
QgsProject.FlagTrustLayerMetadata = QgsProject.ReadFlag.FlagTrustLayerMetadata
QgsProject.FlagTrustLayerMetadata.is_monkey_patched = True
QgsProject.ReadFlag.FlagTrustLayerMetadata.__doc__ = "Trust layer metadata. Improves project read time. Do not use it if layers' extent is not fixed during the project's use by QGIS and QGIS Server."
QgsProject.FlagDontStoreOriginalStyles = QgsProject.ReadFlag.FlagDontStoreOriginalStyles
QgsProject.FlagDontStoreOriginalStyles.is_monkey_patched = True
QgsProject.ReadFlag.FlagDontStoreOriginalStyles.__doc__ = "Skip the initial XML style storage for layers. Useful for minimising project load times in non-interactive contexts."
QgsProject.FlagDontLoad3DViews = QgsProject.ReadFlag.FlagDontLoad3DViews
QgsProject.FlagDontLoad3DViews.is_monkey_patched = True
QgsProject.ReadFlag.FlagDontLoad3DViews.__doc__ = ""
QgsProject.ReadFlag.__doc__ = 'Flags which control project read behavior.\n\n.. versionadded:: 3.10\n\n' + '* ``FlagDontResolveLayers``: ' + QgsProject.ReadFlag.FlagDontResolveLayers.__doc__ + '\n' + '* ``FlagDontLoadLayouts``: ' + QgsProject.ReadFlag.FlagDontLoadLayouts.__doc__ + '\n' + '* ``FlagTrustLayerMetadata``: ' + QgsProject.ReadFlag.FlagTrustLayerMetadata.__doc__ + '\n' + '* ``FlagDontStoreOriginalStyles``: ' + QgsProject.ReadFlag.FlagDontStoreOriginalStyles.__doc__ + '\n' + '* ``FlagDontLoad3DViews``: ' + QgsProject.ReadFlag.FlagDontLoad3DViews.__doc__
# --
# monkey patching scoped based enum
QgsProject.FileFormat.Qgz.__doc__ = "Archive file format, supports auxiliary data"
QgsProject.FileFormat.Qgs.__doc__ = "Project saved in a clear text, does not support auxiliary data"
QgsProject.FileFormat.__doc__ = 'Flags which control project read behavior.\n\n.. versionadded:: 3.12\n\n' + '* ``Qgz``: ' + QgsProject.FileFormat.Qgz.__doc__ + '\n' + '* ``Qgs``: ' + QgsProject.FileFormat.Qgs.__doc__
# --
QgsProject.FileFormat.baseClass = QgsProject
# monkey patching scoped based enum
QgsProject.AvoidIntersectionsMode.AllowIntersections.__doc__ = "Overlap with any feature allowed when digitizing new features"
QgsProject.AvoidIntersectionsMode.AvoidIntersectionsCurrentLayer.__doc__ = "Overlap with features from the active layer when digitizing new features not allowed"
QgsProject.AvoidIntersectionsMode.AvoidIntersectionsLayers.__doc__ = "Overlap with features from a specified list of layers when digitizing new features not allowed"
QgsProject.AvoidIntersectionsMode.__doc__ = 'Flags which control how intersections of pre-existing feature are handled when digitizing new features.\n\n.. versionadded:: 3.14\n\n' + '* ``AllowIntersections``: ' + QgsProject.AvoidIntersectionsMode.AllowIntersections.__doc__ + '\n' + '* ``AvoidIntersectionsCurrentLayer``: ' + QgsProject.AvoidIntersectionsMode.AvoidIntersectionsCurrentLayer.__doc__ + '\n' + '* ``AvoidIntersectionsLayers``: ' + QgsProject.AvoidIntersectionsMode.AvoidIntersectionsLayers.__doc__
# --
QgsProject.AvoidIntersectionsMode.baseClass = QgsProject
