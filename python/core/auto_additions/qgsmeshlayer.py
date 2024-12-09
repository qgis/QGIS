# The following has been generated automatically from src/core/mesh/qgsmeshlayer.h
try:
    QgsMeshLayer.LayerOptions.__attribute_docs__ = {'transformContext': 'Coordinate transform context', 'loadDefaultStyle': 'Set to ``True`` if the default layer style should be loaded.\n\n.. versionadded:: 3.22', 'skipCrsValidation': "Controls whether the layer is allowed to have an invalid/unknown CRS.\n\nIf ``True``, then no validation will be performed on the layer's CRS and the layer\nlayer's :py:func:`~QgsMeshLayer.crs` may be :py:func:`~QgsMeshLayer.invalid` (i.e. the layer will have no georeferencing available\nand will be treated as having purely numerical coordinates).\n\nIf ``False`` (the default), the layer's CRS will be validated using :py:func:`QgsCoordinateReferenceSystem.validate()`,\nwhich may cause a blocking, user-facing dialog asking users to manually select the correct CRS for the\nlayer.\n\n.. versionadded:: 3.10"}
    QgsMeshLayer.LayerOptions.__doc__ = """Setting options for loading mesh layers."""
    QgsMeshLayer.LayerOptions.__group__ = ['mesh']
except (NameError, AttributeError):
    pass
try:
    QgsMeshLayer.__attribute_docs__ = {'activeScalarDatasetGroupChanged': 'Emitted when active scalar group dataset is changed\n\n.. versionadded:: 3.14\n', 'activeVectorDatasetGroupChanged': 'Emitted when active vector group dataset is changed\n\n.. versionadded:: 3.14\n', 'timeSettingsChanged': 'Emitted when time format is changed\n\n.. versionadded:: 3.8\n', 'reloaded': 'Emitted when the mesh layer is reloaded, see :py:func:`~QgsMeshLayer.reload`\n\n.. versionadded:: 3.28\n'}
    QgsMeshLayer.__signal_arguments__ = {'activeScalarDatasetGroupChanged': ['index: int'], 'activeVectorDatasetGroupChanged': ['index: int']}
    QgsMeshLayer.__group__ = ['mesh']
except (NameError, AttributeError):
    pass
