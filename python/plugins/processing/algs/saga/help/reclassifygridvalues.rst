RECLASSIFY GRID VALUES
======================

Description
-----------

Parameters
----------

- ``Grid[Raster]``:
- ``Method[Selection]``:
- ``old value (for single value change)[Number]``:
- ``new value (for single value change)[Number]``:
- ``operator (for single value change)[Selection]``:
- ``minimum value (for range)[Number]``:
- ``maximum value (for range)[Number]``:
- ``new value(for range)[Number]``:
- ``operator (for range)[Selection]``:
- ``Lookup Table[FixedTable]``:
- ``operator (for table)[Selection]``:
- ``replace no data values[Boolean]``:
- ``new value for no data values[Number]``:
- ``replace other values[Boolean]``:
- ``new value for other values[Number]``:

Outputs
-------

- ``Reclassified Grid[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:reclassifygridvalues', input, method, old, new, soperator, min, max, rnew, roperator, retab, toperator, nodataopt, nodata, otheropt, others, result)

	Available options for selection parameters:

	method(Method)
		0 - [0] single
		1 - [1] range
		2 - [2] simple table

	soperator(operator (for single value change))
		0 - [0] =
		1 - [1] <
		2 - [2] <=
		3 - [3] >=
		4 - [4] >

	roperator(operator (for range))
		0 - [0] <=
		1 - [1] <

	toperator(operator (for table))
		0 - [0] min <= value < max
		1 - [1] min <= value <= max
		2 - [2] min < value <= max
		3 - [3] min < value < max
