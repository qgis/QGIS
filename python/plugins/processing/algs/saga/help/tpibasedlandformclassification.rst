TPI BASED LANDFORM CLASSIFICATION
=================================

Description
-----------

Parameters
----------

- ``Elevation[Raster]``:
- ``Min Radius A[Number]``:
- ``Max Radius A[Number]``:
- ``Min Radius B[Number]``:
- ``Min Radius B[Number]``:
- ``Distance Weighting[Selection]``:
- ``Inverse Distance Weighting Power[Number]``:
- ``Inverse Distance Offset[Boolean]``:
- ``Gaussian and Exponential Weighting Bandwidth[Number]``:

Outputs
-------

- ``Landforms[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:tpibasedlandformclassification', dem, radius_a_min, radius_a_max, radius_b_min, radius_b_max, distance_weighting_weighting, distance_weighting_idw_power, distance_weighting_idw_offset, distance_weighting_bandwidth, landforms)

	Available options for selection parameters:

	distance_weighting_weighting(Distance Weighting)
		0 - [0] no distance weighting
		1 - [1] inverse distance to a power
		2 - [2] exponential
		3 - [3] gaussian weighting
