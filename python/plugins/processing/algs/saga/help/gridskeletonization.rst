GRID SKELETONIZATION
====================

Description
-----------

Parameters
----------

- ``Grid[Raster]``:
- ``Method[Selection]``:
- ``Initialisation[Selection]``:
- ``Threshold (Init.)[Number]``:
- ``Convergence[Number]``:

Outputs
-------

- ``Skeleton[Raster]``:
- ``Skeleton[Vector]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:gridskeletonization', input, method, init_method, init_threshold, convergence, result, vector)

	Available options for selection parameters:

	method(Method)
		0 - [0] Standard
		1 - [1] Hilditch's Algorithm
		2 - [2] Channel Skeleton

	init_method(Initialisation)
		0 - [0] Less than
		1 - [1] Greater than
