SAGA WETNESS INDEX
==================

Description
-----------

Parameters
----------

- ``Elevation[Raster]``:
- ``t[Number]``:

Outputs
-------

- ``Catchment area[Raster]``:
- ``Catchment slope[Raster]``:
- ``Modified catchment area[Raster]``:
- ``Wetness index[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:sagawetnessindex', dem, t, c, gn, cs, sb)
