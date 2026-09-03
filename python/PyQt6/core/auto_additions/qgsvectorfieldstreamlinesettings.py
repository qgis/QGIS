# The following has been generated automatically from src/core/vectorfield/qgsvectorfieldstreamlinesettings.h
# monkey patching scoped based enum
QgsVectorFieldStreamlineSettings.SeedingStartPointsMethod.Gridded.__doc__ = "Seeds start points on data grid or user regular grid"
QgsVectorFieldStreamlineSettings.SeedingStartPointsMethod.Random.__doc__ = "Seeds start points randomly"
QgsVectorFieldStreamlineSettings.SeedingStartPointsMethod.MeshGridded.__doc__ = "For backwards compatibility \n.. deprecated:: 4.4"
QgsVectorFieldStreamlineSettings.SeedingStartPointsMethod.__doc__ = """Method used to define start points that are used to draw streamlines

* ``Gridded``: Seeds start points on data grid or user regular grid
* ``Random``: Seeds start points randomly
* ``MeshGridded``: For backwards compatibility

  .. deprecated:: 4.4


"""
# --
try:
    QgsVectorFieldStreamlineSettings.__group__ = ['vectorfield']
except (NameError, AttributeError):
    pass
