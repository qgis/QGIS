# The following has been generated automatically from src/core/mesh/qgsmeshrenderersettings.h
# monkey patching scoped based enum
QgsMeshRendererVectorWindBarbSettings.WindSpeedUnit.MetersPerSecond.__doc__ = "Meters per second"
QgsMeshRendererVectorWindBarbSettings.WindSpeedUnit.KilometersPerHour.__doc__ = "Kilometers per hour"
QgsMeshRendererVectorWindBarbSettings.WindSpeedUnit.Knots.__doc__ = "Knots (Nautical miles per hour)"
QgsMeshRendererVectorWindBarbSettings.WindSpeedUnit.MilesPerHour.__doc__ = "Miles per hour"
QgsMeshRendererVectorWindBarbSettings.WindSpeedUnit.FeetPerSecond.__doc__ = "Feet per second"
QgsMeshRendererVectorWindBarbSettings.WindSpeedUnit.OtherUnit.__doc__ = "Other unit"
QgsMeshRendererVectorWindBarbSettings.WindSpeedUnit.__doc__ = """Wind speed units. Wind barbs use knots so we use this enum for preset conversion values

* ``MetersPerSecond``: Meters per second
* ``KilometersPerHour``: Kilometers per hour
* ``Knots``: Knots (Nautical miles per hour)
* ``MilesPerHour``: Miles per hour
* ``FeetPerSecond``: Feet per second
* ``OtherUnit``: Other unit

"""
# --
try:
    QgsMeshRendererMeshSettings.__group__ = ['mesh']
except (NameError, AttributeError):
    pass
try:
    QgsMeshRendererScalarSettings.__group__ = ['mesh']
except (NameError, AttributeError):
    pass
try:
    QgsMeshRendererVectorArrowSettings.__group__ = ['mesh']
except (NameError, AttributeError):
    pass
try:
    QgsMeshRendererVectorStreamlineSettings.__group__ = ['mesh']
except (NameError, AttributeError):
    pass
try:
    QgsMeshRendererVectorTracesSettings.__group__ = ['mesh']
except (NameError, AttributeError):
    pass
try:
    QgsMeshRendererVectorWindBarbSettings.__group__ = ['mesh']
except (NameError, AttributeError):
    pass
try:
    QgsMeshRendererVectorSettings.__group__ = ['mesh']
except (NameError, AttributeError):
    pass
try:
    QgsMeshRendererSettings.__group__ = ['mesh']
except (NameError, AttributeError):
    pass
