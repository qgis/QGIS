GRID VOLUME
===========

Description
-----------

Parameters
----------

- ``Grid[Raster]``:
- ``Method[Selection]``:
- ``Base Level[Number]``:

Outputs
-------


See also
---------


Console usage
-------------


::

	processing.runalg('saga:gridvolume', grid, method, level)

	Available options for selection parameters:

	method(Method)
		0 - [0] Count Only Above Base Level
		1 - [1] Count Only Below Base Level
		2 - [2] Subtract Volumes Below Base Level
		3 - [3] Add Volumes Below Base Level
