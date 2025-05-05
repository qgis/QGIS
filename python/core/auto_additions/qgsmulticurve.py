# The following has been generated automatically from src/core/geometry/qgsmulticurve.h
try:
    QgsMultiCurve.__overridden_methods__ = ['geometryType', 'clone', 'clear', 'toCurveType', 'fromWkt', 'asGml2', 'asGml3', 'addGeometry', 'addGeometries', 'insertGeometry', 'simplifyByDistance', 'boundary', 'createEmptyWithSameType']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsMultiCurve_addGeometry = QgsMultiCurve.addGeometry
    def __QgsMultiCurve_addGeometry_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsMultiCurve_addGeometry(self, arg)
    QgsMultiCurve.addGeometry = _functools.update_wrapper(__QgsMultiCurve_addGeometry_wrapper, QgsMultiCurve.addGeometry)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsMultiCurve_addGeometries = QgsMultiCurve.addGeometries
    def __QgsMultiCurve_addGeometries_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsMultiCurve_addGeometries(self, arg)
    QgsMultiCurve.addGeometries = _functools.update_wrapper(__QgsMultiCurve_addGeometries_wrapper, QgsMultiCurve.addGeometries)

    QgsMultiCurve.__group__ = ['geometry']
except (NameError, AttributeError):
    pass
