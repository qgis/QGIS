# The following has been generated automatically from src/3d/symbols/qgsline3dsymbol.h
try:
    QgsLine3DSymbol.create = staticmethod(QgsLine3DSymbol.create)
    QgsLine3DSymbol.__overridden_methods__ = ['type', 'clone', 'writeXml', 'readXml', 'compatibleGeometryTypes', 'setDefaultPropertiesFromLayer']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsLine3DSymbol_setMaterialSettings = QgsLine3DSymbol.setMaterialSettings
    def __QgsLine3DSymbol_setMaterialSettings_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsLine3DSymbol_setMaterialSettings(self, arg)
    QgsLine3DSymbol.setMaterialSettings = _functools.update_wrapper(__QgsLine3DSymbol_setMaterialSettings_wrapper, QgsLine3DSymbol.setMaterialSettings)

    QgsLine3DSymbol.__group__ = ['symbols']
except (NameError, AttributeError):
    pass
