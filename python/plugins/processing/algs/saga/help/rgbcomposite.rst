RGB COMPOSITE
=============

Description
-----------

Parameters
----------

- ``R[Raster]``:
- ``G[Raster]``:
- ``B[Raster]``:
- ``Method for R value[Selection]``:
- ``Method for G value[Selection]``:
- ``Method for B value[Selection]``:

Outputs
-------

- ``Output RGB[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:rgbcomposite', grid_r, grid_g, grid_b, method_r, method_g, method_b, grid_rgb)

	Available options for selection parameters:

	method_r(Method for R value)
		0 - 0-255
		1 - Rescale to 0 - 255

	method_g(Method for G value)
		0 - 0-255
		1 - Rescale to 0 - 255

	method_b(Method for B value)
		0 - 0-255
		1 - Rescale to 0 - 255
