# The following has been generated automatically from src/core/qgsgrouplayer.h
try:
    QgsGroupLayer.LayerOptions.__attribute_docs__ = {'transformContext': 'Coordinate transform context'}
    QgsGroupLayer.LayerOptions.__annotations__ = {'transformContext': 'QgsCoordinateTransformContext'}
    QgsGroupLayer.LayerOptions.__doc__ = """Setting options for loading group layers.

.. versionadded:: 3.16"""
except (NameError, AttributeError):
    pass
try:
    QgsGroupLayer.__overridden_methods__ = ['clone', 'createMapRenderer', 'extent', 'setTransformContext', 'readXml', 'writeXml', 'writeSymbology', 'readSymbology', 'dataProvider', 'htmlMetadata', 'resolveReferences']
    import functools as _functools
    __wrapped_QgsGroupLayer_setPaintEffect = QgsGroupLayer.setPaintEffect
    def __QgsGroupLayer_setPaintEffect_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsGroupLayer_setPaintEffect(self, arg)
    QgsGroupLayer.setPaintEffect = _functools.update_wrapper(__QgsGroupLayer_setPaintEffect_wrapper, QgsGroupLayer.setPaintEffect)

except (NameError, AttributeError):
    pass
