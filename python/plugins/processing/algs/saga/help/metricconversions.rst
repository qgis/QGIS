METRIC CONVERSIONS
==================

Description
-----------
This algorithm offers a quick way to execute numerical conversions of the grid values. 
Conversions units available are:
- radians to degree
- degree to radians
- Celsius to Fahrenheit
- Fahrenheit to Celsius

Parameters
----------

- ``Grid[Raster]``: grid in input
- ``Conversion[Selection]``: conversions parameter

Outputs
-------

- ``Converted Grid[Raster]``: the resulting layer

See also
---------


Console usage
-------------


::

	processing.runalg('saga:metricconversions', grid, conversion, conv)

	Available options for selection parameters:

	conversion(Conversion)
		0 - [0] radians to degree
		1 - [1] degree to radians
		2 - [2] Celsius to Fahrenheit
		3 - [3] Fahrenheit to Celsius
