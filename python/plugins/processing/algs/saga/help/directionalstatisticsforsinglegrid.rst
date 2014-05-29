DIRECTIONAL STATISTICS FOR SINGLE GRID
======================================

Description
-----------

Parameters
----------

- ``Grid[Raster]``:
- ``Points[Vector]``:
- ``Direction [Degree][Number]``:
- ``Tolerance [Degree][Number]``:
- ``Maximum Distance [Cells][Number]``:
- ``Distance Weighting[Selection]``:
- ``Inverse Distance Weighting Power[Number]``:
- ``Inverse Distance Offset[Boolean]``:
- ``Gaussian and Exponential Weighting Bandwidth[Number]``:

Outputs
-------

- ``Arithmetic Mean[Raster]``:
- ``Difference from Arithmetic Mean[Raster]``:
- ``Minimum[Raster]``:
- ``Maximum[Raster]``:
- ``Range[Raster]``:
- ``Variance[Raster]``:
- ``Standard Deviation[Raster]``:
- ``Mean less Standard Deviation[Raster]``:
- ``Mean plus Standard Deviation[Raster]``:
- ``Deviation from Arithmetic Mean[Raster]``:
- ``Percentile[Raster]``:
- ``Directional Statistics for Points[Vector]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:directionalstatisticsforsinglegrid', grid, points, direction, tolerance, maxdistance, distance_weighting_weighting, distance_weighting_idw_power, distance_weighting_idw_offset, distance_weighting_bandwidth, mean, difmean, min, max, range, var, stddev, stddevlo, stddevhi, devmean, percent, points_out)

	Available options for selection parameters:

	distance_weighting_weighting(Distance Weighting)
		0 - [0] no distance weighting
		1 - [1] inverse distance to a power
		2 - [2] exponential
		3 - [3] gaussian weighting
