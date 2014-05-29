EFFECTIVE AIR FLOW HEIGHTS
==========================

Description
-----------

Parameters
----------

- ``Elevation[Raster]``:
- ``Wind Direction[Raster]``:
- ``Wind Speed[Raster]``:
- ``Constant Wind Direction [Degree][Number]``:
- ``Old Version[Boolean]``:
- ``Search Distance [km][Number]``:
- ``Acceleration[Number]``:
- ``Use Pyramids with New Version[Boolean]``:
- ``Lee Factor[Number]``:
- ``Luv Factor[Number]``:
- ``Wind Direction Units[Selection]``:
- ``Wind Speed Scale Factor[Number]``:

Outputs
-------

- ``Effective Air Flow Heights[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:effectiveairflowheights', dem, dir, len, dir_const, oldver, maxdist, accel, pyramids, leefact, luvfact, dir_units, len_scale, afh)

	Available options for selection parameters:

	dir_units(Wind Direction Units)
		0 - [0] radians
		1 - [1] degree
