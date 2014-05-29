VEGETATION INDEX[DISTANCE BASED]
================================

Description
-----------

Parameters
----------

- ``Near Infrared Band[Raster]``:
- ``Red Band[Raster]``:
- ``Slope of the soil line[Number]``:
- ``Intercept of the soil line[Number]``:

Outputs
-------

- ``PVI (Richardson and Wiegand)[Raster]``:
- ``PVI (Perry & Lautenschlager)[Raster]``:
- ``PVI (Walther & Shabaani)[Raster]``:
- ``PVI (Qi, et al)[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:vegetationindexdistancebased', nir, red, slope, intercept, pvi, pvi1, pvi2, pvi3)
