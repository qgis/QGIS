REPROJECT LAYER
===============

Description
-----------
This algorithm reprojects a vector layer in a different CRS.
Parameters
----------

- ``Input layer[Vector]``: vector to be reprojected
- ``Target CRS[Crs]``: the final CRS of the vector

Outputs
-------

- ``Reprojected layer[Vector]``:resulting vector layer reprojected in the new CRS.

See also
---------


Console usage
-------------


::

	processing.runalg('qgis:reprojectlayer', input, target_crs, output)
