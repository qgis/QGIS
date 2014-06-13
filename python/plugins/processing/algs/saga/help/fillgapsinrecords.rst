FILL GAPS IN RECORDS
====================

Description
-----------

Parameters
----------

- ``Table[Table]``:
- ``Order[TableField]``:
- ``Interpolation[Selection]``:

Outputs
-------

- ``Table without Gaps[Table]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:fillgapsinrecords', table, order, method, nogaps)

	Available options for selection parameters:

	method(Interpolation)
		0 - [0] Nearest Neighbour
		1 - [1] Linear
		2 - [2] Spline
