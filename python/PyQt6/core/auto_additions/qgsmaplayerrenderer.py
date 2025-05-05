# The following has been generated automatically from src/core/qgsmaplayerrenderer.h
try:
    QgsMapLayerRenderer.__virtual_methods__ = ['forceRasterRender', 'flags', 'feedback']
    QgsMapLayerRenderer.__abstract_methods__ = ['render']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsMapLayerRenderer_appendRenderedItemDetails = QgsMapLayerRenderer.appendRenderedItemDetails
    def __QgsMapLayerRenderer_appendRenderedItemDetails_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsMapLayerRenderer_appendRenderedItemDetails(self, arg)
    QgsMapLayerRenderer.appendRenderedItemDetails = _functools.update_wrapper(__QgsMapLayerRenderer_appendRenderedItemDetails_wrapper, QgsMapLayerRenderer.appendRenderedItemDetails)

except (NameError, AttributeError):
    pass
