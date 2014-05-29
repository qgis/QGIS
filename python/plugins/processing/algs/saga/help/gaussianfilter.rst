GAUSSIAN FILTER
===============

Description
-----------

Parameters
----------

- ``Grid[Raster]``:
- ``Standard Deviation[Number]``:
- ``Search Mode[Selection]``:
- ``Search Radius[Number]``:

Outputs
-------

- ``Filtered Grid[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:gaussianfilter', input, sigma, mode, radius, result)

	Available options for selection parameters:

	mode(Search Mode)
		0 - [0] Square
		1 - [1] Circle
