# The following has been generated automatically from src/core/geometry/qgsmultipoint.h
try:
    QgsMultiPoint.__overridden_methods__ = ['geometryType', 'clone', 'toCurveType', 'fromWkt', 'clear', 'asGml2', 'asGml3', 'nCoordinates', 'addGeometry', 'addGeometries', 'insertGeometry', 'boundary', 'vertexNumberFromVertexId', 'segmentLength', 'isValid', 'simplifyByDistance', 'createEmptyWithSameType', 'wktOmitChildType']
    import functools as _functools
    __wrapped_QgsMultiPoint_addGeometry = QgsMultiPoint.addGeometry
    def __QgsMultiPoint_addGeometry_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsMultiPoint_addGeometry(self, arg)
    QgsMultiPoint.addGeometry = _functools.update_wrapper(__QgsMultiPoint_addGeometry_wrapper, QgsMultiPoint.addGeometry)

    import functools as _functools
    __wrapped_QgsMultiPoint_addGeometries = QgsMultiPoint.addGeometries
    def __QgsMultiPoint_addGeometries_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsMultiPoint_addGeometries(self, arg)
    QgsMultiPoint.addGeometries = _functools.update_wrapper(__QgsMultiPoint_addGeometries_wrapper, QgsMultiPoint.addGeometries)

    QgsMultiPoint.__group__ = ['geometry']
except (NameError, AttributeError):
    pass
