# The following has been generated automatically from src/3d/terrain/qgs3dterrainregistry.h
try:
    Qgs3DTerrainAbstractMetadata.__abstract_methods__ = ['createTerrainSettings']
    Qgs3DTerrainAbstractMetadata.__group__ = ['terrain']
except (NameError, AttributeError):
    pass
try:
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_Qgs3DTerrainRegistry_addType = Qgs3DTerrainRegistry.addType
    def __Qgs3DTerrainRegistry_addType_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_Qgs3DTerrainRegistry_addType(self, arg)
    Qgs3DTerrainRegistry.addType = _functools.update_wrapper(__Qgs3DTerrainRegistry_addType_wrapper, Qgs3DTerrainRegistry.addType)

    Qgs3DTerrainRegistry.__group__ = ['terrain']
except (NameError, AttributeError):
    pass
