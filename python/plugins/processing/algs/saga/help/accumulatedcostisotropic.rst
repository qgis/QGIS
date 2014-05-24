ACCUMULATED COST (ISOTROPIC)
============================

Description
-----------

Parameters
----------

- ``Cost Grid[Raster]``:
- ``Destination Points[Raster]``:
- ``Threshold for different route[Number]``:

Outputs
-------

- ``Accumulated Cost[Raster]``:
- ``Closest Point[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:accumulatedcostisotropic', cost, points, threshold, acccost, closestpt)
