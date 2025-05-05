# The following has been generated automatically from src/3d/symbols/qgspointcloud3dsymbol.h
QgsPointCloud3DSymbol.NoRendering = QgsPointCloud3DSymbol.RenderingStyle.NoRendering
QgsPointCloud3DSymbol.SingleColor = QgsPointCloud3DSymbol.RenderingStyle.SingleColor
QgsPointCloud3DSymbol.ColorRamp = QgsPointCloud3DSymbol.RenderingStyle.ColorRamp
QgsPointCloud3DSymbol.RgbRendering = QgsPointCloud3DSymbol.RenderingStyle.RgbRendering
QgsPointCloud3DSymbol.Classification = QgsPointCloud3DSymbol.RenderingStyle.Classification
try:
    QgsPointCloud3DSymbol.__abstract_methods__ = ['symbolType', 'byteStride']
    QgsPointCloud3DSymbol.__overridden_methods__ = ['type', 'copyBaseSettings']
    QgsPointCloud3DSymbol.__group__ = ['symbols']
except (NameError, AttributeError):
    pass
try:
    QgsSingleColorPointCloud3DSymbol.__overridden_methods__ = ['symbolType', 'clone', 'writeXml', 'readXml', 'byteStride']
    QgsSingleColorPointCloud3DSymbol.__group__ = ['symbols']
except (NameError, AttributeError):
    pass
try:
    QgsColorRampPointCloud3DSymbol.__overridden_methods__ = ['clone', 'symbolType', 'writeXml', 'readXml', 'byteStride']
    QgsColorRampPointCloud3DSymbol.__group__ = ['symbols']
except (NameError, AttributeError):
    pass
try:
    QgsRgbPointCloud3DSymbol.__overridden_methods__ = ['symbolType', 'clone', 'writeXml', 'readXml', 'byteStride']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsRgbPointCloud3DSymbol_setRedContrastEnhancement = QgsRgbPointCloud3DSymbol.setRedContrastEnhancement
    def __QgsRgbPointCloud3DSymbol_setRedContrastEnhancement_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsRgbPointCloud3DSymbol_setRedContrastEnhancement(self, arg)
    QgsRgbPointCloud3DSymbol.setRedContrastEnhancement = _functools.update_wrapper(__QgsRgbPointCloud3DSymbol_setRedContrastEnhancement_wrapper, QgsRgbPointCloud3DSymbol.setRedContrastEnhancement)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsRgbPointCloud3DSymbol_setGreenContrastEnhancement = QgsRgbPointCloud3DSymbol.setGreenContrastEnhancement
    def __QgsRgbPointCloud3DSymbol_setGreenContrastEnhancement_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsRgbPointCloud3DSymbol_setGreenContrastEnhancement(self, arg)
    QgsRgbPointCloud3DSymbol.setGreenContrastEnhancement = _functools.update_wrapper(__QgsRgbPointCloud3DSymbol_setGreenContrastEnhancement_wrapper, QgsRgbPointCloud3DSymbol.setGreenContrastEnhancement)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsRgbPointCloud3DSymbol_setBlueContrastEnhancement = QgsRgbPointCloud3DSymbol.setBlueContrastEnhancement
    def __QgsRgbPointCloud3DSymbol_setBlueContrastEnhancement_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsRgbPointCloud3DSymbol_setBlueContrastEnhancement(self, arg)
    QgsRgbPointCloud3DSymbol.setBlueContrastEnhancement = _functools.update_wrapper(__QgsRgbPointCloud3DSymbol_setBlueContrastEnhancement_wrapper, QgsRgbPointCloud3DSymbol.setBlueContrastEnhancement)

    QgsRgbPointCloud3DSymbol.__group__ = ['symbols']
except (NameError, AttributeError):
    pass
try:
    QgsClassificationPointCloud3DSymbol.__overridden_methods__ = ['clone', 'symbolType', 'writeXml', 'readXml', 'byteStride']
    QgsClassificationPointCloud3DSymbol.__group__ = ['symbols']
except (NameError, AttributeError):
    pass
