# The following has been generated automatically from src/core/tiledscene/qgstiledscenetexturerenderer.h
try:
    QgsTiledSceneTextureRenderer.create = staticmethod(QgsTiledSceneTextureRenderer.create)
    QgsTiledSceneTextureRenderer.createDefaultFillSymbol = staticmethod(QgsTiledSceneTextureRenderer.createDefaultFillSymbol)
    QgsTiledSceneTextureRenderer.__overridden_methods__ = ['type', 'clone', 'save', 'flags', 'renderTriangle', 'renderLine', 'startRender', 'stopRender']
    import functools as _functools
    __wrapped_QgsTiledSceneTextureRenderer_setFillSymbol = QgsTiledSceneTextureRenderer.setFillSymbol
    def __QgsTiledSceneTextureRenderer_setFillSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsTiledSceneTextureRenderer_setFillSymbol(self, arg)
    QgsTiledSceneTextureRenderer.setFillSymbol = _functools.update_wrapper(__QgsTiledSceneTextureRenderer_setFillSymbol_wrapper, QgsTiledSceneTextureRenderer.setFillSymbol)

    QgsTiledSceneTextureRenderer.__group__ = ['tiledscene']
except (NameError, AttributeError):
    pass
