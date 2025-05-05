# The following has been generated automatically from src/3d/qgsvectorlayer3drenderer.h
try:
    QgsVectorLayer3DRendererMetadata.__overridden_methods__ = ['createRenderer']
except (NameError, AttributeError):
    pass
try:
    QgsVectorLayer3DRenderer.__overridden_methods__ = ['type', 'clone', 'writeXml', 'readXml']
    import functools as _functools
    __wrapped_QgsVectorLayer3DRenderer_QgsVectorLayer3DRenderer = QgsVectorLayer3DRenderer.QgsVectorLayer3DRenderer
    def __QgsVectorLayer3DRenderer_QgsVectorLayer3DRenderer_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsVectorLayer3DRenderer_QgsVectorLayer3DRenderer(self, arg)
    QgsVectorLayer3DRenderer.QgsVectorLayer3DRenderer = _functools.update_wrapper(__QgsVectorLayer3DRenderer_QgsVectorLayer3DRenderer_wrapper, QgsVectorLayer3DRenderer.QgsVectorLayer3DRenderer)

    import functools as _functools
    __wrapped_QgsVectorLayer3DRenderer_setSymbol = QgsVectorLayer3DRenderer.setSymbol
    def __QgsVectorLayer3DRenderer_setSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsVectorLayer3DRenderer_setSymbol(self, arg)
    QgsVectorLayer3DRenderer.setSymbol = _functools.update_wrapper(__QgsVectorLayer3DRenderer_setSymbol_wrapper, QgsVectorLayer3DRenderer.setSymbol)

except (NameError, AttributeError):
    pass
