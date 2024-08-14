# The following has been generated automatically from src/core/qgsmaplayermodel.h
QgsMapLayerModel.ItemDataRole = QgsMapLayerModel.CustomRole
# monkey patching scoped based enum
QgsMapLayerModel.LayerIdRole = QgsMapLayerModel.CustomRole.LayerId
QgsMapLayerModel.ItemDataRole.LayerIdRole = QgsMapLayerModel.CustomRole.LayerId
QgsMapLayerModel.LayerIdRole.is_monkey_patched = True
QgsMapLayerModel.LayerIdRole.__doc__ = "Stores the map layer ID"
QgsMapLayerModel.LayerRole = QgsMapLayerModel.CustomRole.Layer
QgsMapLayerModel.ItemDataRole.LayerRole = QgsMapLayerModel.CustomRole.Layer
QgsMapLayerModel.LayerRole.is_monkey_patched = True
QgsMapLayerModel.LayerRole.__doc__ = "Stores pointer to the map layer itself"
QgsMapLayerModel.EmptyRole = QgsMapLayerModel.CustomRole.Empty
QgsMapLayerModel.ItemDataRole.EmptyRole = QgsMapLayerModel.CustomRole.Empty
QgsMapLayerModel.EmptyRole.is_monkey_patched = True
QgsMapLayerModel.EmptyRole.__doc__ = "True if index corresponds to the empty (not set) value"
QgsMapLayerModel.AdditionalRole = QgsMapLayerModel.CustomRole.Additional
QgsMapLayerModel.ItemDataRole.AdditionalRole = QgsMapLayerModel.CustomRole.Additional
QgsMapLayerModel.AdditionalRole.is_monkey_patched = True
QgsMapLayerModel.AdditionalRole.__doc__ = "True if index corresponds to an additional (non map layer) item"
QgsMapLayerModel.CustomRole.__doc__ = "Custom model roles.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as QgsMapLayerModel.ItemDataRole\n\n.. versionadded:: 3.36\n\n" + '* ``LayerIdRole``: ' + QgsMapLayerModel.CustomRole.LayerId.__doc__ + '\n' + '* ``LayerRole``: ' + QgsMapLayerModel.CustomRole.Layer.__doc__ + '\n' + '* ``EmptyRole``: ' + QgsMapLayerModel.CustomRole.Empty.__doc__ + '\n' + '* ``AdditionalRole``: ' + QgsMapLayerModel.CustomRole.Additional.__doc__
# --
QgsMapLayerModel.CustomRole.baseClass = QgsMapLayerModel
QgsMapLayerModel.iconForLayer = staticmethod(QgsMapLayerModel.iconForLayer)
