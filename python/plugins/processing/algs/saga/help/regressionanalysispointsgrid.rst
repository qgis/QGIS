REGRESSION ANALYSIS
===================

Description
-----------

Parameters
----------

- ``Grid[Raster]``:
- ``Shapes[Vector]``:
- ``Attribute[TableField]``:
- ``Grid Interpolation[Selection]``:
- ``Regression Function[Selection]``:

Outputs
-------

- ``Regression[Raster]``:
- ``Residuals[Vector]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:regressionanalysispointsgrid', grid, shapes, attribute, interpol, method, regression, residual)

	Available options for selection parameters:

	interpol(Grid Interpolation)
		0 - [0] Nearest Neighbor
		1 - [1] Bilinear Interpolation
		2 - [2] Inverse Distance Interpolation
		3 - [3] Bicubic Spline Interpolation
		4 - [4] B-Spline Interpolation

	method(Regression Function)
		0 - [0] Y = a + b * X (linear)
		1 - [1] Y = a + b / X
		2 - [2] Y = a / (b - X)
		3 - [3] Y = a * X^b (power)
		4 - [4] Y = a e^(b * X) (exponential)
		5 - [5] Y = a + b * ln(X) (logarithmic)
