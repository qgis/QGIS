GAUSSIAN LANDSCAPES
===================

Description
-----------

Parameters
----------

- ``Width (Cells)[Number]``:
- ``Height (Cells)[Number]``:
- ``Roughness/Smoothness[Number]``:
- ``Method[Selection]``:
- ``Flattening[Number]``:

Outputs
-------

- ``Grid[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:gaussianlandscapes', nx, ny, h, method, m, grid)

	Available options for selection parameters:

	method(Method)
		0 - [0] Simple
		1 - [1] Flattening
