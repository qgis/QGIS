THIN PLATE SPLINE (TIN)
=======================

Description
-----------

Parameters
----------

- ``Points[Vector]``:
- ``Attribute[TableField]``:
- ``Target Grid[Selection]``:
- ``Regularisation[Number]``:
- ``Neighbourhood[Selection]``:
- ``Add Frame[Boolean]``:
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

	processing.runalg('saga:thinplatesplinetin', shapes, field, target, regul, level, frame, output_extent, user_size, user_grid)

	Available options for selection parameters:

	target(Target Grid)
		0 - [0] user defined

	level(Neighbourhood)
		0 - [0] immediate
		1 - [1] level 1
		2 - [2] level 2
