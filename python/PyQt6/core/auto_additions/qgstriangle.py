# The following has been generated automatically from src/core/geometry/qgstriangle.h
try:
    QgsTriangle.__overridden_methods__ = ['operator==', 'operator!=', 'geometryType', 'clone', 'clear', 'fromWkb', 'fromWkt', 'asGml3', 'surfaceToPolygon', 'toCurveType', 'addInteriorRing', 'deleteVertex', 'insertVertex', 'moveVertex', 'setExteriorRing', 'boundary', 'vertexAt', 'createEmptyWithSameType']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsTriangle_addInteriorRing = QgsTriangle.addInteriorRing
    def __QgsTriangle_addInteriorRing_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsTriangle_addInteriorRing(self, arg)
    QgsTriangle.addInteriorRing = _functools.update_wrapper(__QgsTriangle_addInteriorRing_wrapper, QgsTriangle.addInteriorRing)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsTriangle_boundary = QgsTriangle.boundary
    def __QgsTriangle_boundary_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsTriangle_boundary(self, arg)
    QgsTriangle.boundary = _functools.update_wrapper(__QgsTriangle_boundary_wrapper, QgsTriangle.boundary)

    QgsTriangle.__group__ = ['geometry']
except (NameError, AttributeError):
    pass
