LINE DISSOLVE
=============

Description
-----------

Parameters
----------

- ``Lines[Vector]``:
- ``1. Attribute[TableField]``:
- ``2. Attribute[TableField]``:
- ``3. Attribute[TableField]``:
- ``Dissolve...[Selection]``:

Outputs
-------

- ``Dissolved Lines[Vector]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:linedissolve', lines, field_1, field_2, field_3, all, dissolved)

	Available options for selection parameters:

	all(Dissolve...)
		0 - [0] lines with same attribute value(s)
		1 - [1] all lines
