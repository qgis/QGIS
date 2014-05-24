GEOGRAPHICALLY WEIGHTED REGRESSION
==================================

Description
-----------

Parameters
----------

- ``Points[Vector]``:
- ``Dependent Variable[TableField]``:
- ``Predictor[TableField]``:
- ``Target Grids[Selection]``:
- ``Distance Weighting[Selection]``:
- ``Inverse Distance Weighting Power[Number]``:
- ``Inverse Distance Offset[Boolean]``:
- ``Gaussian and Exponential Weighting Bandwidth[Number]``:
- ``Search Range[Selection]``:
- ``Search Radius[Number]``:
- ``Search Mode[Selection]``:
- ``Number of Points[Selection]``:
- ``Maximum Number of Observations[Number]``:
- ``Minimum Number of Observations[Number]``:
- ``Output extent[Extent]``:
- ``Cellsize[Number]``:

Outputs
-------

- ``Grid[Raster]``:
- ``Quality[Raster]``:
- ``Intercept[Raster]``:
- ``Slope[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:geographicallyweightedregression', points, dependent, predictor, target, distance_weighting_weighting, distance_weighting_idw_power, distance_weighting_idw_offset, distance_weighting_bandwidth, range, radius, mode, npoints, maxpoints, minpoints, output_extent, user_size, user_grid, user_quality, user_intercept, user_slope)

	Available options for selection parameters:

	target(Target Grids)
		0 - [0] user defined

	distance_weighting_weighting(Distance Weighting)
		0 - [0] no distance weighting
		1 - [1] inverse distance to a power
		2 - [2] exponential
		3 - [3] gaussian weighting

	range(Search Range)
		0 - [0] search radius (local)
		1 - [1] no search radius (global)

	mode(Search Mode)
		0 - [0] all directions
		1 - [1] quadrants

	npoints(Number of Points)
		0 - [0] maximum number of observations
		1 - [1] all points
