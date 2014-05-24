FRAGMENTATION (STANDARD)
========================

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
- ``Neighborhood Type[Selection]``:
- ``Include diagonal neighbour relations[Boolean]``:

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

	processing.runalg('saga:fragmentationstandard', classes, class, neighborhood_min, neighborhood_max, aggregation, border, weight, density_min, density_int, circular, diagonal, density, connectivity, fragmentation, fragstats)

	Available options for selection parameters:

	aggregation(Level Aggregation)
		0 - [0] average
		1 - [1] multiplicative

	circular(Neighborhood Type)
		0 - [0] square
		1 - [1] circle
