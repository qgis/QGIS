GET SHAPES EXTENTS
==================

Description
-----------
This algorithm draws new polygons according to the extent of the layer features. You can choose to draw either a new 
polygon for each features or just a single polygon of the total extent (in this case the input layer has to be a 
multipart layer).

Parameters
----------

- ``Shapes[Vector]``: input layer
- ``Parts[Boolean]``: YES draws a polygon for each feature, NO draws only one polygon 

Outputs
-------

- ``Extents[Vector]``: the resulting layer

See also
---------


Console usage
-------------


::

	processing.runalg('saga:getshapesextents', shapes, parts, extents)
