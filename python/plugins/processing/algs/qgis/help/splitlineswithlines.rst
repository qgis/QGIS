SPLIT LINES WITH LINES
=======================

Description
-----------
This algorithm splits the features of a line layer with the lines of another line layer. 

Parameters
----------

- ``Input layer[Vector]``:line layer
- ``Split layer[Vector]``:line layer

Outputs
-------

- ``Output layer[Vector]``: resulting layer

See also
---------


Console usage
-------------


::

	processing.runalg('qgis:splitlineswithlines', input_lines, split_lines, output)
