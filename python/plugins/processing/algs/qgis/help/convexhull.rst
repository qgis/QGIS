CONVEX HULL
===========

Description
-----------

Parameters
----------

- ``Input layer[Vector]``:
- ``Field[TableField]``:
- ``Method[Selection]``:

Outputs
-------

- ``Convex hull[Vector]``:

See also
---------


Console usage
-------------


::

	processing.runalg('qgis:convexhull', input, field, method, output)

	Available options for selection parameters:

	method(Method)
		0 - Create single minimum convex hull
		1 - Create convex hulls based on field
