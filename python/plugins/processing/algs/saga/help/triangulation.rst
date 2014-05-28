TRIANGULATION
=============

Description
-----------

Parameters
----------

- ``Points[Vector]``:
- ``Attribute[TableField]``:
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

	processing.runalg('saga:triangulation', shapes, field, target, output_extent, user_size, user_grid)

	Available options for selection parameters:

	target(Target Grid)
		0 - [0] user defined
