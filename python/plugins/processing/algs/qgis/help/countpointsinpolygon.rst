COUNT POINTS IN POLYGON
=======================

Description
-----------
This algorithm counts the number of points present in each feature of a polygons layer. 

Parameters
----------

- ``Polygons[Vector]``:polygons layer
- ``Points[Vector]``:points layer
- ``Count field name[String]``:the name of the attribute table column containing the points number

Outputs
-------

- ``Result[Vector]``: resulting layer with the attribute table containing the new column of the points count.

See also
---------


Console usage
-------------


::

	processing.runalg('qgis:countpointsinpolygon', polygons, points, field, output)
