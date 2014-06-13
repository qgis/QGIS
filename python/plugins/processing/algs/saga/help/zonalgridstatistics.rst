ZONAL GRID STATISTICS
=====================

Description
-----------

Parameters
----------

- ``Zone Grid[Raster]``:
- ``Categorial Grids[MultipleInput]``:
- ``Grids to analyse[MultipleInput]``:
- ``Aspect[Raster]``:
- ``Short Field Names[Boolean]``:

Outputs
-------

- ``Zonal Statistics[Table]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:zonalgridstatistics', zones, catlist, statlist, aspect, shortnames, outtab)
