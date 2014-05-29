FLAT DETECTION
==============

Description
-----------

Parameters
----------

- ``DEM[Raster]``:
- ``Flat Area Values[Selection]``:

Outputs
-------

- ``No Flats[Raster]``:
- ``Flat Areas[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:flatdetection', dem, flat_output, noflats, flats)

	Available options for selection parameters:

	flat_output(Flat Area Values)
		0 - [0] elevation
		1 - [1] enumeration
