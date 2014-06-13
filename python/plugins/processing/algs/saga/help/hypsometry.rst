HYPSOMETRY
==========

Description
-----------

Parameters
----------

- ``Elevation[Raster]``:
- ``Number of Classes[Number]``:
- ``Sort[Selection]``:
- ``Classification Constant[Selection]``:
- ``Use Z-Range[Boolean]``:
- ``Z-Range Min[Number]``:
- ``Z-Range Max[Number]``:

Outputs
-------

- ``Hypsometry[Table]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:hypsometry', elevation, count, sorting, method, bzrange, zrange_min, zrange_max, table)

	Available options for selection parameters:

	sorting(Sort)
		0 - [0] up
		1 - [1] down

	method(Classification Constant)
		0 - [0] height
		1 - [1] area
