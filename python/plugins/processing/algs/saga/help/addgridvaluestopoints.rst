ADD GRID VALUES TO POINTS
=========================

Description
-----------
This algorithm creates a new vector layer as a result of the union of a points layer with the interpolated value of one or more base background grid layer/s. This way, the new layer created will have a new column in the attribute table that reflects the interpolated value of the background grid. 
There are several interpolation methods available:
- nearest neighbor 
- bilinear interpolation
- inverse distance interpolation
- bicubic spline interpolation
- b-spline interpolation
Parameters
----------

- ``Points[Vector]``: points layer in input
- ``Grids[MultipleInput]``: background grid layer (it possible to choose multiple grids)
- ``Interpolation[Selection]``: interpolation method 

Outputs
-------

- ``Result[Vector]``: the resulting vector

See also
---------


Console usage
-------------


::

	processing.runalg('saga:addgridvaluestopoints', shapes, grids, interpol, result)

	Available options for selection parameters:

	interpol(Interpolation)
		0 - [0] Nearest Neighbor
		1 - [1] Bilinear Interpolation
		2 - [2] Inverse Distance Interpolation
		3 - [3] Bicubic Spline Interpolation
		4 - [4] B-Spline Interpolation
