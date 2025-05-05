# The following has been generated automatically from src/core/raster/qgsrasterresamplefilter.h
try:
    QgsRasterResampleFilter.__overridden_methods__ = ['clone', 'bandCount', 'dataType', 'setInput', 'block', 'writeXml', 'readXml']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsRasterResampleFilter_setZoomedInResampler = QgsRasterResampleFilter.setZoomedInResampler
    def __QgsRasterResampleFilter_setZoomedInResampler_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsRasterResampleFilter_setZoomedInResampler(self, arg)
    QgsRasterResampleFilter.setZoomedInResampler = _functools.update_wrapper(__QgsRasterResampleFilter_setZoomedInResampler_wrapper, QgsRasterResampleFilter.setZoomedInResampler)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsRasterResampleFilter_setZoomedOutResampler = QgsRasterResampleFilter.setZoomedOutResampler
    def __QgsRasterResampleFilter_setZoomedOutResampler_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsRasterResampleFilter_setZoomedOutResampler(self, arg)
    QgsRasterResampleFilter.setZoomedOutResampler = _functools.update_wrapper(__QgsRasterResampleFilter_setZoomedOutResampler_wrapper, QgsRasterResampleFilter.setZoomedOutResampler)

    QgsRasterResampleFilter.__group__ = ['raster']
except (NameError, AttributeError):
    pass
