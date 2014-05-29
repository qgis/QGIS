TERRAIN RUGGEDNESS INDEX (TRI)
==============================

Description
-----------

Parameters
----------

- ``Elevation[Raster]``:
- ``Radius (Cells)[Number]``:
- ``Distance Weighting[Selection]``:
- ``Inverse Distance Weighting Power[Number]``:
- ``Inverse Distance Offset[Boolean]``:
- ``Gaussian and Exponential Weighting Bandwidth[Number]``:

Outputs
-------

- ``Terrain Ruggedness Index (TRI)[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:terrainruggednessindextri', dem, radius, distance_weighting_weighting, distance_weighting_idw_power, distance_weighting_idw_offset, distance_weighting_bandwidth, tri)

	Available options for selection parameters:

	distance_weighting_weighting(Distance Weighting)
		0 - [0] no distance weighting
		1 - [1] inverse distance to a power
		2 - [2] exponential
		3 - [3] gaussian weighting
