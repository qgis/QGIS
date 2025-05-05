# The following has been generated automatically from src/core/geometry/qgsgeometrycollection.h
try:
    QgsGeometryCollection.__virtual_methods__ = ['addGeometry', 'addGeometries', 'insertGeometry', 'removeGeometry', 'wktOmitChildType']
    QgsGeometryCollection.__overridden_methods__ = ['operator==', 'operator!=', 'fuzzyEqual', 'fuzzyDistanceEqual', 'clone', 'isEmpty', 'dimension', 'geometryType', 'clear', 'snappedToGrid', 'removeDuplicateNodes', 'boundary', 'adjacentVertices', 'vertexNumberFromVertexId', 'boundingBoxIntersects', 'normalize', 'transform', 'draw', 'asQPainterPath', 'fromWkb', 'fromWkt', 'wkbSize', 'asWkb', 'asWkt', 'asGml2', 'asGml3', 'asKml', 'boundingBox3D', 'coordinateSequence', 'nCoordinates', 'closestSegment', 'nextVertex', 'insertVertex', 'moveVertex', 'deleteVertex', 'length', 'area', 'perimeter', 'hasCurvedSegments', 'segmentize', 'vertexAngle', 'segmentLength', 'vertexCount', 'ringCount', 'partCount', 'vertexAt', 'isValid', 'addZValue', 'addMValue', 'dropZValue', 'dropMValue', 'swapXy', 'toCurveType', 'simplifiedTypeRef', 'simplifyByDistance', 'createEmptyWithSameType', 'childCount', 'childGeometry', 'compareToSameClass', 'calculateBoundingBox3D', 'clearCache']
    import functools as _functools
    __wrapped_QgsGeometryCollection_addGeometry = QgsGeometryCollection.addGeometry
    def __QgsGeometryCollection_addGeometry_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsGeometryCollection_addGeometry(self, arg)
    QgsGeometryCollection.addGeometry = _functools.update_wrapper(__QgsGeometryCollection_addGeometry_wrapper, QgsGeometryCollection.addGeometry)

    import functools as _functools
    __wrapped_QgsGeometryCollection_addGeometries = QgsGeometryCollection.addGeometries
    def __QgsGeometryCollection_addGeometries_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsGeometryCollection_addGeometries(self, arg)
    QgsGeometryCollection.addGeometries = _functools.update_wrapper(__QgsGeometryCollection_addGeometries_wrapper, QgsGeometryCollection.addGeometries)

    QgsGeometryCollection.__group__ = ['geometry']
except (NameError, AttributeError):
    pass
