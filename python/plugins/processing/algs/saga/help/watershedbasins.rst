WATERSHED BASINS
================

Description
-----------

Parameters
----------

- ``Elevation[Raster]``:
- ``Channel Network[Raster]``:
- ``Sink Route[Raster]``:
- ``Min. Size[Number]``:

Outputs
-------

- ``Watershed Basins[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:watershedbasins', elevation, channels, sinkroute, minsize, basins)
