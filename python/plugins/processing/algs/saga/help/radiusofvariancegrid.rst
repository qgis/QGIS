RADIUS OF VARIANCE (GRID)
=========================

Description
-----------

Parameters
----------

- ``Grid[Raster]``:
- ``Standard Deviation[Number]``:
- ``Maximum Search Radius (cells)[Number]``:
- ``Type of Output[Selection]``:

Outputs
-------

- ``Variance Radius[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:radiusofvariancegrid', input, variance, radius, output, result)

	Available options for selection parameters:

	output(Type of Output)
		0 - [0] Cells
		1 - [1] Map Units
