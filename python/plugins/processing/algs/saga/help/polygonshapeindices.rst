POLYGON SHAPE INDICES
=====================

Description
-----------
This algorithm perfmorms some spatial statistics for each feature of a polygons layer. The new values calculated will be 
added to the attribute table of the output layer. Spatial statistics available are:

- area
- perimeter
- perimeter / area 
- perimeter / square root of the area
- maximum distance 
- maximum distance / area
- maximum distance / square root of the area
- shape index

By default, the resulting layer is added in the map canvas as a shapefile, so you can easlily access to the attribute 
table within QGIS. You can also save the results as a csv file and load it later in a text editor. 


Parameters
----------

- ``Shapes[Vector]``: polygons layer 

Outputs
-------

- ``Shape Index[Vector]``: resulting layer

See also
---------


Console usage
-------------


::

	processing.runalg('saga:polygonshapeindices', shapes, index)
