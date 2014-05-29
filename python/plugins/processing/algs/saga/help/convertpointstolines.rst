CONVERT POINTS TO LINE(S)
=========================

Description
-----------
This algorithm transforms a points layer into a lines one.

Parameters
----------

- ``Points[Vector]``: starting points layer
- ``Order by...[TableField]``: lines will be ordered following this field
- ``Separate by...[TableField]``: lines will be grouped according to this field

Outputs
-------

- ``Lines[Vector]``: resulting lines layer

See also
---------


Console usage
-------------


::

	processing.runalg('saga:convertpointstolines', points, order, separate, lines)
