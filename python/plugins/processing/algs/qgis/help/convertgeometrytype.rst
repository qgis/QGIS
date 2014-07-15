CONVERT GEOMETRY TYPE
=====================

Description
-----------
With this algorithm you can convert a geometry type to another one. For example you can convert a polygons layer into its centroid
(points layer). 

Parameters
----------

- ``Input layer[Vector]``: layer in input (it can be a points, a lines or a polygons layer)
- ``New Geometry Type[Selection]``: type of conversion to perform

Outputs
-------

- ``Output[Vector]``: the resulting layer

See also
---------


Console usage
-------------


::

	processing.runalg('qgis:convertgeometrytype', layername, newtype, savename)

	Available options for selection parameters:

	newtype(New Geometry Type)
		0 - Centroids
		1 - Nodes
		2 - Linestrings
		3 - Multilinestrings
		4 - Polygons
