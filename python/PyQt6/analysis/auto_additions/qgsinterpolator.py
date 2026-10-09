# The following has been generated automatically from src/analysis/interpolation/qgsinterpolator.h
# monkey patching scoped based enum
QgsInterpolator.Result.Success.__doc__ = "Operation was successful"
QgsInterpolator.Result.Canceled.__doc__ = "Operation was manually canceled"
QgsInterpolator.Result.InvalidSource.__doc__ = "Operation failed due to invalid source"
QgsInterpolator.Result.FeatureGeometryError.__doc__ = "Operation failed due to invalid feature geometry"
QgsInterpolator.Result.__doc__ = """Result of an interpolation operation

* ``Success``: Operation was successful
* ``Canceled``: Operation was manually canceled
* ``InvalidSource``: Operation failed due to invalid source
* ``FeatureGeometryError``: Operation failed due to invalid feature geometry

"""
# --
try:
    QgsInterpolatorVertexData.__attribute_docs__ = {'x': 'X-coordinate', 'y': 'Y-coordinate', 'z': 'Z-coordinate'}
    QgsInterpolatorVertexData.__annotations__ = {'x': float, 'y': float, 'z': float}
    QgsInterpolatorVertexData.__doc__ = """Interpolation data for an individual source vertex."""
    QgsInterpolatorVertexData.__group__ = ['interpolation']
except (NameError, AttributeError):
    pass
try:
    QgsInterpolator.LayerData.__attribute_docs__ = {'source': 'Feature source', 'valueSource': 'Source for feature values to interpolate', 'interpolationAttribute': 'Index of feature attribute to use for interpolation', 'sourceType': 'Source type', 'transformContext': 'Coordinate transform context.\n\n.. versionadded:: 3.10.1'}
    QgsInterpolator.LayerData.__annotations__ = {'source': 'QgsFeatureSource', 'valueSource': 'Qgis.InterpolationValueSource', 'interpolationAttribute': int, 'sourceType': 'Qgis.InterpolationSourceType', 'transformContext': 'QgsCoordinateTransformContext'}
    QgsInterpolator.LayerData.__doc__ = """A source together with the information about interpolation attribute / z-coordinate interpolation and the type (point, structure line, breakline)"""
    QgsInterpolator.LayerData.__group__ = ['interpolation']
except (NameError, AttributeError):
    pass
try:
    QgsInterpolator.__abstract_methods__ = ['interpolatePoint']
    QgsInterpolator.__group__ = ['interpolation']
except (NameError, AttributeError):
    pass
