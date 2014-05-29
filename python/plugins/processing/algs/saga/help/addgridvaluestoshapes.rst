ADD GRID VALUES TO SHAPES
=========================

Description
-----------

Parameters
----------

- ``Shapes[Vector]``:
- ``Grids[MultipleInput]``:
- ``Interpolation[Selection]``:

Outputs
-------

- ``Result[Vector]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:addgridvaluestoshapes', shapes, grids, interpol, result)

	Available options for selection parameters:

	interpol(Interpolation)
		0 - [0] Nearest Neighbor
		1 - [1] Bilinear Interpolation
		2 - [2] Inverse Distance Interpolation
		3 - [3] Bicubic Spline Interpolation
		4 - [4] B-Spline Interpolation
