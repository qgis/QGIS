CLUSTER ANALYSIS FOR GRIDS
==========================

Description
-----------

Parameters
----------

- ``Grids[MultipleInput]``:
- ``Method[Selection]``:
- ``Clusters[Number]``:
- ``Normalise[Boolean]``:
- ``Old Version[Boolean]``:

Outputs
-------

- ``Clusters[Raster]``:
- ``Statistics[Table]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:clusteranalysisforgrids', grids, method, ncluster, normalise, oldversion, cluster, statistics)

	Available options for selection parameters:

	method(Method)
		0 - [0] Iterative Minimum Distance (Forgy 1965)
		1 - [1] Hill-Climbing (Rubin 1967)
		2 - [2] Combined Minimum Distance / Hillclimbing
