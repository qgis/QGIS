# The following has been generated automatically from src/core/geometry/qgspolyhedralsurface.h
try:
    QgsPolyhedralSurface.__virtual_methods__ = ['setPatches', 'addPatch']
    QgsPolyhedralSurface.__overridden_methods__ = ['fuzzyEqual', 'fuzzyDistanceEqual', 'operator==', 'operator!=', 'geometryType', 'dimension', 'clone', 'clear', 'fromWkb', 'fromWkt', 'isValid', 'wkbSize', 'asWkb', 'asWkt', 'asGml2', 'asGml3', 'asKml', 'normalize', 'area', 'perimeter', 'boundary', 'snappedToGrid', 'simplifyByDistance', 'removeDuplicateNodes', 'boundingBoxIntersects', 'asQPainterPath', 'draw', 'transform', 'insertVertex', 'moveVertex', 'deleteVertex', 'coordinateSequence', 'nCoordinates', 'vertexNumberFromVertexId', 'isEmpty', 'closestSegment', 'nextVertex', 'adjacentVertices', 'hasCurvedSegments', 'segmentize', 'vertexAngle', 'vertexCount', 'ringCount', 'partCount', 'vertexAt', 'segmentLength', 'addZValue', 'addMValue', 'dropZValue', 'dropMValue', 'swapXy', 'toCurveType', 'createEmptyWithSameType', 'childCount', 'childGeometry', 'compareToSameClass', 'calculateBoundingBox3D']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsPolyhedralSurface_setPatches = QgsPolyhedralSurface.setPatches
    def __QgsPolyhedralSurface_setPatches_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsPolyhedralSurface_setPatches(self, arg)
    QgsPolyhedralSurface.setPatches = _functools.update_wrapper(__QgsPolyhedralSurface_setPatches_wrapper, QgsPolyhedralSurface.setPatches)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsPolyhedralSurface_addPatch = QgsPolyhedralSurface.addPatch
    def __QgsPolyhedralSurface_addPatch_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsPolyhedralSurface_addPatch(self, arg)
    QgsPolyhedralSurface.addPatch = _functools.update_wrapper(__QgsPolyhedralSurface_addPatch_wrapper, QgsPolyhedralSurface.addPatch)

    QgsPolyhedralSurface.__group__ = ['geometry']
except (NameError, AttributeError):
    pass
