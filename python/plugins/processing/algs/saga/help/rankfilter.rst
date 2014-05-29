RANK FILTER
===========

Description
-----------

Parameters
----------

- ``Grid[Raster]``:
- ``Search Mode[Selection]``:
- ``Radius[Number]``:
- ``Rank [Percent][Number]``:

Outputs
-------

- ``Filtered Grid[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:rankfilter', input, mode, radius, rank, result)

	Available options for selection parameters:

	mode(Search Mode)
		0 - [0] Square
		1 - [1] Circle
