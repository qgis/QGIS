# The following has been generated automatically from src/core/scalebar/qgsscalebarrendererregistry.h
try:
    import functools as _functools
    __wrapped_QgsScaleBarRendererRegistry_addRenderer = QgsScaleBarRendererRegistry.addRenderer
    def __QgsScaleBarRendererRegistry_addRenderer_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsScaleBarRendererRegistry_addRenderer(self, arg)
    QgsScaleBarRendererRegistry.addRenderer = _functools.update_wrapper(__QgsScaleBarRendererRegistry_addRenderer_wrapper, QgsScaleBarRendererRegistry.addRenderer)

    QgsScaleBarRendererRegistry.__group__ = ['scalebar']
except (NameError, AttributeError):
    pass
