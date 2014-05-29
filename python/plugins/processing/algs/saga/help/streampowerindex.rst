STREAM POWER INDEX
==================

Description
-----------

Parameters
----------

- ``Slope[Raster]``:
- ``Catchment Area[Raster]``:
- ``Area Conversion[Selection]``:

Outputs
-------

- ``Stream Power Index[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:streampowerindex', slope, area, conv, spi)

	Available options for selection parameters:

	conv(Area Conversion)
		0 - [0] no conversion (areas already given as specific catchment area)
		1 - [1] 1 / cell size (pseudo specific catchment area)
