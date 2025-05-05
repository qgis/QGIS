# The following has been generated automatically from src/core/layout/qgslayoutitemelevationprofile.h
try:
    QgsLayoutItemElevationProfile.__attribute_docs__ = {'previewRefreshed': "Emitted whenever the item's preview has been refreshed.\n\n.. versionadded:: 3.34\n"}
    QgsLayoutItemElevationProfile.create = staticmethod(QgsLayoutItemElevationProfile.create)
    QgsLayoutItemElevationProfile.__overridden_methods__ = ['type', 'icon', 'refreshDataDefinedProperty', 'itemFlags', 'requiresRasterization', 'containsAdvancedEffects', 'paint', 'refresh', 'invalidateCache', 'draw', 'writePropertiesToElement', 'readPropertiesFromElement']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsLayoutItemElevationProfile_setProfileCurve = QgsLayoutItemElevationProfile.setProfileCurve
    def __QgsLayoutItemElevationProfile_setProfileCurve_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsLayoutItemElevationProfile_setProfileCurve(self, arg)
    QgsLayoutItemElevationProfile.setProfileCurve = _functools.update_wrapper(__QgsLayoutItemElevationProfile_setProfileCurve_wrapper, QgsLayoutItemElevationProfile.setProfileCurve)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsLayoutItemElevationProfile_setSubsectionsSymbol = QgsLayoutItemElevationProfile.setSubsectionsSymbol
    def __QgsLayoutItemElevationProfile_setSubsectionsSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsLayoutItemElevationProfile_setSubsectionsSymbol(self, arg)
    QgsLayoutItemElevationProfile.setSubsectionsSymbol = _functools.update_wrapper(__QgsLayoutItemElevationProfile_setSubsectionsSymbol_wrapper, QgsLayoutItemElevationProfile.setSubsectionsSymbol)

    QgsLayoutItemElevationProfile.__group__ = ['layout']
except (NameError, AttributeError):
    pass
