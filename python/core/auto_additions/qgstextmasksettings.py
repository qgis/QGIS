# The following has been generated automatically from src/core/textrenderer/qgstextmasksettings.h
try:
    import functools as _functools
    __wrapped_QgsTextMaskSettings_setPaintEffect = QgsTextMaskSettings.setPaintEffect
    def __QgsTextMaskSettings_setPaintEffect_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsTextMaskSettings_setPaintEffect(self, arg)
    QgsTextMaskSettings.setPaintEffect = _functools.update_wrapper(__QgsTextMaskSettings_setPaintEffect_wrapper, QgsTextMaskSettings.setPaintEffect)

    QgsTextMaskSettings.__group__ = ['textrenderer']
except (NameError, AttributeError):
    pass
