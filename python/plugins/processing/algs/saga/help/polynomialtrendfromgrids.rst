POLYNOMIAL TREND FROM GRIDS
===========================

Description
-----------

Parameters
----------

- ``Dependent Variables[MultipleInput]``:
- ``Independent Variable (per Grid and Cell)[MultipleInput]``:
- ``Independent Variable (per Grid)[FixedTable]``:
- ``Type of Approximated Function[Selection]``:

Outputs
-------

- ``Polynomial Coefficients[Raster]``:
- ``Coefficient of Determination[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:polynomialtrendfromgrids', grids, y_grids, y_table, polynom, parms, quality)

	Available options for selection parameters:

	polynom(Type of Approximated Function)
		0 - [0] first order polynom (linear regression)
		1 - [1] second order polynom
		2 - [2] third order polynom
		3 - [3] fourth order polynom
		4 - [4] fifth order polynom
