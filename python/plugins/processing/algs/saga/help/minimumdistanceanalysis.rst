MINIMUM DISTANCE ANALYSIS
=========================

Description
-----------
This algorithm perform a complete distance analysis of a point layer. It calculates:
- minimum distance of points
- maximum distance of points
- average distance of all the points
- standard deviation of the distance
- duplicated points
You can display the results as a normal attribute table or you can save the table as a csv file (save to file option).

Parameters
----------

- ``Points[Vector]``: points layer in input

Outputs
-------

- ``Minimum Distance Analysis[Table]``: the resulting table with all the statistics

See also
---------


Console usage
-------------


::

	processing.runalg('saga:minimumdistanceanalysis', points, table)
