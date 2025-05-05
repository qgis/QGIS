# The following has been generated automatically from src/core/elevation/qgsprofilerequest.h
try:
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsProfileRequest_QgsProfileRequest = QgsProfileRequest.QgsProfileRequest
    def __QgsProfileRequest_QgsProfileRequest_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsProfileRequest_QgsProfileRequest(self, arg)
    QgsProfileRequest.QgsProfileRequest = _functools.update_wrapper(__QgsProfileRequest_QgsProfileRequest_wrapper, QgsProfileRequest.QgsProfileRequest)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsProfileRequest_setProfileCurve = QgsProfileRequest.setProfileCurve
    def __QgsProfileRequest_setProfileCurve_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsProfileRequest_setProfileCurve(self, arg)
    QgsProfileRequest.setProfileCurve = _functools.update_wrapper(__QgsProfileRequest_setProfileCurve_wrapper, QgsProfileRequest.setProfileCurve)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsProfileRequest_setTerrainProvider = QgsProfileRequest.setTerrainProvider
    def __QgsProfileRequest_setTerrainProvider_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsProfileRequest_setTerrainProvider(self, arg)
    QgsProfileRequest.setTerrainProvider = _functools.update_wrapper(__QgsProfileRequest_setTerrainProvider_wrapper, QgsProfileRequest.setTerrainProvider)

    QgsProfileRequest.__group__ = ['elevation']
except (NameError, AttributeError):
    pass
