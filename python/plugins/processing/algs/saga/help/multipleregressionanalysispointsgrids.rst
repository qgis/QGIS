MULTIPLE REGRESSION ANALYSIS (POINTS/GRIDS)
===========================================

Description
-----------

Parameters
----------

- ``Grids[MultipleInput]``:
- ``Shapes[Vector]``:
- ``Attribute[TableField]``:
- ``Grid Interpolation[Selection]``:
- ``Include X Coordinate[Boolean]``:
- ``Include Y Coordinate[Boolean]``:
- ``Method[Selection]``:
- ``P in[Number]``:
- ``P out[Number]``:

Outputs
-------

- ``Details: Coefficients[Table]``:
- ``Details: Model[Table]``:
- ``Details: Steps[Table]``:
- ``Residuals[Vector]``:
- ``Regression[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:multipleregressionanalysispointsgrids', grids, shapes, attribute, interpol, coord_x, coord_y, method, p_in, p_out, info_coeff, info_model, info_steps, residuals, regression)

	Available options for selection parameters:

	interpol(Grid Interpolation)
		0 - [0] Nearest Neighbor
		1 - [1] Bilinear Interpolation
		2 - [2] Inverse Distance Interpolation
		3 - [3] Bicubic Spline Interpolation
		4 - [4] B-Spline Interpolation

	method(Method)
		0 - [0] include all
		1 - [1] forward
		2 - [2] backward
		3 - [3] stepwise
