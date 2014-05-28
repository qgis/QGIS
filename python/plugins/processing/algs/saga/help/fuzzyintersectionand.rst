FUZZY INTERSECTION (AND)
========================

Description
-----------

Parameters
----------

- ``Grids[MultipleInput]``:
- ``Operator Type[Selection]``:

Outputs
-------

- ``Intersection[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:fuzzyintersectionand', grids, type, and)

	Available options for selection parameters:

	type(Operator Type)
		0 - [0] min(a, b) (non-interactive)
		1 - [1] a * b
		2 - [2] max(0, a + b - 1)
