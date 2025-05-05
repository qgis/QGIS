# The following has been generated automatically from src/core/raster/qgspalettedrasterrenderer.h
try:
    QgsPalettedRasterRenderer.Class.__attribute_docs__ = {'value': 'Value', 'color': 'Color to render value', 'label': 'Label for value'}
    QgsPalettedRasterRenderer.Class.__annotations__ = {'value': float, 'color': 'QColor', 'label': str}
    QgsPalettedRasterRenderer.Class.__doc__ = """Properties of a single value class"""
    QgsPalettedRasterRenderer.Class.__group__ = ['raster']
except (NameError, AttributeError):
    pass
try:
    QgsPalettedRasterRenderer.MultiValueClass.__attribute_docs__ = {'values': 'Values', 'color': 'Color to render values', 'label': 'Label for values'}
    QgsPalettedRasterRenderer.MultiValueClass.__annotations__ = {'values': 'List[object]', 'color': 'QColor', 'label': str}
    QgsPalettedRasterRenderer.MultiValueClass.__group__ = ['raster']
except (NameError, AttributeError):
    pass
try:
    QgsPalettedRasterRenderer.create = staticmethod(QgsPalettedRasterRenderer.create)
    QgsPalettedRasterRenderer.colorTableToClassData = staticmethod(QgsPalettedRasterRenderer.colorTableToClassData)
    QgsPalettedRasterRenderer.rasterAttributeTableToClassData = staticmethod(QgsPalettedRasterRenderer.rasterAttributeTableToClassData)
    QgsPalettedRasterRenderer.classDataFromString = staticmethod(QgsPalettedRasterRenderer.classDataFromString)
    QgsPalettedRasterRenderer.classDataFromFile = staticmethod(QgsPalettedRasterRenderer.classDataFromFile)
    QgsPalettedRasterRenderer.classDataToString = staticmethod(QgsPalettedRasterRenderer.classDataToString)
    QgsPalettedRasterRenderer.classDataFromRaster = staticmethod(QgsPalettedRasterRenderer.classDataFromRaster)
    QgsPalettedRasterRenderer.__overridden_methods__ = ['clone', 'flags', 'block', 'canCreateRasterAttributeTable', 'inputBand', 'setInputBand', 'writeXml', 'legendSymbologyItems', 'createLegendNodes', 'usesBands', 'toSld', 'accept']
    import functools as _functools
    __wrapped_QgsPalettedRasterRenderer_setSourceColorRamp = QgsPalettedRasterRenderer.setSourceColorRamp
    def __QgsPalettedRasterRenderer_setSourceColorRamp_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsPalettedRasterRenderer_setSourceColorRamp(self, arg)
    QgsPalettedRasterRenderer.setSourceColorRamp = _functools.update_wrapper(__QgsPalettedRasterRenderer_setSourceColorRamp_wrapper, QgsPalettedRasterRenderer.setSourceColorRamp)

    QgsPalettedRasterRenderer.__group__ = ['raster']
except (NameError, AttributeError):
    pass
