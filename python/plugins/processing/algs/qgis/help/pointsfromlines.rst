POINTS FROM LINES
=================

Description
-----------

This algorithm creates a new point vector layer from input polyline vector and
raster. Each point represents center of pixel along polyline. To determite
pixels Bresenheim algorithms used.

Parameters
----------

- ``Raster layer``: The raster layer to use.
- ``Vector layer``: The vector layer to use.

Outputs
-------

- ``Output layer``: The resulting point layer.

See also
--------


Console usage
-------------


::

  processing.runalg("qgis:pointsfromlines", input_raster, input_vector, output_layer)
