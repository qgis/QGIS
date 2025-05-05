# The following has been generated automatically from src/core/geometry/qgstriangulatedsurface.h
try:
    QgsTriangulatedSurface.__overridden_methods__ = ['fuzzyEqual', 'fuzzyDistanceEqual', 'operator==', 'operator!=', 'geometryType', 'clone', 'clear', 'fromWkb', 'fromWkt', 'asGml2', 'asGml3', 'asKml', 'normalize', 'snappedToGrid', 'insertVertex', 'deleteVertex', 'addPatch', 'createEmptyWithSameType', 'compareToSameClass']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsTriangulatedSurface_setTriangles = QgsTriangulatedSurface.setTriangles
    def __QgsTriangulatedSurface_setTriangles_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsTriangulatedSurface_setTriangles(self, arg)
    QgsTriangulatedSurface.setTriangles = _functools.update_wrapper(__QgsTriangulatedSurface_setTriangles_wrapper, QgsTriangulatedSurface.setTriangles)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsTriangulatedSurface_addPatch = QgsTriangulatedSurface.addPatch
    def __QgsTriangulatedSurface_addPatch_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsTriangulatedSurface_addPatch(self, arg)
    QgsTriangulatedSurface.addPatch = _functools.update_wrapper(__QgsTriangulatedSurface_addPatch_wrapper, QgsTriangulatedSurface.addPatch)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsTriangulatedSurface_addTriangle = QgsTriangulatedSurface.addTriangle
    def __QgsTriangulatedSurface_addTriangle_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsTriangulatedSurface_addTriangle(self, arg)
    QgsTriangulatedSurface.addTriangle = _functools.update_wrapper(__QgsTriangulatedSurface_addTriangle_wrapper, QgsTriangulatedSurface.addTriangle)

    QgsTriangulatedSurface.__group__ = ['geometry']
except (NameError, AttributeError):
    pass
