# The following has been generated automatically from src/core/geometry/qgsmultilinestring.h
try:
    QgsMultiLineString.__overridden_methods__ = ['geometryType', 'clone', 'clear', 'fromWkt', 'asGml2', 'asGml3', 'addGeometry', 'addGeometries', 'insertGeometry', 'simplifyByDistance', 'toCurveType', 'createEmptyWithSameType', 'wktOmitChildType']
    import functools as _functools
    __wrapped_QgsMultiLineString_QgsMultiLineString = QgsMultiLineString.QgsMultiLineString
    def __QgsMultiLineString_QgsMultiLineString_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsMultiLineString_QgsMultiLineString(self, arg)
    QgsMultiLineString.QgsMultiLineString = _functools.update_wrapper(__QgsMultiLineString_QgsMultiLineString_wrapper, QgsMultiLineString.QgsMultiLineString)

    import functools as _functools
    __wrapped_QgsMultiLineString_addGeometry = QgsMultiLineString.addGeometry
    def __QgsMultiLineString_addGeometry_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsMultiLineString_addGeometry(self, arg)
    QgsMultiLineString.addGeometry = _functools.update_wrapper(__QgsMultiLineString_addGeometry_wrapper, QgsMultiLineString.addGeometry)

    import functools as _functools
    __wrapped_QgsMultiLineString_addGeometries = QgsMultiLineString.addGeometries
    def __QgsMultiLineString_addGeometries_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsMultiLineString_addGeometries(self, arg)
    QgsMultiLineString.addGeometries = _functools.update_wrapper(__QgsMultiLineString_addGeometries_wrapper, QgsMultiLineString.addGeometries)

    QgsMultiLineString.__group__ = ['geometry']
except (NameError, AttributeError):
    pass
