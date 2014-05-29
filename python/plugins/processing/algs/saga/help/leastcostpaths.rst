LEAST COST PATHS
================

Description
-----------

Parameters
----------

- ``Source Point(s)[Vector]``:
- ``Accumulated cost[Raster]``:
- ``Values[MultipleInput]``:

Outputs
-------

- ``Profile (points)[Vector]``:
- ``Profile (lines)[Vector]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:leastcostpaths', source, dem, values, points, line)
