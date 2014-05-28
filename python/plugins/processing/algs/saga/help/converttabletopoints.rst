CONVERT TABLE TO POINTS
=======================

Description
-----------
This algorithm converts the attribute table of a layer into a points layer. The resulting layer will be reprojected 
according to the X and Y fields.

Parameters
----------

- ``Table[Table]``: attribute table of the layer in input
- ``X[TableField]``: X coordinate
- ``Y[TableField]``: Y coordinate

Outputs
-------

- ``Points[Vector]``: resulting points layer

See also
---------


Console usage
-------------


::

	processing.runalg('saga:converttabletopoints', table, x, y, points)
