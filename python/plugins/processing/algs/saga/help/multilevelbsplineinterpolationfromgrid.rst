MULTILEVEL B-SPLINE INTERPOLATION (FROM GRID)
=============================================

Description
-----------

Parameters
----------

- ``Grid[Raster]``:
- ``Target Grid[Selection]``:
- ``Method[Selection]``:
- ``Threshold Error[Number]``:
- ``Maximum Level[Number]``:
- ``Data Type[Selection]``:
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

	processing.runalg('saga:multilevelbsplineinterpolationfromgrid', gridpoints, target, method, epsilon, level_max, datatype, output_extent, user_size, user_grid)

	Available options for selection parameters:

	target(Target Grid)
		0 - [0] user defined

	method(Method)
		0 - [0] without B-spline refinement
		1 - [1] with B-spline refinement

	datatype(Data Type)
		0 - [0] same as input grid
		1 - [1] floating point
