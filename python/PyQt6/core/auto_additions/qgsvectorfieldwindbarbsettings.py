# The following has been generated automatically from src/core/vectorfield/qgsvectorfieldwindbarbsettings.h
# monkey patching scoped based enum
QgsVectorFieldWindBarbSettings.WindSpeedUnit.MetersPerSecond.__doc__ = "Meters per second"
QgsVectorFieldWindBarbSettings.WindSpeedUnit.KilometersPerHour.__doc__ = "Kilometers per hour"
QgsVectorFieldWindBarbSettings.WindSpeedUnit.Knots.__doc__ = "Knots (Nautical miles per hour)"
QgsVectorFieldWindBarbSettings.WindSpeedUnit.MilesPerHour.__doc__ = "Miles per hour"
QgsVectorFieldWindBarbSettings.WindSpeedUnit.FeetPerSecond.__doc__ = "Feet per second"
QgsVectorFieldWindBarbSettings.WindSpeedUnit.OtherUnit.__doc__ = "Other unit"
QgsVectorFieldWindBarbSettings.WindSpeedUnit.__doc__ = """Wind speed units. Wind barbs use knots so we use this enum for preset conversion values

* ``MetersPerSecond``: Meters per second
* ``KilometersPerHour``: Kilometers per hour
* ``Knots``: Knots (Nautical miles per hour)
* ``MilesPerHour``: Miles per hour
* ``FeetPerSecond``: Feet per second
* ``OtherUnit``: Other unit

"""
# --
try:
    QgsVectorFieldWindBarbSettings.__group__ = ['vectorfield']
except (NameError, AttributeError):
    pass
