SURFACE SPECIFIC POINTS
=======================

Description
-----------

Parameters
----------

- ``Elevation[Raster]``:
- ``Method[Selection]``:
- ``Threshold[Number]``:

Outputs
-------

- ``Result[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:surfacespecificpoints', elevation, method, threshold, result)

	Available options for selection parameters:

	method(Method)
		0 - [0] Mark Highest Neighbour
		1 - [1] Opposite Neighbours
		2 - [2] Flow Direction
		3 - [3] Flow Direction (up and down)
		4 - [4] Peucker & Douglas
