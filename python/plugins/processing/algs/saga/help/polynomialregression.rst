POLYNOMIAL REGRESSION
=====================

Description
-----------

Parameters
----------

- ``Points[Vector]``:
- ``Attribute[TableField]``:
- ``Polynom[Selection]``:
- ``Maximum X Order[Number]``:
- ``Maximum Y Order[Number]``:
- ``Maximum Total Order[Number]``:
- ``Trend Surface[Selection]``:
- ``Output extent[Extent]``:
- ``Cellsize[Number]``:

Outputs
-------

- ``Residuals[Vector]``:
- ``Grid[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:polynomialregression', points, attribute, polynom, xorder, yorder, torder, target, output_extent, user_size, residuals, user_grid)

	Available options for selection parameters:

	polynom(Polynom)
		0 - [0] simple planar surface
		1 - [1] bi-linear saddle
		2 - [2] quadratic surface
		3 - [3] cubic surface
		4 - [4] user defined

	target(Trend Surface)
		0 - [0] user defined
