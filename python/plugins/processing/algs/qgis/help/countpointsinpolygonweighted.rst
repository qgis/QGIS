COUNT POINTS IN POLYGON(WEIGHTED)
=================================

Description
-----------
This algorithm counts the number of points in each feature of a polygons layer and calculates the mean of the selected field for 
each feature of the polygons layer.
These values will be added to the attribute table of the resulting polygons layer.

Parameters
----------

- ``Polygons[Vector]``: polygons layer in input
- ``Points[Vector]``: points layer in input
- ``Weight field[TableField]``: weight field of the points attribute table
- ``Count field name[String]``: name of the column for the new weighted field

Outputs
-------

- ``Result[Vector]``: the resulting polygons layer

See also
---------
Count points in polygon algorithm

Console usage
-------------


::

	processing.runalg('qgis:countpointsinpolygonweighted', polygons, points, weight, field, output)
