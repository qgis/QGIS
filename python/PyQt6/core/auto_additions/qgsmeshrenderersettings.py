# The following has been generated automatically from src/core/mesh/qgsmeshrenderersettings.h
QgsMeshRendererScalarSettings.NoResampling = QgsMeshRendererScalarSettings.DataResamplingMethod.NoResampling
QgsMeshRendererScalarSettings.NeighbourAverage = QgsMeshRendererScalarSettings.DataResamplingMethod.NeighbourAverage
# monkey patching scoped based enum
QgsMeshRendererScalarSettings.MinMaxValueType.UserDefined.__doc__ = ""
QgsMeshRendererScalarSettings.MinMaxValueType.WholeMesh.__doc__ = ""
QgsMeshRendererScalarSettings.MinMaxValueType.FixedCanvas.__doc__ = ""
QgsMeshRendererScalarSettings.MinMaxValueType.InteractiveFromCanvas.__doc__ = ""
QgsMeshRendererScalarSettings.MinMaxValueType.__doc__ = """This enumerator describes the extent used to compute min/max values

* ``UserDefined``: 
* ``WholeMesh``: 
* ``FixedCanvas``: 
* ``InteractiveFromCanvas``: 

"""
# --
QgsMeshRendererVectorArrowSettings.MinMax = QgsMeshRendererVectorArrowSettings.ArrowScalingMethod.MinMax
QgsMeshRendererVectorArrowSettings.Scaled = QgsMeshRendererVectorArrowSettings.ArrowScalingMethod.Scaled
QgsMeshRendererVectorArrowSettings.Fixed = QgsMeshRendererVectorArrowSettings.ArrowScalingMethod.Fixed
QgsMeshRendererVectorStreamlineSettings.MeshGridded = QgsMeshRendererVectorStreamlineSettings.SeedingStartPointsMethod.MeshGridded
QgsMeshRendererVectorStreamlineSettings.Random = QgsMeshRendererVectorStreamlineSettings.SeedingStartPointsMethod.Random
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
QgsMeshRendererVectorSettings.Arrows = QgsMeshRendererVectorSettings.Symbology.Arrows
QgsMeshRendererVectorSettings.Streamlines = QgsMeshRendererVectorSettings.Symbology.Streamlines
QgsMeshRendererVectorSettings.Traces = QgsMeshRendererVectorSettings.Symbology.Traces
QgsMeshRendererVectorSettings.WindBarbs = QgsMeshRendererVectorSettings.Symbology.WindBarbs
try:
    QgsMeshRendererMeshSettings.__group__ = ['mesh']
except NameError:
    pass
try:
    QgsMeshRendererScalarSettings.__group__ = ['mesh']
except NameError:
    pass
try:
    QgsMeshRendererVectorArrowSettings.__group__ = ['mesh']
except NameError:
    pass
try:
    QgsMeshRendererVectorStreamlineSettings.__group__ = ['mesh']
except NameError:
    pass
try:
    QgsMeshRendererVectorTracesSettings.__group__ = ['mesh']
except NameError:
    pass
try:
    QgsMeshRendererVectorWindBarbSettings.__group__ = ['mesh']
except NameError:
    pass
try:
    QgsMeshRendererVectorSettings.__group__ = ['mesh']
except NameError:
    pass
try:
    QgsMeshRendererSettings.__group__ = ['mesh']
except NameError:
    pass
