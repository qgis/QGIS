FRAGMENTATION (ALTERNATIVE)
===========================

Description
-----------

Parameters
----------

- ``Classification[Raster]``:
- ``Class Identifier[Number]``:
- ``Neighborhood Min[Number]``:
- ``Neighborhood Max[Number]``:
- ``Level Aggregation[Selection]``:
- ``Add Border[Boolean]``:
- ``Connectivity Weighting[Number]``:
- ``Minimum Density [Percent][Number]``:
- ``Minimum Density for Interior Forest [Percent][Number]``:
- ``Search Distance Increment[Number]``:
- ``Density from Neighbourhood[Boolean]``:

Outputs
-------

- ``Density [Percent][Raster]``:
- ``Connectivity [Percent][Raster]``:
- ``Fragmentation[Raster]``:
- ``Summary[Table]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:fragmentationalternative', classes, class, neighborhood_min, neighborhood_max, aggregation, border, weight, density_min, density_int, level_grow, density_mean, density, connectivity, fragmentation, fragstats)

	Available options for selection parameters:

	aggregation(Level Aggregation)
		0 - [0] average
		1 - [1] multiplicative
