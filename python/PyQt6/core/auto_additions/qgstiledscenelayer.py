# The following has been generated automatically from src/core/tiledscene/qgstiledscenelayer.h
try:
    QgsTiledSceneLayer.LayerOptions.__attribute_docs__ = {'transformContext': 'Coordinate transform context', 'loadDefaultStyle': 'Set to ``True`` if the default layer style should be loaded', 'skipCrsValidation': "Controls whether the layer is allowed to have an invalid/unknown CRS.\n\nIf ``True``, then no validation will be performed on the layer's CRS and the layer\nlayer's :py:func:`~QgsTiledSceneLayer.crs` may be :py:func:`~QgsTiledSceneLayer.invalid` (i.e. the layer will have no georeferencing available\nand will be treated as having purely numerical coordinates).\n\nIf ``False`` (the default), the layer's CRS will be validated using :py:func:`QgsCoordinateReferenceSystem.validate()`,\nwhich may cause a blocking, user-facing dialog asking users to manually select the correct CRS for the\nlayer."}
    QgsTiledSceneLayer.LayerOptions.__doc__ = """Setting options for loading tiled scene layers."""
    QgsTiledSceneLayer.LayerOptions.__group__ = ['tiledscene']
except (NameError, AttributeError):
    pass
try:
    QgsTiledSceneLayer.__group__ = ['tiledscene']
except (NameError, AttributeError):
    pass
