MEAN COORDINATE(S)
==================

Description
-----------
This algorithm calculates the mean of the coordinates of a layer starting from a field of the attribute table.

Parameters
----------

- ``Input layer[Vector]``: input layer 
- ``Weight field[TableField]``: field to add if you want to perform a weighted mean
- ``Unique ID field[TableField]``: unique field on which the calculation of the mean will be made


Outputs
-------

- ``Result[Vector]``: the resulting points layer



See also
---------


Console usage
-------------


::

	processing.runalg('qgis:meancoordinates', points, weight, uid, output)
