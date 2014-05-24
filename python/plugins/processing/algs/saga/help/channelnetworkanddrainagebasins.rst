CHANNEL NETWORK AND DRAINAGE BASINS
===================================

Description
-----------

Parameters
----------

- ``Elevation[Raster]``:
- ``Threshold[Number]``:

Outputs
-------

- ``Flow Direction[Raster]``:
- ``Flow Connectivity[Raster]``:
- ``Strahler Order[Raster]``:
- ``Drainage Basins[Raster]``:
- ``Channels[Vector]``:
- ``Drainage Basins[Vector]``:
- ``Junctions[Vector]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:channelnetworkanddrainagebasins', dem, threshold, direction, connection, order, basin, segments, basins, nodes)
