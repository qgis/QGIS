MODIFED QUADRATIC SHEPARD
=========================

Description
-----------

Parameters
----------

- ``Points[Vector]``:
- ``Attribute[TableField]``:
- ``Target Grid[Selection]``:
- ``Quadratic Neighbors[Number]``:
- ``Weighting Neighbors[Number]``:
- ``Left[Number]``:
- ``Right[Number]``:
- ``Bottom[Number]``:
- ``Top[Number]``:
- ``Cellsize[Number]``:

Outputs
-------

- ``Grid[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:modifedquadraticshepard', shapes, field, target, quadratic_neighbors, weighting_neighbors, user_xmin, user_xmax, user_ymin, user_ymax, user_size, user_grid)

	Available options for selection parameters:

	target(Target Grid)
		0 - [0] user defined
