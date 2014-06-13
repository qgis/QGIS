MERGE RASTER LAYERS
===================

Description
-----------

Parameters
----------

- ``Grids to Merge[MultipleInput]``:
- ``Preferred data storage type[Selection]``:
- ``Interpolation[Selection]``:
- ``Overlapping Cells[Selection]``:

Outputs
-------

- ``Merged Grid[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:merging', grids, type, interpol, overlap, merged)

	Available options for selection parameters:

	type(Preferred data storage type)
		0 - [0] 1 bit
		1 - [1] 1 byte unsigned integer
		2 - [2] 1 byte signed integer
		3 - [3] 2 byte unsigned integer
		4 - [4] 2 byte signed integer
		5 - [5] 4 byte unsigned integer
		6 - [6] 4 byte signed integer
		7 - [7] 4 byte floating point
		8 - [8] 8 byte floating point

	interpol(Interpolation)
		0 - [0] Nearest Neighbor
		1 - [1] Bilinear Interpolation
		2 - [2] Inverse Distance Interpolation
		3 - [3] Bicubic Spline Interpolation
		4 - [4] B-Spline Interpolation

	overlap(Overlapping Cells)
		0 - [0] mean value
		1 - [1] first value in order of grid list
