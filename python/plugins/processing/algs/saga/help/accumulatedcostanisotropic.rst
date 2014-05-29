ACCUMULATED COST (ANISOTROPIC)
==============================

Description
-----------

Parameters
----------

- ``Cost Grid[Raster]``:
- ``Direction of max cost[Raster]``:
- ``Destination Points[Raster]``:
- ``k factor[Number]``:
- ``Threshold for different route[Number]``:

Outputs
-------

- ``Accumulated Cost[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:accumulatedcostanisotropic', cost, direction, points, k, threshold, acccost)
