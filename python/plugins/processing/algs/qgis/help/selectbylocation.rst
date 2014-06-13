SELECT BY LOCATION
==================

Description
-----------

Parameters
----------

- ``Layer to select from[Vector]``:
- ``Additional layer (intersection layer)[Vector]``:
- ``Modify current selection by[Selection]``:

Outputs
-------

- ``Selection[Vector]``:

See also
---------


Console usage
-------------


::

	processing.runalg('qgis:selectbylocation', input, intersect, method)

	Available options for selection parameters:

	method(Modify current selection by)
		0 - creating new selection
		1 - adding to current selection
		2 - removing from current selection
