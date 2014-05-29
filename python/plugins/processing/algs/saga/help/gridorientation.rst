GRID ORIENTATION
================

Description
-----------

Parameters
----------

- ``Grid[Raster]``:
- ``Method[Selection]``:

Outputs
-------

- ``Changed Grid[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:gridorientation', input, method, result)

	Available options for selection parameters:

	method(Method)
		0 - [0] Copy
		1 - [1] Flip
		2 - [2] Mirror
		3 - [3] Invert
