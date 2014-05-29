SKY VIEW FACTOR
===============

Description
-----------

Parameters
----------

- ``Elevation[Raster]``:
- ``Maximum Search Radius[Number]``:
- ``Method[Selection]``:
- ``Multi Scale Factor[Number]``:
- ``Number of Sectors[Number]``:

Outputs
-------

- ``Visible Sky[Raster]``:
- ``Sky View Factor[Raster]``:
- ``Sky View Factor (Simplified)[Raster]``:
- ``Terrain View Factor[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:skyviewfactor', dem, maxradius, method, level_inc, ndirs, visible, svf, simple, terrain)

	Available options for selection parameters:

	method(Method)
		0 - [0] multi scale
		1 - [1] sectors
