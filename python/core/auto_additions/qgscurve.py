# The following has been generated automatically from src/core/geometry/qgscurve.h
try:
    QgsCurve.__virtual_methods__ = ['isClosed', 'isClosed2D', 'isRing', 'asQPolygonF']
    QgsCurve.__abstract_methods__ = ['equals', 'clone', 'startPoint', 'endPoint', 'curveToLine', 'addToPainterPath', 'drawAsPolygon', 'points', 'numPoints', 'sumUpArea', 'pointAt', 'indexOf', 'reversed', 'xAt', 'yAt', 'zAt', 'mAt', 'interpolatePoint', 'curveSubstring', 'scroll']
    QgsCurve.__overridden_methods__ = ['operator==', 'operator!=', 'clone', 'asQPainterPath', 'coordinateSequence', 'nextVertex', 'adjacentVertices', 'vertexNumberFromVertexId', 'boundary', 'asKml', 'segmentize', 'vertexCount', 'ringCount', 'partCount', 'vertexAt', 'toCurveType', 'normalize', 'boundingBox3D', 'isValid', 'clearCache', 'childCount', 'childPoint']
    QgsCurve.__group__ = ['geometry']
except (NameError, AttributeError):
    pass
