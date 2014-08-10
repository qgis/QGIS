SAVE SELECTED FEATURES
======================

Description
-----------
This algorithm saves the selected features as a new layer. It is possible to assign specific CRS to the new layer.
Parameters
----------

- ``Input layer[Vector]``: the layer with selected features.

Outputs
-------

- ``Output layer with selected features[Vector]``: resulting layer

See also
---------


Console usage
-------------


::

	processing.runalg('qgis:saveselectedfeatures', input_layer, output_layer)
