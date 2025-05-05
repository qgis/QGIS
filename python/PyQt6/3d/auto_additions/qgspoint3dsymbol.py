# The following has been generated automatically from src/3d/symbols/qgspoint3dsymbol.h
try:
    QgsPoint3DSymbol.create = staticmethod(QgsPoint3DSymbol.create)
    QgsPoint3DSymbol.shapeFromString = staticmethod(QgsPoint3DSymbol.shapeFromString)
    QgsPoint3DSymbol.shapeToString = staticmethod(QgsPoint3DSymbol.shapeToString)
    QgsPoint3DSymbol.__overridden_methods__ = ['type', 'clone', 'writeXml', 'readXml', 'compatibleGeometryTypes', 'setDefaultPropertiesFromLayer']
    import functools as _functools
    __wrapped_QgsPoint3DSymbol_setMaterialSettings = QgsPoint3DSymbol.setMaterialSettings
    def __QgsPoint3DSymbol_setMaterialSettings_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsPoint3DSymbol_setMaterialSettings(self, arg)
    QgsPoint3DSymbol.setMaterialSettings = _functools.update_wrapper(__QgsPoint3DSymbol_setMaterialSettings_wrapper, QgsPoint3DSymbol.setMaterialSettings)

    QgsPoint3DSymbol.__group__ = ['symbols']
except (NameError, AttributeError):
    pass
