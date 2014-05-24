MAJORITY FILTER
===============

Description
-----------

Parameters
----------

- ``Grid[Raster]``:
- ``Search Mode[Selection]``:
- ``Radius[Number]``:
- ``Threshold [Percent][Number]``:

Outputs
-------

- ``Filtered Grid[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:majorityfilter', input, mode, radius, threshold, result)

	Available options for selection parameters:

	mode(Search Mode)
		0 - [0] Square
		1 - [1] Circle
