GRID PROXIMITY BUFFER
=====================

Description
-----------

Parameters
----------

- ``Source Grid[Raster]``:
- ``Buffer distance[Number]``:
- ``Equidistance[Number]``:

Outputs
-------

- ``Distance Grid[Raster]``:
- ``Allocation Grid[Raster]``:
- ``Buffer Grid[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:gridproximitybuffer', source, dist, ival, distance, alloc, buffer)
