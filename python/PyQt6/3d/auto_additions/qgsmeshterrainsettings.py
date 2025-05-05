# The following has been generated automatically from src/3d/terrain/qgsmeshterrainsettings.h
try:
    QgsMeshTerrainSettings.create = staticmethod(QgsMeshTerrainSettings.create)
    QgsMeshTerrainSettings.__overridden_methods__ = ['clone', 'type', 'readXml', 'writeXml', 'resolveReferences', 'equals']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsMeshTerrainSettings_setSymbol = QgsMeshTerrainSettings.setSymbol
    def __QgsMeshTerrainSettings_setSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsMeshTerrainSettings_setSymbol(self, arg)
    QgsMeshTerrainSettings.setSymbol = _functools.update_wrapper(__QgsMeshTerrainSettings_setSymbol_wrapper, QgsMeshTerrainSettings.setSymbol)

    QgsMeshTerrainSettings.__group__ = ['terrain']
except (NameError, AttributeError):
    pass
