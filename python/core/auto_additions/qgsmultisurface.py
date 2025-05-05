# The following has been generated automatically from src/core/geometry/qgsmultisurface.h
try:
    QgsMultiSurface.__overridden_methods__ = ['geometryType', 'clear', 'clone', 'toCurveType', 'fromWkt', 'asGml2', 'asGml3', 'addGeometry', 'addGeometries', 'insertGeometry', 'boundary', 'simplifyByDistance', 'createEmptyWithSameType']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsMultiSurface_addGeometry = QgsMultiSurface.addGeometry
    def __QgsMultiSurface_addGeometry_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsMultiSurface_addGeometry(self, arg)
    QgsMultiSurface.addGeometry = _functools.update_wrapper(__QgsMultiSurface_addGeometry_wrapper, QgsMultiSurface.addGeometry)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsMultiSurface_addGeometries = QgsMultiSurface.addGeometries
    def __QgsMultiSurface_addGeometries_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsMultiSurface_addGeometries(self, arg)
    QgsMultiSurface.addGeometries = _functools.update_wrapper(__QgsMultiSurface_addGeometries_wrapper, QgsMultiSurface.addGeometries)

    QgsMultiSurface.__group__ = ['geometry']
except (NameError, AttributeError):
    pass
