# The following has been generated automatically from src/core/geometry/qgsmultipolygon.h
try:
    QgsMultiPolygon.__overridden_methods__ = ['geometryType', 'clear', 'clone', 'fromWkt', 'asGml2', 'asGml3', 'addGeometry', 'addGeometries', 'insertGeometry', 'simplifyByDistance', 'toCurveType', 'boundary', 'createEmptyWithSameType', 'wktOmitChildType']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsMultiPolygon_QgsMultiPolygon = QgsMultiPolygon.QgsMultiPolygon
    def __QgsMultiPolygon_QgsMultiPolygon_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsMultiPolygon_QgsMultiPolygon(self, arg)
    QgsMultiPolygon.QgsMultiPolygon = _functools.update_wrapper(__QgsMultiPolygon_QgsMultiPolygon_wrapper, QgsMultiPolygon.QgsMultiPolygon)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsMultiPolygon_addGeometry = QgsMultiPolygon.addGeometry
    def __QgsMultiPolygon_addGeometry_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsMultiPolygon_addGeometry(self, arg)
    QgsMultiPolygon.addGeometry = _functools.update_wrapper(__QgsMultiPolygon_addGeometry_wrapper, QgsMultiPolygon.addGeometry)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsMultiPolygon_addGeometries = QgsMultiPolygon.addGeometries
    def __QgsMultiPolygon_addGeometries_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsMultiPolygon_addGeometries(self, arg)
    QgsMultiPolygon.addGeometries = _functools.update_wrapper(__QgsMultiPolygon_addGeometries_wrapper, QgsMultiPolygon.addGeometries)

    QgsMultiPolygon.__group__ = ['geometry']
except (NameError, AttributeError):
    pass
