EXPORT/ADD GEOMETRY COLUMNS
===========================

Description
-----------

Parameters
----------

- ``Input layer[Vector]``:
- ``Calculate using[Selection]``:

Outputs
-------

- ``Output layer[Vector]``:

See also
---------


Console usage
-------------


::

	processing.runalg('qgis:exportaddgeometrycolumns', input, calc_method, output)

	Available options for selection parameters:

	calc_method(Calculate using)
		0 - Layer CRS
		1 - Project CRS
		2 - Ellipsoidal
