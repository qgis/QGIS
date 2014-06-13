GRID SHRINK/EXPAND
==================

Description
-----------

Parameters
----------

- ``Grid[Raster]``:
- ``Operation[Selection]``:
- ``Search Mode[Selection]``:
- ``Radius[Number]``:
- ``Method[Selection]``:

Outputs
-------

- ``Result Grid[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:gridshrinkexpand', input, operation, mode, radius, method_expand, result)

	Available options for selection parameters:

	operation(Operation)
		0 - [0] Shrink
		1 - [1] Expand

	mode(Search Mode)
		0 - [0] Square
		1 - [1] Circle

	method_expand(Method)
		0 - [0] min
		1 - [1] max
		2 - [2] mean
		3 - [3] majority
