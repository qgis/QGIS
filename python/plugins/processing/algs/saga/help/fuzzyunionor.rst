FUZZY UNION (OR)
================

Description
-----------

Parameters
----------

- ``Grids[MultipleInput]``:
- ``Operator Type[Selection]``:

Outputs
-------

- ``Union[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:fuzzyunionor', grids, type, or)

	Available options for selection parameters:

	type(Operator Type)
		0 - [0] max(a, b) (non-interactive)
		1 - [1] a + b - a * b
		2 - [2] min(1, a + b)
