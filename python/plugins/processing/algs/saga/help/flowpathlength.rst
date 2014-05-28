FLOW PATH LENGTH
================

Description
-----------

Parameters
----------

- ``Elevation[Raster]``:
- ``Seeds[Raster]``:
- ``Seeds Only[Boolean]``:
- ``Flow Routing Algorithm[Selection]``:
- ``Convergence (FD8)[Number]``:

Outputs
-------

- ``Flow Path Length[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:flowpathlength', elevation, seed, seeds_only, method, convergence, length)

	Available options for selection parameters:

	method(Flow Routing Algorithm)
		0 - [0] Deterministic 8 (D8)
		1 - [1] Multiple Flow Direction (FD8)
