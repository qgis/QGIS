# The following has been generated automatically from src/core/geometry/qgslinestring.h
try:
    QgsLineString.fromBezierCurve = staticmethod(QgsLineString.fromBezierCurve)
    QgsLineString.fromQPolygonF = staticmethod(QgsLineString.fromQPolygonF)
    QgsLineString.__overridden_methods__ = ['fuzzyEqual', 'fuzzyDistanceEqual', 'equals', 'xAt', 'yAt', 'zAt', 'mAt', 'toCurveType', 'geometryType', 'dimension', 'clone', 'clear', 'isEmpty', 'indexOf', 'isValid', 'snappedToGrid', 'removeDuplicateNodes', 'isClosed', 'isClosed2D', 'boundingBoxIntersects', 'asQPolygonF', 'simplifyByDistance', 'fromWkb', 'fromWkt', 'wkbSize', 'asWkb', 'asWkt', 'asGml2', 'asGml3', 'asKml', 'length', 'startPoint', 'endPoint', 'curveToLine', 'numPoints', 'nCoordinates', 'points', 'draw', 'transform', 'addToPainterPath', 'drawAsPolygon', 'insertVertex', 'moveVertex', 'deleteVertex', 'reversed', 'interpolatePoint', 'curveSubstring', 'closestSegment', 'pointAt', 'centroid', 'sumUpArea', 'vertexAngle', 'segmentLength', 'addZValue', 'addMValue', 'dropZValue', 'dropMValue', 'swapXy', 'convertTo', 'scroll', 'createEmptyWithSameType', 'calculateBoundingBox3D', 'compareToSameClass']
    QgsLineString.__group__ = ['geometry']
except (NameError, AttributeError):
    pass
