CONVEX HULL
===========

Description
-----------

Parameters
----------

- ``Points[Vector]``:
- ``Hull Construction[Selection]``:

Outputs
-------

- ``Convex Hull[Vector]``:
- ``Minimum Bounding Box[Vector]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:convexhull', shapes, polypoints, hulls, boxes)

	Available options for selection parameters:

	polypoints(Hull Construction)
		0 - [0] one hull for all shapes
		1 - [1] one hull per shape
		2 - [2] one hull per shape part
