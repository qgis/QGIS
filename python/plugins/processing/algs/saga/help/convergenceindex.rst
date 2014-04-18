CONVERGENCE INDEX
=================

Description
-----------

Parameters
----------

- ``Elevation[Raster]``:
- ``Method[Selection]``:
- ``Gradient Calculation[Selection]``:

Outputs
-------

- ``Convergence Index[Raster]``:

See also
---------


Console usage
-------------


::

	sextante.runalg('saga:convergenceindex', elevation, method, neighbours, result)

	Available options for selection parameters:

	method(Method)
		0 - [0] Aspect
		1 - [1] Gradient

	neighbours(Gradient Calculation)
		0 - [0] 2 x 2
		1 - [1] 3 x 3
