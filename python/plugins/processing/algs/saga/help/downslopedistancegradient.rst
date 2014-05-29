DOWNSLOPE DISTANCE GRADIENT
===========================

Description
-----------

Parameters
----------

- ``Elevation[Raster]``:
- ``Vertical Distance[Number]``:
- ``Output[Selection]``:

Outputs
-------

- ``Gradient[Raster]``:
- ``Gradient Difference[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:downslopedistancegradient', dem, distance, output, gradient, difference)

	Available options for selection parameters:

	output(Output)
		0 - [0] distance
		1 - [1] gradient (tangens)
		2 - [2] gradient (degree)
