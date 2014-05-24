CHANGE DETECTION
================

Description
-----------

Parameters
----------

- ``Initial State[Raster]``:
- ``Look-up Table[Table]``:
- ``Value[TableField]``:
- ``Value (Maximum)[TableField]``:
- ``Name[TableField]``:
- ``Final State[Raster]``:
- ``Look-up Table[Table]``:
- ``Value[TableField]``:
- ``Value (Maximum)[TableField]``:
- ``Name[TableField]``:
- ``Report Unchanged Classes[Boolean]``:
- ``Output as...[Selection]``:

Outputs
-------

- ``Changes[Raster]``:
- ``Changes[Table]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:changedetection', initial, ini_lut, ini_lut_min, ini_lut_max, ini_lut_nam, final, fin_lut, fin_lut_min, fin_lut_max, fin_lut_nam, nochange, output, change, changes)

	Available options for selection parameters:

	output(Output as...)
		0 - [0] cells
		1 - [1] percent
		2 - [2] area
