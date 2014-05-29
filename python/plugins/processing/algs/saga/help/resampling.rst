RESAMPLING
==========

Description
-----------

Parameters
----------

- ``Grid[Raster]``:
- ``Preserve Data Type[Boolean]``:
- ``Target Grid[Selection]``:
- ``Interpolation Method[Selection]``:
- ``Interpolation Method[Selection]``:
- ``Output extent[Extent]``:
- ``Cellsize[Number]``:

Outputs
-------

- ``Grid[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:resampling', input, keep_type, target, scale_up_method, scale_down_method, output_extent, user_size, user_grid)

	Available options for selection parameters:

	target(Target Grid)
		0 - [0] user defined

	scale_up_method(Interpolation Method)
		0 - [0] Nearest Neighbor
		1 - [1] Bilinear Interpolation
		2 - [2] Inverse Distance Interpolation
		3 - [3] Bicubic Spline Interpolation
		4 - [4] B-Spline Interpolation
		5 - [5] Mean Value
		6 - [6] Mean Value (cell area weighted)
		7 - [7] Minimum Value
		8 - [8] Maximum Value
		9 - [9] Majority

	scale_down_method(Interpolation Method)
		0 - [0] Nearest Neighbor
		1 - [1] Bilinear Interpolation
		2 - [2] Inverse Distance Interpolation
		3 - [3] Bicubic Spline Interpolation
		4 - [4] B-Spline Interpolation
