# The following has been generated automatically from src/core/tiledscene/qgstiledscenewireframerenderer.h
try:
    QgsTiledSceneWireframeRenderer.create = staticmethod(QgsTiledSceneWireframeRenderer.create)
    QgsTiledSceneWireframeRenderer.createDefaultFillSymbol = staticmethod(QgsTiledSceneWireframeRenderer.createDefaultFillSymbol)
    QgsTiledSceneWireframeRenderer.createDefaultLineSymbol = staticmethod(QgsTiledSceneWireframeRenderer.createDefaultLineSymbol)
    QgsTiledSceneWireframeRenderer.__overridden_methods__ = ['type', 'clone', 'save', 'renderTriangle', 'renderLine', 'startRender', 'stopRender', 'flags']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsTiledSceneWireframeRenderer_setFillSymbol = QgsTiledSceneWireframeRenderer.setFillSymbol
    def __QgsTiledSceneWireframeRenderer_setFillSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsTiledSceneWireframeRenderer_setFillSymbol(self, arg)
    QgsTiledSceneWireframeRenderer.setFillSymbol = _functools.update_wrapper(__QgsTiledSceneWireframeRenderer_setFillSymbol_wrapper, QgsTiledSceneWireframeRenderer.setFillSymbol)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsTiledSceneWireframeRenderer_setLineSymbol = QgsTiledSceneWireframeRenderer.setLineSymbol
    def __QgsTiledSceneWireframeRenderer_setLineSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsTiledSceneWireframeRenderer_setLineSymbol(self, arg)
    QgsTiledSceneWireframeRenderer.setLineSymbol = _functools.update_wrapper(__QgsTiledSceneWireframeRenderer_setLineSymbol_wrapper, QgsTiledSceneWireframeRenderer.setLineSymbol)

    QgsTiledSceneWireframeRenderer.__group__ = ['tiledscene']
except (NameError, AttributeError):
    pass
