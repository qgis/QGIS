MULTILEVEL B-SPLINE INTERPOLATION
=================================

Description
-----------

Parameters
----------

- ``Points[Vector]``:
- ``Attribute[TableField]``:
- ``Target Grid[Selection]``:
- ``Method[Selection]``:
- ``Threshold Error[Number]``:
- ``Maximum Level[Number]``:
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

	processing.runalg('saga:multilevelbsplineinterpolation', shapes, field, target, method, epsilon, level_max, output_extent, user_size, user_grid)

	Available options for selection parameters:

	target(Target Grid)
		0 - [0] user defined

	method(Method)
		0 - [0] without B-spline refinement
		1 - [1] with B-spline refinement
