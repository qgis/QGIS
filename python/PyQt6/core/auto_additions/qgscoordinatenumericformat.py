# The following has been generated automatically from src/core/numericformats/qgscoordinatenumericformat.h
# monkey patching scoped based enum
QgsGeographicCoordinateNumericFormat.AngleFormat.DegreesMinutesSeconds.__doc__ = "Degrees, minutes and seconds, eg 30 degrees 45'30"
QgsGeographicCoordinateNumericFormat.AngleFormat.DegreesMinutes.__doc__ = "Degrees and decimal minutes, eg 30 degrees 45.55'"
QgsGeographicCoordinateNumericFormat.AngleFormat.DecimalDegrees.__doc__ = "Decimal degrees, eg 30.7555 degrees"
QgsGeographicCoordinateNumericFormat.AngleFormat.__doc__ = """Angle format options.

* ``DegreesMinutesSeconds``: Degrees, minutes and seconds, eg 30 degrees 45'30
* ``DegreesMinutes``: Degrees and decimal minutes, eg 30 degrees 45.55'
* ``DecimalDegrees``: Decimal degrees, eg 30.7555 degrees

"""
# --
QgsGeographicCoordinateNumericFormat.AngleFormat.baseClass = QgsGeographicCoordinateNumericFormat
try:
    QgsGeographicCoordinateNumericFormat.__group__ = ['numericformats']
except NameError:
    pass
