LAKE FLOOD
==========

Description
-----------

Parameters
----------

- ``DEM[Raster]``:
- ``Seeds[Raster]``:
- ``Absolute Water Levels[Boolean]``:

Outputs
-------

- ``Lake[Raster]``:
- ``Surface[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:lakeflood', elev, seeds, level, outdepth, outlevel)
