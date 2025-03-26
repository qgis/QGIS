# The following has been generated automatically from src/core/geometry/qgsabstractgeometry.h
QgsAbstractGeometry.SegmentationToleranceType.baseClass = QgsAbstractGeometry
try:
    QgsAbstractGeometry.__virtual_methods__ = ['compareTo', 'boundingBox', 'nCoordinates', 'closestSegment', 'length', 'perimeter', 'area', 'centroid', 'isEmpty', 'hasCurvedSegments', 'boundingBoxIntersects', 'segmentize', 'convertTo', 'hasChildGeometries', 'childCount', 'childGeometry', 'childPoint', 'calculateBoundingBox', 'calculateBoundingBox3D', 'clearCache']
    QgsAbstractGeometry.__abstract_methods__ = ['operator==', 'operator!=', 'fuzzyEqual', 'fuzzyDistanceEqual', 'clone', 'clear', 'boundingBox3D', 'dimension', 'geometryType', 'boundary', 'normalize', 'fromWkb', 'fromWkt', 'wkbSize', 'asWkb', 'asWkt', 'asGml2', 'asGml3', 'asKml', 'transform', 'draw', 'asQPainterPath', 'vertexNumberFromVertexId', 'nextVertex', 'adjacentVertices', 'coordinateSequence', 'vertexAt', 'closestSegment', 'insertVertex', 'moveVertex', 'deleteVertex', 'segmentLength', 'toCurveType', 'snappedToGrid', 'simplifyByDistance', 'removeDuplicateNodes', 'vertexAngle', 'vertexCount', 'ringCount', 'partCount', 'addZValue', 'addMValue', 'dropZValue', 'dropMValue', 'swapXy', 'isValid', 'createEmptyWithSameType', 'compareToSameClass']
    QgsAbstractGeometry.__group__ = ['geometry']
except (NameError, AttributeError):
    pass
try:
    QgsVertexIterator.__group__ = ['geometry']
except (NameError, AttributeError):
    pass
try:
    QgsGeometryPartIterator.__group__ = ['geometry']
except (NameError, AttributeError):
    pass
try:
    QgsGeometryConstPartIterator.__group__ = ['geometry']
except (NameError, AttributeError):
    pass
