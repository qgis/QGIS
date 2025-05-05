# The following has been generated automatically from src/core/vectortile/qgsvectortilebasicrenderer.h
try:
    QgsVectorTileBasicRenderer.simpleStyle = staticmethod(QgsVectorTileBasicRenderer.simpleStyle)
    QgsVectorTileBasicRenderer.simpleStyleWithRandomColors = staticmethod(QgsVectorTileBasicRenderer.simpleStyleWithRandomColors)
    QgsVectorTileBasicRenderer.__overridden_methods__ = ['type', 'clone', 'startRender', 'requiredLayers', 'stopRender', 'renderBackground', 'renderTile', 'renderSelectedFeatures', 'willRenderFeature', 'writeXml', 'readXml']
    QgsVectorTileBasicRenderer.__group__ = ['vectortile']
except (NameError, AttributeError):
    pass
try:
    import functools as _functools
    __wrapped_QgsVectorTileBasicRendererStyle_setSymbol = QgsVectorTileBasicRendererStyle.setSymbol
    def __QgsVectorTileBasicRendererStyle_setSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsVectorTileBasicRendererStyle_setSymbol(self, arg)
    QgsVectorTileBasicRendererStyle.setSymbol = _functools.update_wrapper(__QgsVectorTileBasicRendererStyle_setSymbol_wrapper, QgsVectorTileBasicRendererStyle.setSymbol)

    QgsVectorTileBasicRendererStyle.__group__ = ['vectortile']
except (NameError, AttributeError):
    pass
