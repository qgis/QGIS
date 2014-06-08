POINTS FROM POLYGONS
====================

Description
-----------

This algorithm creates a new point vector layer from input polygon vector and
raster. Each point represents center of pixel inside polygon boundaries.

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

  processing.runalg("qgis:pointsfrompolygons", input_raster, input_vector, output_layer)
