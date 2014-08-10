CREATE GRID
===========

Description
-----------
This algorithm creates a grid and adds it to the map canvas. You can decide the extent of the grid, the spacing between the 
lines and the output file type (lines or polygons layer according to the grid type option). Moreover you can choose the
position of the grid (according to the center of the grid) and the corrdinate reference system (CRS).

Parameters
----------

- ``Horizontal spacing[Number]``: x-axes spacing between the lines (meters)
- ``Vertical spacing[Number]``: y-axes spacing between the lines (meters)
- ``Width[Number]``: horizontal extent of the grid
- ``Height[Number]``: vertical extent of the grid
- ``Center X[Number]``: x-coordinate of the grid center
- ``Center Y[Number]``: y-coordinate of the grid center
- ``Grid type[Selection]``: 4 grid types availables
	- Rectangle (line)
	- Rectangle (polygon)
	- Diamond (polygon)
	- Hexagon (polygon)

Outputs
-------

- ``Output[Vector]``: the resulting grid (lines or polygons) layer

See also
---------


Console usage
-------------


::

	processing.runalg('qgis:creategrid', hspacing, vspacing, width, height, centerx, centery, gridtype, savename)

	Available options for selection parameters:

	gridtype(Grid type)
		0 - Rectangle (line)
		1 - Rectangle (polygon)
		2 - Diamond (polygon)
		3 - Hexagon (polygon)
