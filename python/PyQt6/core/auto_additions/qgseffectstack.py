# The following has been generated automatically from src/core/effects/qgseffectstack.h
try:
    QgsEffectStack.create = staticmethod(QgsEffectStack.create)
    QgsEffectStack.__overridden_methods__ = ['type', 'clone', 'saveProperties', 'readProperties', 'properties', 'draw']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsEffectStack_appendEffect = QgsEffectStack.appendEffect
    def __QgsEffectStack_appendEffect_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsEffectStack_appendEffect(self, arg)
    QgsEffectStack.appendEffect = _functools.update_wrapper(__QgsEffectStack_appendEffect_wrapper, QgsEffectStack.appendEffect)

    QgsEffectStack.__group__ = ['effects']
except (NameError, AttributeError):
    pass
