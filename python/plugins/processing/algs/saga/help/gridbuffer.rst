GRID BUFFER
===========

Description
-----------

Parameters
----------

- ``Features Grid[Raster]``:
- ``Distance[Number]``:
- ``Buffer Distance[Selection]``:

Outputs
-------

- ``Buffer Grid[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:gridbuffer', features, dist, buffertype, buffer)

	Available options for selection parameters:

	buffertype(Buffer Distance)
		0 - [0] Fixed
		1 - [1] Cell value
