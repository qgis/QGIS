MERGE SHAPES LAYERS
===================

Description
-----------
This algorithm merges two or more input layer into a unique resulting layer. You can merge together only layer of the same
type (polygons with polygons, lines with lines, points with points).
The attribute table of the resulting layer will include only the attributes of the first input layer. Two additional 
columns will be added: one corresponding to the ID of every merged layer and the other one corresponding to the original 
name of the merged layer. 

Parameters
----------

- ``Main Layer[Vector]``: first layer
- ``Additional Layers[MultipleInput]``: layer/s to merge

Outputs
-------

- ``Merged Layer[Vector]``: the resulting layer

See also
---------


Console usage
-------------


::

	processing.runalg('saga:mergeshapeslayers', main, layers, out)
