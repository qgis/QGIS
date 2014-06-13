CHANGE DATE FORMAT
==================

Description
-----------
This algorithm converts the date format of the input layer and creates a table in output. Be careful at the separation characters between the fields, for example:

- dd.mm.yy means 16.08.07
- yymmdd means 090528



Parameters
----------

- ``Table[Table]``: table or layer attribute table in input
- ``Date Field[TableField]``: field of the table containing the date
- ``Input Format[Selection]``: date format in input (see above for some example)
- ``Output Format[Selection]``: date format in output (see above for some example)

Outputs
-------

- ``Output[Table]``: the resulting table

See also
---------


Console usage
-------------


::

	processing.runalg('saga:changedateformat', table, field, fmt_in, fmt_out, output)

	Available options for selection parameters:

	fmt_in(Input Format)
		0 - [0] dd.mm.yy
		1 - [1] yy.mm.dd
		2 - [2] dd:mm:yy
		3 - [3] yy:mm:dd
		4 - [4] ddmmyyyy, fix size
		5 - [5] yyyymmdd, fix size
		6 - [6] ddmmyy, fix size
		7 - [7] yymmdd, fix size
		8 - [8] Julian Day

	fmt_out(Output Format)
		0 - [0] dd.mm.yy
		1 - [1] yy.mm.dd
		2 - [2] dd:mm:yy
		3 - [3] yy:mm:dd
		4 - [4] ddmmyyyy, fix size
		5 - [5] yyyymmdd, fix size
		6 - [6] ddmmyy, fix size
		7 - [7] yymmdd, fix size
		8 - [8] Julian Day
