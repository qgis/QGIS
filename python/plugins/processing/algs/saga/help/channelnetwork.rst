CHANNEL NETWORK
===============

Description
-----------

Parameters
----------

- ``Elevation[Raster]``:
- ``Flow Direction[Raster]``:
- ``Initiation Grid[Raster]``:
- ``Initiation Type[Selection]``:
- ``Initiation Threshold[Number]``:
- ``Divergence[Raster]``:
- ``Tracing: Max. Divergence[Number]``:
- ``Tracing: Weight[Raster]``:
- ``Min. Segment Length[Number]``:

Outputs
-------

- ``Channel Network[Raster]``:
- ``Channel Direction[Raster]``:
- ``Channel Network[Vector]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:channelnetwork', elevation, sinkroute, init_grid, init_method, init_value, div_grid, div_cells, trace_weight, minlen, chnlntwrk, chnlroute, shapes)

	Available options for selection parameters:

	init_method(Initiation Type)
		0 - [0] Less than
		1 - [1] Equals
		2 - [2] Greater than
