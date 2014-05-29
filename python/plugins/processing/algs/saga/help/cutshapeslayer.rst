CUT SHAPES LAYER
================

Description
-----------

Parameters
----------

- ``Shapes[MultipleInput]``:
- ``Method[Selection]``:
- ``Extent[Selection]``:
- ``Left[Number]``:
- ``Right[Number]``:
- ``Bottom[Number]``:
- ``Top[Number]``:
- ``Horizontal Range[Number]``:
- ``Vertical Range[Number]``:
- ``Shapes[Vector]``:
- ``Polygons[Vector]``:

Outputs
-------

- ``Cut[Vector]``:
- ``Extent[Vector]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:cutshapeslayer', shapes, method, target, cut_ax, cut_bx, cut_ay, cut_by, cut_dx, cut_dy, shapes_shapes, polygons_polygons, cut, extent)

	Available options for selection parameters:

	method(Method)
		0 - [0] completely contained
		1 - [1] intersects
		2 - [2] center

	target(Extent)
		0 - [0] user defined
		1 - [1] grid project[do not use this option!]
		2 - [2] shapes layer extent
		3 - [3] polygons
