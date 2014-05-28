THIN PLATE SPLINE (LOCAL)
=========================

Description
-----------

Parameters
----------

- ``Points[Vector]``:
- ``Attribute[TableField]``:
- ``Target Grid[Selection]``:
- ``Regularisation[Number]``:
- ``Search Radius[Number]``:
- ``Search Mode[Selection]``:
- ``Points Selection[Selection]``:
- ``Maximum Number of Points[Number]``:
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

	processing.runalg('saga:thinplatesplinelocal', shapes, field, target, regul, radius, mode, select, maxpoints, output_extent, user_size, user_grid)

	Available options for selection parameters:

	target(Target Grid)
		0 - [0] user defined

	mode(Search Mode)
		0 - [0] all directions
		1 - [1] quadrants

	select(Points Selection)
		0 - [0] all points in search radius
		1 - [1] maximum number of points
