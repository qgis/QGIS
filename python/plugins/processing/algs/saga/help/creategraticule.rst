CREATE GRATICULE
================

Description
-----------
This algorithm creates a grid and adds it to the map canvas. You can decide the extent of the grid, the spacing between the
lines and the output file type (lines or polygons layer). 
Leaving the extent options empty means that the bottom left corner of the grid will correspond to the 0,0 point of the CRS 
currently used.

Parameters
----------

- ``Extent[Vector]``: grid will be drawn according to the selected layer 
- ``Output extent[Extent]``: fill this field with the 4 coordinates of the vertices
- ``Division Width[Number]``: x-axes spacing between the lines
- ``Division Height[Number]``: y-axes spacing between the lines
- ``Type[Selection]``: choose the file type (lines or polygons)

Outputs
-------

- ``Graticule[Vector]``: resulting layer

See also
---------


Console usage
-------------


::

	processing.runalg('saga:creategraticule', extent, output_extent, distx, disty, type, graticule)

	Available options for selection parameters:

	type(Type)
		0 - [0] Lines
		1 - [1] Rectangles
