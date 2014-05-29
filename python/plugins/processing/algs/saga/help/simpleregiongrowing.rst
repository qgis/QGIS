SIMPLE REGION GROWING
=====================

Description
-----------

Parameters
----------

- ``Seeds[Raster]``:
- ``Features[MultipleInput]``:
- ``Method[Selection]``:
- ``Neighbourhood[Selection]``:
- ``Variance in Feature Space[Number]``:
- ``Variance in Position Space[Number]``:
- ``Threshold - Similarity[Number]``:
- ``Refresh[Boolean]``:
- ``Leaf Size (for Speed Optimisation)[Number]``:

Outputs
-------

- ``Segments[Raster]``:
- ``Similarity[Raster]``:
- ``Seeds[Table]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:simpleregiongrowing', seeds, features, method, neighbour, sig_1, sig_2, threshold, refresh, leafsize, segments, similarity, table)

	Available options for selection parameters:

	method(Method)
		0 - [0] feature space and position
		1 - [1] feature space

	neighbour(Neighbourhood)
		0 - [0] 4 (von Neumann)
		1 - [1] 8 (Moore)
