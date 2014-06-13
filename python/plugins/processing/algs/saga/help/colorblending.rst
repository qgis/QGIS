COLOR BLENDING
==============

Description
-----------

Parameters
----------

- ``Grids[MultipleInput]``:
- ``Interpolation Steps[Number]``:
- ``Color Stretch[Selection]``:

Outputs
-------

- ``Grid[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:colorblending', grids, nsteps, range, grid)

	Available options for selection parameters:

	range(Color Stretch)
		0 - [0] fit to each grid
		1 - [1] fit to overall range
		2 - [2] fit to overall 1.5 standard deviation
		3 - [3] fit to overall 2.0 standard deviation
