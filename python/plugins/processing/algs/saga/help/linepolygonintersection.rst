LINE-POLYGON INTERSECTION
=========================

Description
-----------

Parameters
----------

- ``Lines[Vector]``:
- ``Polygons[Vector]``:
- ``Output[Selection]``:

Outputs
-------

- ``Intersection[Vector]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:linepolygonintersection', lines, polygons, method, intersect)

	Available options for selection parameters:

	method(Output)
		0 - [0] one multi-line per polygon
		1 - [1] keep original line attributes
