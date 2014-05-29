SIMPLE FILTER
=============

Description
-----------

Parameters
----------

- ``Grid[Raster]``:
- ``Search Mode[Selection]``:
- ``Filter[Selection]``:
- ``Radius[Number]``:

Outputs
-------

- ``Filtered Grid[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:simplefilter', input, mode, method, radius, result)

	Available options for selection parameters:

	mode(Search Mode)
		0 - [0] Square
		1 - [1] Circle

	method(Filter)
		0 - [0] Smooth
		1 - [1] Sharpen
		2 - [2] Edge
