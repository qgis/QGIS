SHAPES TO GRID
==============

Description
-----------

Parameters
----------

- ``Shapes[Vector]``:
- ``Attribute[TableField]``:
- ``Method for Multiple Values[Selection]``:
- ``Method for Lines[Selection]``:
- ``Preferred Target Grid Type[Selection]``:
- ``Target Grid[Selection]``:
- ``Output extent[Extent]``:
- ``Cellsize[Number]``:

Outputs
-------

- ``Grid[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:shapestogrid', input, field, multiple, line_type, grid_type, target, output_extent, user_size, user_grid)

	Available options for selection parameters:

	multiple(Method for Multiple Values)
		0 - [0] first
		1 - [1] last
		2 - [2] minimum
		3 - [3] maximum
		4 - [4] mean

	line_type(Method for Lines)
		0 - [0] thin
		1 - [1] thick

	grid_type(Preferred Target Grid Type)
		0 - [0] Integer (1 byte)
		1 - [1] Integer (2 byte)
		2 - [2] Integer (4 byte)
		3 - [3] Floating Point (4 byte)
		4 - [4] Floating Point (8 byte)

	target(Target Grid)
		0 - [0] user defined
