RANDOM TERRAIN GENERATION
=========================

Description
-----------

Parameters
----------

- ``Radius (cells)[Number]``:
- ``Iterations[Number]``:
- ``Target Dimensions[Selection]``:
- ``Grid Size[Number]``:
- ``Cols[Number]``:
- ``Rows[Number]``:

Outputs
-------

- ``Grid[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:randomterraingeneration', radius, iterations, target_type, user_cell_size, user_cols, user_rows, target_grid)

	Available options for selection parameters:

	target_type(Target Dimensions)
		0 - [0] User defined
