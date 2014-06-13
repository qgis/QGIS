COUNT UNIQUE POINTS IN POLYGON
==============================

Description
-----------
This algorithm counts the number of unique values of a points layer in a polygons one. The algorithm creates a new polygons
layer with an extra column in the attribute table containing the count of unique values for each feature.

Parameters
----------

- ``Polygons[Vector]``: polygons layer in input
- ``Points[Vector]``: points layer in input
- ``Class field[TableField]``: points layer column name of the unique value chosen
- ``Count field name[String]``: column name containing the count of unique values in the resulting polygons layer

Outputs
-------

- ``Result[Vector]``: the resulting polygons layer

See also
---------


Console usage
-------------


::

	processing.runalg('qgis:countuniquepointsinpolygon', polygons, points, classfield, field, output)
