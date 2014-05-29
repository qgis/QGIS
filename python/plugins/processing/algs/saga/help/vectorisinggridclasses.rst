VECTORISING GRID CLASSES
========================

Description
-----------

Parameters
----------

- ``Grid[Raster]``:
- ``Class Selection[Selection]``:
- ``Class Identifier[Number]``:
- ``Vectorised class as...[Selection]``:

Outputs
-------

- ``Polygons[Vector]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:vectorisinggridclasses', grid, class_all, class_id, split, polygons)

	Available options for selection parameters:

	class_all(Class Selection)
		0 - [0] one single class specified by class identifier
		1 - [1] all classes

	split(Vectorised class as...)
		0 - [0] one single (multi-)polygon object
		1 - [1] each island as separated polygon
