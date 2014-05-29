STATISTICS FOR GRIDS
====================

Description
-----------

Parameters
----------

- ``Grids[MultipleInput]``:

Outputs
-------

- ``Arithmetic Mean[Raster]``:
- ``Minimum[Raster]``:
- ``Maximum[Raster]``:
- ``Variance[Raster]``:
- ``Standard Deviation[Raster]``:
- ``Mean less Standard Deviation[Raster]``:
- ``Mean plus Standard Deviation[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:statisticsforgrids', grids, mean, min, max, var, stddev, stddevlo, stddevhi)
