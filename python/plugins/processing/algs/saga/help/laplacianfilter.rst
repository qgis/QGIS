LAPLACIAN FILTER
================

Description
-----------

Parameters
----------

- ``Grid[Raster]``:
- ``Method[Selection]``:
- ``Standard Deviation (Percent of Radius)[Number]``:
- ``Radius[Number]``:
- ``Search Mode[Selection]``:

Outputs
-------

- ``Filtered Grid[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:laplacianfilter', input, method, sigma, radius, mode, result)

	Available options for selection parameters:

	method(Method)
		0 - [0] standard kernel 1
		1 - [1] standard kernel 2
		2 - [2] Standard kernel 3
		3 - [3] user defined kernel

	mode(Search Mode)
		0 - [0] square
		1 - [1] circle
