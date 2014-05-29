CLIP POINTS WITH POLYGONS
=========================

Description
-----------

Parameters
----------

- ``Points[Vector]``:
- ``Polygons[Vector]``:
- ``Add Attribute to Clipped Points[TableField]``:
- ``Clipping Options[Selection]``:

Outputs
-------

- ``Clipped Points[Vector]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:clippointswithpolygons', points, polygons, field, method, clips)

	Available options for selection parameters:

	method(Clipping Options)
		0 - [0] one layer for all points
		1 - [1] separate layer for each polygon
