PATCHING
========

Description
-----------

Parameters
----------

- ``Grid[Raster]``:
- ``Patch Grid[Raster]``:
- ``Interpolation Method[Selection]``:

Outputs
-------

- ``Completed Grid[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:patching', original, additional, interpolation, completed)

	Available options for selection parameters:

	interpolation(Interpolation Method)
		0 - [0] Nearest Neighbor
		1 - [1] Bilinear Interpolation
		2 - [2] Inverse Distance Interpolation
		3 - [3] Bicubic Spline Interpolation
		4 - [4] B-Spline Interpolation
