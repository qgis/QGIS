CONVERT LINES TO POINTS
=======================

Description
-----------
This algorithm converts the input lines layer into a points layer. 

Parameters
----------

- ``Lines[Vector]``: lines layer in input
- ``Insert Additional Points[Boolean]``: YES adds more points, NO doesn't add more points (just those at the lines vertices) 

- ``Insert Distance[Number]``: distance between the additional points (in meters)

Outputs
-------

- ``Points[Vector]``: resulting points layer

See also
---------


Console usage
-------------


::

	processing.runalg('saga:convertlinestopoints', lines, add, dist, points)
