REMOVE DUPLICATE POINTS
=======================

Description
-----------

Parameters
----------

- ``Points[Vector]``:
- ``Attribute[TableField]``:
- ``Point to Keep[Selection]``:
- ``Numeric Attribute Values[Selection]``:

Outputs
-------

- ``Result[Vector]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:removeduplicatepoints', points, field, method, numeric, result)

	Available options for selection parameters:

	method(Point to Keep)
		0 - [0] first point
		1 - [1] last point
		2 - [2] point with minimum attribute value
		3 - [3] point with maximum attribute value

	numeric(Numeric Attribute Values)
		0 - [0] take value from the point to be kept
		1 - [1] minimum value of all duplicates
		2 - [2] maximum value of all duplicates
		3 - [3] mean value of all duplicates
