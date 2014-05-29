CONVERT POLYGON/LINE VERTICES TO POINTS
=======================================

Description
-----------
This algorithm converts the line vertices into points and creates a points layer. 

Parameters
----------

- ``Shapes[Vector]``: it make sense using polygons or lines layer in input

Outputs
-------

- ``Points[Vector]``: resulting layer

See also
---------


Console usage
-------------


::

	processing.runalg('saga:convertpolygonlineverticestopoints', shapes, points)
