WIND EFFECT
===========

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
- ``Use Pyramids[Boolean]``:
- ``Wind Direction Units[Selection]``:
- ``Wind Speed Scale Factor[Number]``:

Outputs
-------

- ``Wind Effect[Raster]``:
- ``Windward Effect[Raster]``:
- ``Leeward Effect[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:windeffectwindwardleewardindex', dem, dir, len, dir_const, oldver, maxdist, accel, pyramids, dir_units, len_scale, effect, luv, lee)

	Available options for selection parameters:

	dir_units(Wind Direction Units)
		0 - [0] radians
		1 - [1] degree
