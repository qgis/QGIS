TOPOGRAPHIC WETNESS INDEX (TWI)
===============================

Description
-----------

Parameters
----------

- ``Slope[Raster]``:
- ``Catchment Area[Raster]``:
- ``Transmissivity[Raster]``:
- ``Area Conversion[Selection]``:
- ``Method (TWI)[Selection]``:

Outputs
-------

- ``Topographic Wetness Index[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:topographicwetnessindextwi', slope, area, trans, conv, method, twi)

	Available options for selection parameters:

	conv(Area Conversion)
		0 - [0] no conversion (areas already given as specific catchment area)
		1 - [1] 1 / cell size (pseudo specific catchment area)

	method(Method (TWI))
		0 - [0] Standard
		1 - [1] TOPMODEL
