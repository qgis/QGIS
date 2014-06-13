ORDINARY KRIGING (GLOBAL)
=========================

Description
-----------

Parameters
----------

- ``Points[Vector]``:
- ``Attribute[TableField]``:
- ``Create Variance Grid[Boolean]``:
- ``Target Grid[Selection]``:
- ``Variogram Model[Selection]``:
- ``Block Kriging[Boolean]``:
- ``Block Size[Number]``:
- ``Logarithmic Transformation[Boolean]``:
- ``Nugget[Number]``:
- ``Sill[Number]``:
- ``Range[Number]``:
- ``Linear Regression[Number]``:
- ``Exponential Regression[Number]``:
- ``Power Function - A[Number]``:
- ``Power Function - B[Number]``:
- ``Grid Size[Number]``:
- ``Fit Extent[Boolean]``:
- ``Output extent[Extent]``:

Outputs
-------

- ``Grid[Raster]``:
- ``Variance[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:ordinarykrigingglobal', shapes, field, bvariance, target, model, block, dblock, blog, nugget, sill, range, lin_b, exp_b, pow_a, pow_b, user_cell_size, user_fit_extent, output_extent, grid, variance)

	Available options for selection parameters:

	target(Target Grid)
		0 - [0] user defined

	model(Variogram Model)
		0 - [0] Spherical Model
		1 - [1] Exponential Model
		2 - [2] Gaussian Model
		3 - [3] Linear Regression
		4 - [4] Exponential Regression
		5 - [5] Power Function Regression
