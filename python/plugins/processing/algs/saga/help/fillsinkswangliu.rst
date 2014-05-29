FILL SINKS (WANG & LIU)
=======================

Description
-----------

Parameters
----------

- ``DEM[Raster]``:
- ``Minimum Slope [Degree][Number]``:

Outputs
-------

- ``Filled DEM[Raster]``:
- ``Flow Directions[Raster]``:
- ``Watershed Basins[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:fillsinkswangliu', elev, minslope, filled, fdir, wshed)
