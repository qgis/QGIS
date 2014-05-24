OVERLAND FLOW DISTANCE TO CHANNEL NETWORK
=========================================

Description
-----------

Parameters
----------

- ``Elevation[Raster]``:
- ``Channel Network[Raster]``:
- ``Flow Algorithm[Selection]``:

Outputs
-------

- ``Overland Flow Distance[Raster]``:
- ``Vertical Overland Flow Distance[Raster]``:
- ``Horizontal Overland Flow Distance[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:overlandflowdistancetochannelnetwork', elevation, channels, method, distance, distvert, disthorz)

	Available options for selection parameters:

	method(Flow Algorithm)
		0 - [0] D8
		1 - [1] MFD
