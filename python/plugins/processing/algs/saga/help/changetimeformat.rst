CHANGE TIME FORMAT
==================

Description
-----------
This algorithm converts the time format of the input layer and creates a table in output. Be careful at the separation characters between the fields, for example:

- hh.mm.ss means 02.04.59
- hhmmss means 044507

Parameters
----------

- ``Table[Table]``: table or layer attribute table in input
- ``Time Field[TableField]``: field of the table containing the time format
- ``Input Format[Selection]``: time format in input (see above for some example)
- ``Output Format[Selection]``: time format in output (see above for some example)

Outputs
-------

- ``Output[Table]``: the resulting table

See also
---------


Console usage
-------------


::

	processing.runalg('saga:changetimeformat', table, field, fmt_in, fmt_out, output)

	Available options for selection parameters:

	fmt_in(Input Format)
		0 - [0] hh.mm.ss
		1 - [1] hh:mm:ss
		2 - [2] hhmmss, fix size
		3 - [3] hours
		4 - [4] minutes
		5 - [5] seconds

	fmt_out(Output Format)
		0 - [0] hh.mm.ss
		1 - [1] hh:mm:ss
		2 - [2] hhmmss, fix size
		3 - [3] hours
		4 - [4] minutes
		5 - [5] seconds
