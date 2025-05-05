# The following has been generated automatically from src/core/pointcloud/qgspointcloudextentrenderer.h
try:
    QgsPointCloudExtentRenderer.create = staticmethod(QgsPointCloudExtentRenderer.create)
    QgsPointCloudExtentRenderer.defaultFillSymbol = staticmethod(QgsPointCloudExtentRenderer.defaultFillSymbol)
    QgsPointCloudExtentRenderer.__overridden_methods__ = ['type', 'clone', 'renderBlock', 'save', 'startRender', 'stopRender', 'createLegendNodes']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsPointCloudExtentRenderer_QgsPointCloudExtentRenderer = QgsPointCloudExtentRenderer.QgsPointCloudExtentRenderer
    def __QgsPointCloudExtentRenderer_QgsPointCloudExtentRenderer_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsPointCloudExtentRenderer_QgsPointCloudExtentRenderer(self, arg)
    QgsPointCloudExtentRenderer.QgsPointCloudExtentRenderer = _functools.update_wrapper(__QgsPointCloudExtentRenderer_QgsPointCloudExtentRenderer_wrapper, QgsPointCloudExtentRenderer.QgsPointCloudExtentRenderer)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsPointCloudExtentRenderer_setFillSymbol = QgsPointCloudExtentRenderer.setFillSymbol
    def __QgsPointCloudExtentRenderer_setFillSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsPointCloudExtentRenderer_setFillSymbol(self, arg)
    QgsPointCloudExtentRenderer.setFillSymbol = _functools.update_wrapper(__QgsPointCloudExtentRenderer_setFillSymbol_wrapper, QgsPointCloudExtentRenderer.setFillSymbol)

    QgsPointCloudExtentRenderer.__group__ = ['pointcloud']
except (NameError, AttributeError):
    pass
