GEOGRAPHICALLY WEIGHTED MULTIPLE REGRESSION (POINTS)
====================================================

Description
-----------

Parameters
----------

- ``Points[Vector]``:
- ``Dependent Variable[TableField]``:
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

Outputs
-------

- ``Regression[Vector]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:geographicallyweightedmultipleregressionpoints', 
						points, dependent, distance_weighting_weighting, 
						distance_weighting_idw_power, distance_weighting_idw_offset, 
						distance_weighting_bandwidth, range, radius, mode, npoints, 
						maxpoints, minpoints, regression)

	Available options for selection parameters:

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

