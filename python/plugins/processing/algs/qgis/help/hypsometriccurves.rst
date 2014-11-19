HYPSOMETRIC CURVES
==================

Description
-----------
Calculate hypsometric curves for features of polygon layer and save them as
CSV file for further processing.

Parameters
----------

- ``DEM to analyze [Raster]``: DEM to use for calculating altitudes.
- ``Boundary layer [Vector]``: Polygonal vector layer with boundaries of areas
  used to calculate hypsometric curves.
- ``Step [Number]``: Distanse between curves. Default is 100.0
- ``Use % of area instead of absolute value [Boolean]``: Write area percentage
  to "Area" field of the CSV file instead of absolute area value.

Outputs
-------

- ``Output directory [Directory]``: Directory where output will be saved. For
  each feature from input vector layer CSV file with area and altitude values
  will be created.

  File name consists of prefix "hystogram_" followed by layer name and feature
  ID.

See also
--------


Console usage
-------------

::

  processing.runalg('qgis:hypsometriccurves', path_to_DEM, path_to_vector, step, use_percentage, output_path)
