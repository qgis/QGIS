# The following has been generated automatically from src/core/raster/qgsrasterrenderer.h
try:
    QgsRasterRenderer.__virtual_methods__ = ['type', 'flags', 'canCreateRasterAttributeTable', 'inputBand', 'setInputBand', 'legendSymbologyItems', 'createLegendNodes', 'usesBands', 'toSld', 'accept']
    QgsRasterRenderer.__abstract_methods__ = ['clone', 'block']
    QgsRasterRenderer.__overridden_methods__ = ['clone', 'bandCount', 'dataType', 'setInput', 'block', 'readXml']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsRasterRenderer_rasterTransparency = QgsRasterRenderer.rasterTransparency
    def __QgsRasterRenderer_rasterTransparency_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsRasterRenderer_rasterTransparency(self, arg)
    QgsRasterRenderer.rasterTransparency = _functools.update_wrapper(__QgsRasterRenderer_rasterTransparency_wrapper, QgsRasterRenderer.rasterTransparency)

    QgsRasterRenderer.__group__ = ['raster']
except (NameError, AttributeError):
    pass
