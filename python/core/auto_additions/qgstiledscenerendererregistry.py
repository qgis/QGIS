# The following has been generated automatically from src/core/tiledscene/qgstiledscenerendererregistry.h
try:
    QgsTiledSceneRendererRegistry.defaultRenderer = staticmethod(QgsTiledSceneRendererRegistry.defaultRenderer)
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsTiledSceneRendererRegistry_addRenderer = QgsTiledSceneRendererRegistry.addRenderer
    def __QgsTiledSceneRendererRegistry_addRenderer_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsTiledSceneRendererRegistry_addRenderer(self, arg)
    QgsTiledSceneRendererRegistry.addRenderer = _functools.update_wrapper(__QgsTiledSceneRendererRegistry_addRenderer_wrapper, QgsTiledSceneRendererRegistry.addRenderer)

    QgsTiledSceneRendererRegistry.__group__ = ['tiledscene']
except (NameError, AttributeError):
    pass
try:
    QgsTiledSceneRendererAbstractMetadata.__abstract_methods__ = ['createRenderer']
    QgsTiledSceneRendererAbstractMetadata.__group__ = ['tiledscene']
except (NameError, AttributeError):
    pass
try:
    QgsTiledSceneRendererMetadata.__overridden_methods__ = ['createRenderer']
    QgsTiledSceneRendererMetadata.__group__ = ['tiledscene']
except (NameError, AttributeError):
    pass
