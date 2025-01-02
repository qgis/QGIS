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
QgsMapLayerModel.CustomRole.__doc__ = """Custom model roles.

.. note::

   Prior to QGIS 3.36 this was available as QgsMapLayerModel.ItemDataRole

.. versionadded:: 3.36

* ``LayerId``: Stores the map layer ID

  Available as ``QgsMapLayerModel.LayerIdRole`` in older QGIS releases.

* ``Layer``: Stores pointer to the map layer itself

  Available as ``QgsMapLayerModel.LayerRole`` in older QGIS releases.

* ``Empty``: True if index corresponds to the empty (not set) value

  Available as ``QgsMapLayerModel.EmptyRole`` in older QGIS releases.

* ``Additional``: True if index corresponds to an additional (non map layer) item

  Available as ``QgsMapLayerModel.AdditionalRole`` in older QGIS releases.


"""
# --
QgsMapLayerModel.CustomRole.baseClass = QgsMapLayerModel
try:
    QgsMapLayerModel.iconForLayer = staticmethod(QgsMapLayerModel.iconForLayer)
except (NameError, AttributeError):
    pass
