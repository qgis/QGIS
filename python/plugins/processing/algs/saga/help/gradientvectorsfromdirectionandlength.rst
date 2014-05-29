GRADIENT VECTORS FROM DIRECTION AND LENGTH
==========================================

Description
-----------

Parameters
----------

- ``Direction[Raster]``:
- ``Length[Raster]``:
- ``Step[Number]``:
- ``Size Range Min[Number]``:
- ``Size Range Max[Number]``:
- ``Aggregation[Selection]``:
- ``Style[Selection]``:

Outputs
-------

- ``Gradient Vectors[Vector]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:gradientvectorsfromdirectionandlength', dir, len, step, size_min, size_max, aggr, style, vectors)

	Available options for selection parameters:

	aggr(Aggregation)
		0 - [0] nearest neighbour
		1 - [1] mean value

	style(Style)
		0 - [0] simple line
		1 - [1] arrow
		2 - [2] arrow (centered to cell)
