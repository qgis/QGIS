GRID VALUES TO POINTS
=====================

Description
-----------

Parameters
----------

- ``Grids[MultipleInput]``:
- ``Polygons[Vector]``:
- ``Exclude NoData Cells[Boolean]``:
- ``Type[Selection]``:

Outputs
-------

- ``Shapes[Vector]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:gridvaluestopoints', grids, polygons, nodata, type, shapes)

	Available options for selection parameters:

	type(Type)
		0 - [0] nodes
		1 - [1] cells
