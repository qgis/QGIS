# The following has been generated automatically from src/analysis/georeferencing/qgsgcppoint.h
# monkey patching scoped based enum
QgsGcpPoint.PointType.Source.__doc__ = "Source point"
QgsGcpPoint.PointType.Destination.__doc__ = "Destination point"
QgsGcpPoint.PointType.__doc__ = """Coordinate point types

* ``Source``: Source point
* ``Destination``: Destination point

"""
# --
try:
    QgsGcpPoint.__group__ = ['georeferencing']
except (NameError, AttributeError):
    pass
