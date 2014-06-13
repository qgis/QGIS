MASS BALANCE INDEX
==================

Description
-----------

Parameters
----------

- ``Elevation[Raster]``:
- ``Vertical Distance to Channel Network[Raster]``:
- ``T Slope[Number]``:
- ``T Curvature[Number]``:
- ``T Vertical Distance to Channel Network[Number]``:

Outputs
-------

- ``Mass Balance Index[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:massbalanceindex', dem, hrel, tslope, tcurve, threl, mbi)
