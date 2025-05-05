# The following has been generated automatically from src/core/textrenderer/qgstextbuffersettings.h
try:
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsTextBufferSettings_setPaintEffect = QgsTextBufferSettings.setPaintEffect
    def __QgsTextBufferSettings_setPaintEffect_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsTextBufferSettings_setPaintEffect(self, arg)
    QgsTextBufferSettings.setPaintEffect = _functools.update_wrapper(__QgsTextBufferSettings_setPaintEffect_wrapper, QgsTextBufferSettings.setPaintEffect)

    QgsTextBufferSettings.__group__ = ['textrenderer']
except (NameError, AttributeError):
    pass
