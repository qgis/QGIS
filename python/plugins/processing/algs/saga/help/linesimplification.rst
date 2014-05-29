LINE SIMPLIFICATION
===================

Description
-----------
This algorithm simplyfies the geometry of a lines layer. You can set the tolerance options: a small number means a tiny
simplification. 

Parameters
----------

- ``Lines[Vector]``: lines layer in input
- ``Tolerance[Number]``: tolerance settings

Outputs
-------

- ``Simplified Lines[Vector]``: the resulting layer

See also
---------


Console usage
-------------


::

	processing.runalg('saga:linesimplification', lines, tolerance, output)
