# The following has been generated automatically from src/core/effects/qgspainteffectregistry.h
try:
    QgsPaintEffectRegistry.defaultStack = staticmethod(QgsPaintEffectRegistry.defaultStack)
    QgsPaintEffectRegistry.isDefaultStack = staticmethod(QgsPaintEffectRegistry.isDefaultStack)
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsPaintEffectRegistry_addEffectType = QgsPaintEffectRegistry.addEffectType
    def __QgsPaintEffectRegistry_addEffectType_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsPaintEffectRegistry_addEffectType(self, arg)
    QgsPaintEffectRegistry.addEffectType = _functools.update_wrapper(__QgsPaintEffectRegistry_addEffectType_wrapper, QgsPaintEffectRegistry.addEffectType)

    QgsPaintEffectRegistry.__group__ = ['effects']
except (NameError, AttributeError):
    pass
try:
    QgsPaintEffectAbstractMetadata.__virtual_methods__ = ['createWidget']
    QgsPaintEffectAbstractMetadata.__abstract_methods__ = ['createPaintEffect']
    QgsPaintEffectAbstractMetadata.__group__ = ['effects']
except (NameError, AttributeError):
    pass
