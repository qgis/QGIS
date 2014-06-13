POLYGON INTERSECTION
====================

Description
-----------

Parameters
----------

- ``Layer A[Vector]``:
- ``Attribute A[TableField]``:
- ``Layer B[Vector]``:
- ``Attribute B[TableField]``:
- ``Method[Selection]``:
- ``Split Parts[Boolean]``:

Outputs
-------

- ``Intersection[Vector]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:polygonintersection', shapes_a, field_a, shapes_b, field_b, method, splitparts, shapes_ab)

	Available options for selection parameters:

	method(Method)
		0 - [0] Complete Intersection
		1 - [1] Intersection
		2 - [2] Difference (A - B)
		3 - [3] Difference (B - A)
