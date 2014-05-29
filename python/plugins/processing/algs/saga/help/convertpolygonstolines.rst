CONVERT POLYGONS TO LINES
=========================

Description
-----------
Starting from a polygons layer, this algorithm creates a lines layer of the polygon borders. 


Parameters
----------

- ``Polygons[Vector]``: polygons layer in input

Outputs
-------

- ``Lines[Vector]``: resulting lines layer

See also
---------


Console usage
-------------


::

	processing.runalg('saga:convertpolygonstolines', polygons, lines)
