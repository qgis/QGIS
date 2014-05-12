INVERSE DISTANCE WEIGHTED
=========================

Description
-----------

Parameters
----------

- ``Points[Vector]``:
- ``Attribute[TableField]``:
- ``Target Grid[Selection]``:
- ``Distance Weighting[Selection]``:
- ``Inverse Distance Power[Number]``:
- ``Exponential and Gaussian Weighting Bandwidth[Number]``:
- ``Search Range[Selection]``:
- ``Search Radius[Number]``:
- ``Search Mode[Selection]``:
- ``Number of Points[Selection]``:
- ``Maximum Number of Points[Number]``:
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

	sextante.runalg('saga:inversedistanceweighted', shapes, field, target, weighting, power, bandwidth, range, radius, mode, points, npoints, output_extent, user_size, user_grid)

	Available options for selection parameters:

	target(Target Grid)
		0 - [0] user defined

	weighting(Distance Weighting)
		0 - [0] inverse distance to a power
		1 - [1] linearly decreasing within search radius
		2 - [2] exponential weighting scheme
		3 - [3] gaussian weighting scheme

	range(Search Range)
		0 - [0] search radius (local)
		1 - [1] no search radius (global)

	mode(Search Mode)
		0 - [0] all directions
		1 - [1] quadrants

	points(Number of Points)
		0 - [0] maximum number of points
		1 - [1] all points
