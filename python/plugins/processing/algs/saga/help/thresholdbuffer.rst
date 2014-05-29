THRESHOLD BUFFER
================

Description
-----------

Parameters
----------

- ``Features Grid[Raster]``:
- ``Value Grid[Raster]``:
- ``Threshold Grid[Raster]``:
- ``Threshold[Number]``:
- ``Threshold Type[Selection]``:

Outputs
-------

- ``Buffer Grid[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:thresholdbuffer', features, value, thresholdgrid, threshold, thresholdtype, buffer)

	Available options for selection parameters:

	thresholdtype(Threshold Type)
		0 - [0] Absolute
		1 - [1] Relative from cell value
