POINTS DISPLACEMENT
===================

Description
-----------

This algorithm creates a new vector layer from input point vector, where points
with same coordinates placed around original position, so all points becomes
visible.

Parameters
----------

- ``Input layer``: The vector layer to use.
- ``Displacement distance``: Displacement distance. **IMPORTANT**! It is
  necessary to use for distance same units as input shapefile used. For example,
  if shapefile in decimal degrees than distance also should be in decimal
  degrees.
- ``Horizontal distribution for two point case``: If set to ``Yes`` then two
  overlapped points will be shifted in horizontal direscion, otherwise —-- in
  vertical direction. Default value is ``Yes``, so points will be shifted
  horizontally.

Outputs
-------

- ``Output layer``: The resulting layer, with shifted overlapped features.

See also
--------


Console usage
-------------


::

  processing.runalg("qgis:pointsdisplacement", input_layer, distance, horizontal_distr, output_layer)
