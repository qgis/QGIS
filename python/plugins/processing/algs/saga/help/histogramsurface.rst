HISTOGRAM SURFACE
=================

Description
-----------

Parameters
----------

- ``Grid[Raster]``:
- ``Method[Selection]``:

Outputs
-------

- ``Histogram[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:histogramsurface', grid, method, hist)

	Available options for selection parameters:

	method(Method)
		0 - [0] rows
		1 - [1] columns
		2 - [2] circle
