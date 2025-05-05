# The following has been generated automatically from src/core/symbology/qgsheatmaprenderer.h
try:
    QgsHeatmapRenderer.create = staticmethod(QgsHeatmapRenderer.create)
    QgsHeatmapRenderer.convertFromRenderer = staticmethod(QgsHeatmapRenderer.convertFromRenderer)
    QgsHeatmapRenderer.__overridden_methods__ = ['clone', 'startRender', 'renderFeature', 'stopRender', 'symbolForFeature', 'symbols', 'dump', 'usedAttributes', 'save', 'accept', 'createLegendNodes', 'modifyRequestExtent']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsHeatmapRenderer_setColorRamp = QgsHeatmapRenderer.setColorRamp
    def __QgsHeatmapRenderer_setColorRamp_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsHeatmapRenderer_setColorRamp(self, arg)
    QgsHeatmapRenderer.setColorRamp = _functools.update_wrapper(__QgsHeatmapRenderer_setColorRamp_wrapper, QgsHeatmapRenderer.setColorRamp)

    QgsHeatmapRenderer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
