# The following has been generated automatically from src/core/vectortile/qgsvectortilelayer.h
try:
    QgsVectorTileLayer.LayerOptions.__attribute_docs__ = {'transformContext': 'Coordinate transform context'}
    QgsVectorTileLayer.LayerOptions.__annotations__ = {'transformContext': 'QgsCoordinateTransformContext'}
    QgsVectorTileLayer.LayerOptions.__doc__ = """Setting options for loading vector tile layers.

.. versionadded:: 3.22"""
    QgsVectorTileLayer.LayerOptions.__group__ = ['vectortile']
except (NameError, AttributeError):
    pass
try:
    QgsVectorTileLayer.__attribute_docs__ = {'selectionChanged': 'Emitted whenever the selected features in the layer are changed.\n\n.. versionadded:: 3.28\n'}
    QgsVectorTileLayer.__virtual_methods__ = ['encodedSource', 'decodedSource']
    QgsVectorTileLayer.__overridden_methods__ = ['clone', 'dataProvider', 'createMapRenderer', 'readXml', 'writeXml', 'readSymbology', 'writeSymbology', 'setTransformContext', 'loadDefaultStyle', 'properties', 'loadDefaultMetadata', 'htmlMetadata']
    import functools as _functools
    __wrapped_QgsVectorTileLayer_setRenderer = QgsVectorTileLayer.setRenderer
    def __QgsVectorTileLayer_setRenderer_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsVectorTileLayer_setRenderer(self, arg)
    QgsVectorTileLayer.setRenderer = _functools.update_wrapper(__QgsVectorTileLayer_setRenderer_wrapper, QgsVectorTileLayer.setRenderer)

    import functools as _functools
    __wrapped_QgsVectorTileLayer_setLabeling = QgsVectorTileLayer.setLabeling
    def __QgsVectorTileLayer_setLabeling_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsVectorTileLayer_setLabeling(self, arg)
    QgsVectorTileLayer.setLabeling = _functools.update_wrapper(__QgsVectorTileLayer_setLabeling_wrapper, QgsVectorTileLayer.setLabeling)

    QgsVectorTileLayer.__group__ = ['vectortile']
except (NameError, AttributeError):
    pass
