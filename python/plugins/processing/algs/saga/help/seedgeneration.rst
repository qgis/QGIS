SEED GENERATION
===============

Description
-----------

Parameters
----------

- ``Features[MultipleInput]``:
- ``Bandwidth (Cells)[Number]``:
- ``Type of Surface[Selection]``:
- ``Extraction of...[Selection]``:
- ``Feature Aggregation[Selection]``:
- ``Normalized[Boolean]``:

Outputs
-------

- ``Surface[Raster]``:
- ``Seeds Grid[Raster]``:
- ``Seeds[Vector]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:seedgeneration', grids, factor, type_surface, type_seeds, type_merge, normalize, surface, seeds_grid, seeds)

	Available options for selection parameters:

	type_surface(Type of Surface)
		0 - [0] smoothed surface
		1 - [1] variance (a)
		2 - [2] variance (b)

	type_seeds(Extraction of...)
		0 - [0] minima
		1 - [1] maxima
		2 - [2] minima and maxima

	type_merge(Feature Aggregation)
		0 - [0] additive
		1 - [1] multiplicative
