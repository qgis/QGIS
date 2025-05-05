# The following has been generated automatically from src/core/annotations/qgsannotationlayer.h
try:
    QgsAnnotationLayer.LayerOptions.__attribute_docs__ = {'transformContext': 'Coordinate transform context'}
    QgsAnnotationLayer.LayerOptions.__annotations__ = {'transformContext': 'QgsCoordinateTransformContext'}
    QgsAnnotationLayer.LayerOptions.__doc__ = """Setting options for loading annotation layers.

.. versionadded:: 3.16"""
    QgsAnnotationLayer.LayerOptions.__group__ = ['annotations']
except (NameError, AttributeError):
    pass
try:
    QgsAnnotationLayer.__overridden_methods__ = ['properties', 'clone', 'createMapRenderer', 'extent', 'setTransformContext', 'readXml', 'writeXml', 'writeSymbology', 'readSymbology', 'writeStyle', 'readStyle', 'isEditable', 'supportsEditing', 'dataProvider', 'htmlMetadata', 'resolveReferences']
    import functools as _functools
    __wrapped_QgsAnnotationLayer_addItem = QgsAnnotationLayer.addItem
    def __QgsAnnotationLayer_addItem_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsAnnotationLayer_addItem(self, arg)
    QgsAnnotationLayer.addItem = _functools.update_wrapper(__QgsAnnotationLayer_addItem_wrapper, QgsAnnotationLayer.addItem)

    import functools as _functools
    __wrapped_QgsAnnotationLayer_setPaintEffect = QgsAnnotationLayer.setPaintEffect
    def __QgsAnnotationLayer_setPaintEffect_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsAnnotationLayer_setPaintEffect(self, arg)
    QgsAnnotationLayer.setPaintEffect = _functools.update_wrapper(__QgsAnnotationLayer_setPaintEffect_wrapper, QgsAnnotationLayer.setPaintEffect)

    QgsAnnotationLayer.__group__ = ['annotations']
except (NameError, AttributeError):
    pass
