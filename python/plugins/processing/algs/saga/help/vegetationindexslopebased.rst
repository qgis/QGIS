VEGETATION INDEX[SLOPE BASED]
=============================

Description
-----------

Parameters
----------

- ``Near Infrared Band[Raster]``:
- ``Red Band[Raster]``:

Outputs
-------

- ``Normalized Difference Vegetation Index[Raster]``:
- ``Ratio Vegetation Index[Raster]``:
- ``Transformed Vegetation Index[Raster]``:
- ``Corrected Transformed Vegetation Index[Raster]``:
- ``Thiam's Transformed Vegetation Index[Raster]``:
- ``Normalized Ratio Vegetation Index[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:vegetationindexslopebased', nir, red, ndvi, ratio, tvi, ctvi, ttvi, nratio)
