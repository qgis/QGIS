POINTS FILTER
=============

Description
-----------

Parameters
----------

- ``Points[Vector]``:
- ``Attribute[TableField]``:
- ``Radius[Number]``:
- ``Minimum Number of Points[Number]``:
- ``Maximum Number of Points[Number]``:
- ``Quadrants[Boolean]``:
- ``Filter Criterion[Selection]``:
- ``Tolerance[Number]``:
- ``Percentile[Number]``:

Outputs
-------

- ``Filtered Points[Vector]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:pointsfilter', points, field, radius, minnum, maxnum, quadrants, method, tolerance, percent, filter)

	Available options for selection parameters:

	method(Filter Criterion)
		0 - [0] keep maxima (with tolerance)
		1 - [1] keep minima (with tolerance)
		2 - [2] remove maxima (with tolerance)
		3 - [3] remove minima (with tolerance)
		4 - [4] remove below percentile
		5 - [5] remove above percentile
