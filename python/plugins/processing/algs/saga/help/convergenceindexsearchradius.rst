CONVERGENCE INDEX (SEARCH RADIUS)
=================================

Description
-----------

Parameters
----------

- ``Elevation[Raster]``:
- ``Radius [Cells][Number]``:
- ``Distance Weighting[Selection]``:
- ``Inverse Distance Weighting Power[Number]``:
- ``Inverse Distance Offset[Boolean]``:
- ``Gaussian and Exponential Weighting Bandwidth[Number]``:
- ``Gradient[Boolean]``:
- ``Difference[Selection]``:

Outputs
-------

- ``Convergence Index[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:convergenceindexsearchradius', elevation, radius, distance_weighting_weighting, distance_weighting_idw_power, distance_weighting_idw_offset, distance_weighting_bandwidth, slope, difference, convergence)

	Available options for selection parameters:

	distance_weighting_weighting(Distance Weighting)
		0 - [0] no distance weighting
		1 - [1] inverse distance to a power
		2 - [2] exponential
		3 - [3] gaussian weighting

	difference(Difference)
		0 - [0] direction to the center cell
		1 - [1] center cell's aspect direction
