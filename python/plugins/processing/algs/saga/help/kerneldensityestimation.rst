KERNEL DENSITY ESTIMATION
=========================

Description
-----------

Parameters
----------

- ``Points[Vector]``:
- ``Weight[TableField]``:
- ``Radius[Number]``:
- ``Kernel[Selection]``:
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

	processing.runalg('saga:kerneldensityestimation', points, population, radius, kernel, target, output_extent, user_size, user_grid)

	Available options for selection parameters:

	kernel(Kernel)
		0 - [0] quartic kernel
		1 - [1] gaussian kernel

	target(Target Grid)
		0 - [0] user defined
