MULTI-BAND VARIATION
====================

Description
-----------

Parameters
----------

- ``Grids[MultipleInput]``:
- ``Radius [Cells][Number]``:
- ``Distance Weighting[Selection]``:
- ``Inverse Distance Weighting Power[Number]``:
- ``Inverse Distance Offset[Boolean]``:
- ``Gaussian and Exponential Weighting Bandwidth[Number]``:

Outputs
-------

- ``Mean Distance[Raster]``:
- ``Standard Deviation[Raster]``:
- ``Distance[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:multibandvariation', bands, radius, distance_weighting_weighting, distance_weighting_idw_power, distance_weighting_idw_offset, distance_weighting_bandwidth, mean, stddev, diff)

	Available options for selection parameters:

	distance_weighting_weighting(Distance Weighting)
		0 - [0] no distance weighting
		1 - [1] inverse distance to a power
		2 - [2] exponential
		3 - [3] gaussian weighting
