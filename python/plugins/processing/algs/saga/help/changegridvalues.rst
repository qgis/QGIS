CHANGE GRID VALUES
==================

Description
-----------

Parameters
----------

- ``Grid[Raster]``:
- ``Replace Condition[Selection]``:
- ``Lookup Table[FixedTable]``:

Outputs
-------

- ``Changed Grid[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:changegridvalues', grid_in, method, lookup, grid_out)

	Available options for selection parameters:

	method(Replace Condition)
		0 - [0] Grid value equals low value
		1 - [1] Low value < grid value < high value
		2 - [2] Low value <= grid value < high value
