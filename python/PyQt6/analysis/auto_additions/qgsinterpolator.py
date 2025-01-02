# The following has been generated automatically from src/analysis/interpolation/qgsinterpolator.h
QgsInterpolator.SourcePoints = QgsInterpolator.SourceType.SourcePoints
QgsInterpolator.SourceStructureLines = QgsInterpolator.SourceType.SourceStructureLines
QgsInterpolator.SourceBreakLines = QgsInterpolator.SourceType.SourceBreakLines
QgsInterpolator.ValueAttribute = QgsInterpolator.ValueSource.ValueAttribute
QgsInterpolator.ValueZ = QgsInterpolator.ValueSource.ValueZ
QgsInterpolator.ValueM = QgsInterpolator.ValueSource.ValueM
QgsInterpolator.Success = QgsInterpolator.Result.Success
QgsInterpolator.Canceled = QgsInterpolator.Result.Canceled
QgsInterpolator.InvalidSource = QgsInterpolator.Result.InvalidSource
QgsInterpolator.FeatureGeometryError = QgsInterpolator.Result.FeatureGeometryError
try:
    QgsInterpolatorVertexData.__attribute_docs__ = {'x': 'X-coordinate', 'y': 'Y-coordinate', 'z': 'Z-coordinate'}
    QgsInterpolatorVertexData.__doc__ = """Interpolation data for an individual source vertex."""
    QgsInterpolatorVertexData.__group__ = ['interpolation']
except (NameError, AttributeError):
    pass
try:
    QgsInterpolator.LayerData.__attribute_docs__ = {'source': 'Feature source', 'valueSource': 'Source for feature values to interpolate', 'interpolationAttribute': 'Index of feature attribute to use for interpolation', 'sourceType': 'Source type', 'transformContext': 'Coordinate transform context.\n\n.. versionadded:: 3.10.1'}
    QgsInterpolator.LayerData.__doc__ = """A source together with the information about interpolation attribute / z-coordinate interpolation and the type (point, structure line, breakline)"""
    QgsInterpolator.LayerData.__group__ = ['interpolation']
except (NameError, AttributeError):
    pass
try:
    QgsInterpolator.__group__ = ['interpolation']
except (NameError, AttributeError):
    pass
