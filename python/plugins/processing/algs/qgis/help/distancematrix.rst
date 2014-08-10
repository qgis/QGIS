DISTANCE MATRIX
===============

Description
-----------

Parameters
----------

- ``Input point layer[Vector]``:
- ``Input unique ID field[TableField]``:
- ``Target point layer[Vector]``:
- ``Target unique ID field[TableField]``:
- ``Output matrix type[Selection]``:
- ``Use only the nearest (k) target points[Number]``:

Outputs
-------

- ``Distance matrix[File]``:

See also
---------


Console usage
-------------


::

	processing.runalg('qgis:distancematrix', input_layer, input_field, target_layer, target_field, matrix_type, nearest_points, distance_matrix)

	Available options for selection parameters:

	matrix_type(Output matrix type)
		0 - Linear (N*k x 3) distance matrix
		1 - Standard (N x T) distance matrix
		2 - Summary distance matrix (mean, std. dev., min, max)
