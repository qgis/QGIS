# The following has been generated automatically from src/3d/materials/qgsmaterialregistry.h
try:
    QgsMaterialSettingsAbstractMetadata.__abstract_methods__ = ['create', 'supportsTechnique']
    QgsMaterialSettingsAbstractMetadata.__group__ = ['materials']
except (NameError, AttributeError):
    pass
try:
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsMaterialRegistry_addMaterialSettingsType = QgsMaterialRegistry.addMaterialSettingsType
    def __QgsMaterialRegistry_addMaterialSettingsType_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsMaterialRegistry_addMaterialSettingsType(self, arg)
    QgsMaterialRegistry.addMaterialSettingsType = _functools.update_wrapper(__QgsMaterialRegistry_addMaterialSettingsType_wrapper, QgsMaterialRegistry.addMaterialSettingsType)

    QgsMaterialRegistry.__group__ = ['materials']
except (NameError, AttributeError):
    pass
