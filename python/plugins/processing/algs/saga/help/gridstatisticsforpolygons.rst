GRID STATISTICS FOR POLYGONS
============================

Description
-----------

Parameters
----------

- ``Grids[MultipleInput]``:
- ``Polygons[Vector]``:
- ``Number of Cells[Boolean]``:
- ``Minimum[Boolean]``:
- ``Maximum[Boolean]``:
- ``Range[Boolean]``:
- ``Sum[Boolean]``:
- ``Mean[Boolean]``:
- ``Variance[Boolean]``:
- ``Standard Deviation[Boolean]``:
- ``Quantiles[Number]``:

Outputs
-------

- ``Statistics[Vector]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:gridstatisticsforpolygons', grids, polygons, count, min, max, range, sum, mean, var, stddev, quantile, result)
