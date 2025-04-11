# The following has been generated automatically from src/analysis/interpolation/qgsinterpolator.h
# monkey patching scoped based enum
QgsInterpolator.SourceType.SourcePoints.__doc__ = "Point source"
QgsInterpolator.SourceType.SourceStructureLines.__doc__ = "Structure lines"
QgsInterpolator.SourceType.SourceBreakLines.__doc__ = "Break lines"
QgsInterpolator.SourceType.__doc__ = """Describes the type of input data

* ``SourcePoints``: Point source
* ``SourceStructureLines``: Structure lines
* ``SourceBreakLines``: Break lines

"""
# --
# monkey patching scoped based enum
QgsInterpolator.ValueSource.ValueAttribute.__doc__ = "Take value from feature's attribute"
QgsInterpolator.ValueSource.ValueZ.__doc__ = "Use feature's geometry Z values for interpolation"
QgsInterpolator.ValueSource.ValueM.__doc__ = "Use feature's geometry M values for interpolation"
QgsInterpolator.ValueSource.__doc__ = """Source for interpolated values from features

* ``ValueAttribute``: Take value from feature's attribute
* ``ValueZ``: Use feature's geometry Z values for interpolation
* ``ValueM``: Use feature's geometry M values for interpolation

"""
# --
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
    QgsInterpolator.LayerData.__annotations__ = {'source': 'QgsFeatureSource', 'valueSource': 'QgsInterpolator.ValueSource', 'interpolationAttribute': int, 'sourceType': 'QgsInterpolator.SourceType', 'transformContext': 'QgsCoordinateTransformContext'}
    QgsInterpolator.LayerData.__doc__ = """A source together with the information about interpolation attribute / z-coordinate interpolation and the type (point, structure line, breakline)"""
    QgsInterpolator.LayerData.__group__ = ['interpolation']
except (NameError, AttributeError):
    pass
try:
    QgsInterpolator.__abstract_methods__ = ['interpolatePoint']
    QgsInterpolator.__group__ = ['interpolation']
except (NameError, AttributeError):
    pass
