# The following has been generated automatically from src/core/geometry/qgscurvepolygon.h
try:
    QgsCurvePolygon.__virtual_methods__ = ['surfaceToPolygon', 'toPolygon', 'setExteriorRing', 'addInteriorRing']
    QgsCurvePolygon.__overridden_methods__ = ['fuzzyEqual', 'fuzzyDistanceEqual', 'operator==', 'operator!=', 'geometryType', 'dimension', 'clone', 'clear', 'fromWkb', 'fromWkt', 'wkbSize', 'asWkb', 'asWkt', 'asGml2', 'asGml3', 'asKml', 'normalize', 'area', 'perimeter', 'boundary', 'snappedToGrid', 'simplifyByDistance', 'removeDuplicateNodes', 'boundingBoxIntersects', 'asQPainterPath', 'draw', 'transform', 'insertVertex', 'moveVertex', 'deleteVertex', 'coordinateSequence', 'nCoordinates', 'vertexNumberFromVertexId', 'isEmpty', 'closestSegment', 'nextVertex', 'adjacentVertices', 'hasCurvedSegments', 'segmentize', 'vertexAngle', 'vertexCount', 'ringCount', 'partCount', 'vertexAt', 'segmentLength', 'addZValue', 'addMValue', 'dropZValue', 'dropMValue', 'swapXy', 'toCurveType', 'createEmptyWithSameType', 'childCount', 'childGeometry', 'compareToSameClass', 'calculateBoundingBox3D']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsCurvePolygon_setExteriorRing = QgsCurvePolygon.setExteriorRing
    def __QgsCurvePolygon_setExteriorRing_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsCurvePolygon_setExteriorRing(self, arg)
    QgsCurvePolygon.setExteriorRing = _functools.update_wrapper(__QgsCurvePolygon_setExteriorRing_wrapper, QgsCurvePolygon.setExteriorRing)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsCurvePolygon_setInteriorRings = QgsCurvePolygon.setInteriorRings
    def __QgsCurvePolygon_setInteriorRings_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsCurvePolygon_setInteriorRings(self, arg)
    QgsCurvePolygon.setInteriorRings = _functools.update_wrapper(__QgsCurvePolygon_setInteriorRings_wrapper, QgsCurvePolygon.setInteriorRings)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsCurvePolygon_addInteriorRing = QgsCurvePolygon.addInteriorRing
    def __QgsCurvePolygon_addInteriorRing_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsCurvePolygon_addInteriorRing(self, arg)
    QgsCurvePolygon.addInteriorRing = _functools.update_wrapper(__QgsCurvePolygon_addInteriorRing_wrapper, QgsCurvePolygon.addInteriorRing)

    QgsCurvePolygon.__group__ = ['geometry']
except (NameError, AttributeError):
    pass
