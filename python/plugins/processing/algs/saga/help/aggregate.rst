AGGREGATE
=========

Description
-----------

Parameters
----------

- ``Grid[Raster]``:
- ``Aggregation Size[Number]``:
- ``Method[Selection]``:

Outputs
-------


See also
---------


Console usage
-------------


::

	processing.runalg('saga:aggregate', input, size, method)

	Available options for selection parameters:

	method(Method)
		0 - [0] Sum
		1 - [1] Min
		2 - [2] Max
