RANDOM FIELD
============

Description
-----------
This algorithm generates a random grid layer. You can set the geographical position (referring to the south-west corner of the grid) and the statistical method used for the calculation of the cell values. 
Two statistical methods are available:
- uniform
- gaussian
Parameters
----------

- ``Width (Cells)[Number]``:x-axis cell number
- ``Height (Cells)[Number]``:y-axis cell number
- ``Cellsize[Number]``: size of the cell (in meters)
- ``West[Number]``:west coordinate of the bottom-left corner of the grid
- ``South[Number]``:south coordinate of the bottom-left corner of the grid
- ``Method[Selection]``:statistical method used for the calculation
- ``Range Min[Number]``:minimum value of the cell (range)
- ``Range Max[Number]``:maximum value of the cell (range)
- ``Arithmetic Mean[Number]``:mean of all the cell values
- ``Standard Deviation[Number]``:standard deviation of all the cell values

Outputs
-------

- ``Random Field[Raster]``: the resulting grid

See also
---------


Console usage
-------------


::

	processing.runalg('saga:randomfield', nx, ny, cellsize, xmin, ymin, method, range_min, range_max, mean, stddev, output)

	Available options for selection parameters:

	method(Method)
		0 - [0] Uniform
		1 - [1] Gaussian
