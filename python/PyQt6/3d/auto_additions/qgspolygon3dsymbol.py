# The following has been generated automatically from src/3d/symbols/qgspolygon3dsymbol.h
try:
    QgsPolygon3DSymbol.create = staticmethod(QgsPolygon3DSymbol.create)
    QgsPolygon3DSymbol.__overridden_methods__ = ['type', 'clone', 'writeXml', 'readXml', 'compatibleGeometryTypes', 'setDefaultPropertiesFromLayer']
    import functools as _functools
    __wrapped_QgsPolygon3DSymbol_setMaterialSettings = QgsPolygon3DSymbol.setMaterialSettings
    def __QgsPolygon3DSymbol_setMaterialSettings_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsPolygon3DSymbol_setMaterialSettings(self, arg)
    QgsPolygon3DSymbol.setMaterialSettings = _functools.update_wrapper(__QgsPolygon3DSymbol_setMaterialSettings_wrapper, QgsPolygon3DSymbol.setMaterialSettings)

    QgsPolygon3DSymbol.__group__ = ['symbols']
except (NameError, AttributeError):
    pass
