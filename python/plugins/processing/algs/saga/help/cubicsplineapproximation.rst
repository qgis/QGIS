CUBIC SPLINE APPROXIMATION
==========================

Description
-----------

Parameters
----------

- ``Points[Vector]``:
- ``Attribute[TableField]``:
- ``Target Grid[Selection]``:
- ``Minimal Number of Points[Number]``:
- ``Maximal Number of Points[Number]``:
- ``Points per Square[Number]``:
- ``Tolerance[Number]``:
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

	processing.runalg('saga:cubicsplineapproximation', shapes, field, target, npmin, npmax, nppc, k, output_extent, user_size, user_grid)

	Available options for selection parameters:

	target(Target Grid)
		0 - [0] user defined
