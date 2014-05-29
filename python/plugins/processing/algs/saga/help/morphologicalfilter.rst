MORPHOLOGICAL FILTER
====================

Description
-----------

Parameters
----------

- ``Grid[Raster]``:
- ``Search Mode[Selection]``:
- ``Radius[Number]``:
- ``Method[Selection]``:

Outputs
-------

- ``Filtered Grid[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:morphologicalfilter', input, mode, radius, method, result)

	Available options for selection parameters:

	mode(Search Mode)
		0 - [0] Square
		1 - [1] Circle

	method(Method)
		0 - [0] Dilation
		1 - [1] Erosion
		2 - [2] Opening
		3 - [3] Closing
