# The following has been generated automatically from src/core/./3d/qgs3dsymbolregistry.h
try:
    Qgs3DSymbolAbstractMetadata.__abstract_methods__ = ['create']
    Qgs3DSymbolAbstractMetadata.__group__ = ['3d']
except (NameError, AttributeError):
    pass
try:
    import functools as _functools
    __wrapped_Qgs3DSymbolRegistry_addSymbolType = Qgs3DSymbolRegistry.addSymbolType
    def __Qgs3DSymbolRegistry_addSymbolType_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_Qgs3DSymbolRegistry_addSymbolType(self, arg)
    Qgs3DSymbolRegistry.addSymbolType = _functools.update_wrapper(__Qgs3DSymbolRegistry_addSymbolType_wrapper, Qgs3DSymbolRegistry.addSymbolType)

    Qgs3DSymbolRegistry.__group__ = ['3d']
except (NameError, AttributeError):
    pass
