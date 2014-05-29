SINK REMOVAL
============

Description
-----------

Parameters
----------

- ``DEM[Raster]``:
- ``Sink Route[Raster]``:
- ``Method[Selection]``:
- ``Threshold[Boolean]``:
- ``Threshold Height[Number]``:

Outputs
-------

- ``Preprocessed DEM[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:sinkremoval', dem, sinkroute, method, threshold, thrsheight, dem_preproc)

	Available options for selection parameters:

	method(Method)
		0 - [0] Deepen Drainage Routes
		1 - [1] Fill Sinks
