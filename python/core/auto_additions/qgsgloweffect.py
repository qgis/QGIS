# The following has been generated automatically from src/core/effects/qgsgloweffect.h
try:
    QgsOuterGlowEffect.create = staticmethod(QgsOuterGlowEffect.create)
    QgsOuterGlowEffect.__overridden_methods__ = ['type', 'clone', 'shadeExterior']
    QgsOuterGlowEffect.__group__ = ['effects']
except (NameError, AttributeError):
    pass
try:
    QgsInnerGlowEffect.create = staticmethod(QgsInnerGlowEffect.create)
    QgsInnerGlowEffect.__overridden_methods__ = ['type', 'clone', 'shadeExterior']
    QgsInnerGlowEffect.__group__ = ['effects']
except (NameError, AttributeError):
    pass
try:
    QgsGlowEffect.__abstract_methods__ = ['shadeExterior']
    QgsGlowEffect.__overridden_methods__ = ['properties', 'readProperties', 'boundingRect', 'draw']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsGlowEffect_setRamp = QgsGlowEffect.setRamp
    def __QgsGlowEffect_setRamp_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsGlowEffect_setRamp(self, arg)
    QgsGlowEffect.setRamp = _functools.update_wrapper(__QgsGlowEffect_setRamp_wrapper, QgsGlowEffect.setRamp)

    QgsGlowEffect.__group__ = ['effects']
except (NameError, AttributeError):
    pass
