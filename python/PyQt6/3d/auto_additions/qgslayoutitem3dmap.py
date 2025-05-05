# The following has been generated automatically from src/3d/qgslayoutitem3dmap.h
try:
    QgsLayoutItem3DMap.create = staticmethod(QgsLayoutItem3DMap.create)
    QgsLayoutItem3DMap.__overridden_methods__ = ['type', 'icon', 'displayName', 'finalizeRestoreFromXml', 'refresh', 'draw', 'writePropertiesToElement', 'readPropertiesFromElement']
    import functools as _functools
    __wrapped_QgsLayoutItem3DMap_setMapSettings = QgsLayoutItem3DMap.setMapSettings
    def __QgsLayoutItem3DMap_setMapSettings_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsLayoutItem3DMap_setMapSettings(self, arg)
    QgsLayoutItem3DMap.setMapSettings = _functools.update_wrapper(__QgsLayoutItem3DMap_setMapSettings_wrapper, QgsLayoutItem3DMap.setMapSettings)

except (NameError, AttributeError):
    pass
