# The following has been generated automatically from src/core/textrenderer/qgstextbackgroundsettings.h
QgsTextBackgroundSettings.ShapeRectangle = QgsTextBackgroundSettings.ShapeType.ShapeRectangle
QgsTextBackgroundSettings.ShapeSquare = QgsTextBackgroundSettings.ShapeType.ShapeSquare
QgsTextBackgroundSettings.ShapeEllipse = QgsTextBackgroundSettings.ShapeType.ShapeEllipse
QgsTextBackgroundSettings.ShapeCircle = QgsTextBackgroundSettings.ShapeType.ShapeCircle
QgsTextBackgroundSettings.ShapeSVG = QgsTextBackgroundSettings.ShapeType.ShapeSVG
QgsTextBackgroundSettings.ShapeMarkerSymbol = QgsTextBackgroundSettings.ShapeType.ShapeMarkerSymbol
QgsTextBackgroundSettings.SizeBuffer = QgsTextBackgroundSettings.SizeType.SizeBuffer
QgsTextBackgroundSettings.SizeFixed = QgsTextBackgroundSettings.SizeType.SizeFixed
QgsTextBackgroundSettings.SizePercent = QgsTextBackgroundSettings.SizeType.SizePercent
QgsTextBackgroundSettings.RotationSync = QgsTextBackgroundSettings.RotationType.RotationSync
QgsTextBackgroundSettings.RotationOffset = QgsTextBackgroundSettings.RotationType.RotationOffset
QgsTextBackgroundSettings.RotationFixed = QgsTextBackgroundSettings.RotationType.RotationFixed
try:
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsTextBackgroundSettings_setMarkerSymbol = QgsTextBackgroundSettings.setMarkerSymbol
    def __QgsTextBackgroundSettings_setMarkerSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsTextBackgroundSettings_setMarkerSymbol(self, arg)
    QgsTextBackgroundSettings.setMarkerSymbol = _functools.update_wrapper(__QgsTextBackgroundSettings_setMarkerSymbol_wrapper, QgsTextBackgroundSettings.setMarkerSymbol)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsTextBackgroundSettings_setFillSymbol = QgsTextBackgroundSettings.setFillSymbol
    def __QgsTextBackgroundSettings_setFillSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsTextBackgroundSettings_setFillSymbol(self, arg)
    QgsTextBackgroundSettings.setFillSymbol = _functools.update_wrapper(__QgsTextBackgroundSettings_setFillSymbol_wrapper, QgsTextBackgroundSettings.setFillSymbol)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsTextBackgroundSettings_setPaintEffect = QgsTextBackgroundSettings.setPaintEffect
    def __QgsTextBackgroundSettings_setPaintEffect_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsTextBackgroundSettings_setPaintEffect(self, arg)
    QgsTextBackgroundSettings.setPaintEffect = _functools.update_wrapper(__QgsTextBackgroundSettings_setPaintEffect_wrapper, QgsTextBackgroundSettings.setPaintEffect)

    QgsTextBackgroundSettings.__group__ = ['textrenderer']
except (NameError, AttributeError):
    pass
