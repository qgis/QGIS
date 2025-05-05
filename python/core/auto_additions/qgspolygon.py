# The following has been generated automatically from src/core/geometry/qgspolygon.h
try:
    QgsPolygon.__overridden_methods__ = ['geometryType', 'clone', 'clear', 'fromWkb', 'wkbSize', 'asWkb', 'asWkt', 'surfaceToPolygon', 'toCurveType', 'addInteriorRing', 'setExteriorRing', 'boundary', 'createEmptyWithSameType']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsPolygon_addInteriorRing = QgsPolygon.addInteriorRing
    def __QgsPolygon_addInteriorRing_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsPolygon_addInteriorRing(self, arg)
    QgsPolygon.addInteriorRing = _functools.update_wrapper(__QgsPolygon_addInteriorRing_wrapper, QgsPolygon.addInteriorRing)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsPolygon_boundary = QgsPolygon.boundary
    def __QgsPolygon_boundary_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsPolygon_boundary(self, arg)
    QgsPolygon.boundary = _functools.update_wrapper(__QgsPolygon_boundary_wrapper, QgsPolygon.boundary)

    QgsPolygon.__group__ = ['geometry']
except (NameError, AttributeError):
    pass
