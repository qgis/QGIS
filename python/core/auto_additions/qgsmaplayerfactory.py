# The following has been generated automatically from src/core/qgsmaplayerfactory.h
try:
    QgsMapLayerFactory.LayerOptions.__attribute_docs__ = {'transformContext': 'Transform context', 'loadDefaultStyle': 'Set to ``True`` if the default layer style should be loaded', 'loadAllStoredStyles': "Controls whether the stored styles will be all loaded.\n\nIf ``True`` and the layer's provider supports style stored in the\ndata source all the available styles will be loaded in addition\nto the default one.\n\nIf ``False`` (the default), the layer's provider will only load\nthe default style.\n\n.. versionadded:: 3.30"}
    QgsMapLayerFactory.LayerOptions.__doc__ = """Setting options for loading layers.

.. versionadded:: 3.22"""
except (NameError, AttributeError):
    pass
try:
    QgsMapLayerFactory.typeFromString = staticmethod(QgsMapLayerFactory.typeFromString)
    QgsMapLayerFactory.typeToString = staticmethod(QgsMapLayerFactory.typeToString)
    QgsMapLayerFactory.createLayer = staticmethod(QgsMapLayerFactory.createLayer)
except (NameError, AttributeError):
    pass
