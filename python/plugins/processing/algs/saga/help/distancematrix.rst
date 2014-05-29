DISTANCE MATRIX
===============

Description
-----------
This algorithm generates a distance matrix between each point of the input layer. 
A unique ID will be created in the first row of the resulting matrix (symmetric matrix), while every other cell reflects 
the distance between the points.



Parameters
----------

- ``Points[Vector]``: points layer in input

Outputs
-------

- ``Distance Matrix Table[Table]``: resulting matrix (values in meters)

See also
---------


Console usage
-------------


::

	processing.runalg('saga:distancematrix', points, table)
