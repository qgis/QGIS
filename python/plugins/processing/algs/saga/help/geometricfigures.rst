GEOMETRIC FIGURES
=================

Description
-----------
This algorithm draws simple geometric figures. It is possible to choose the total amount of the cells and their size of the resulting figure.
The geometric figures available are:
- cone (up), values rise from the center to the borders
- cone (down), values rise from the borders to the center
- plane, values rise from the bottom to the upper part
You can also choose the rotation of the figure (in degrees, clockwise) 

Parameters
----------

- ``Cell Count[Number]``: number of cells 
- ``Cell Size[Number]``: size of the single cell
- ``Figure[Selection]``: type of geometrical figure
- ``Direction of Plane [Degree][Number]``:rotation factor in degrees

Outputs
-------

- ``Result[Raster]``: the resulting grid layer

See also
---------


Console usage
-------------


::

	processing.runalg('saga:geometricfigures', cell_count, cell_size, figure, plane, result)

	Available options for selection parameters:

	figure(Figure)
		0 - [0] Cone (up)
		1 - [1] Cone (down)
		2 - [2] Plane
