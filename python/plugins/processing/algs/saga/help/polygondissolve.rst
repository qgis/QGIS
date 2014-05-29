POLYGON DISSOLVE
================

Description
-----------

Parameters
----------

- ``Polygons[Vector]``:
- ``1. Attribute[TableField]``:
- ``2. Attribute[TableField]``:
- ``3. Attribute[TableField]``:
- ``Dissolve...[Selection]``:

Outputs
-------

- ``Dissolved Polygons[Vector]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:polygondissolve', polygons, field_1, field_2, field_3, dissolve, dissolved)

	Available options for selection parameters:

	dissolve(Dissolve...)
		0 - [0] polygons with same attribute value
		1 - [1] all polygons
		2 - [2] polygons with same attribute value (keep inner boundaries)
		3 - [3] all polygons (keep inner boundaries)
