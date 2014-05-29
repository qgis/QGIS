POLYGON CENTROIDS
=================

Description
-----------
This algorithm calculates the centroids of a polygons layer. 

Parameters
----------

- ``Polygons[Vector]``: polygons layer
- ``Centroids for each part[Boolean]``: YES if you want to calculate centroids for each part of the layer, NO if you want 
to calculate centroids only for the single features

Outputs
-------

- ``Centroids[Vector]``: the resulting points layer

See also
---------


Console usage
-------------


::

	processing.runalg('saga:polygoncentroids', polygons, method, centroids)
