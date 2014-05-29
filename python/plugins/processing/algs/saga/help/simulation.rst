SIMULATION
==========

Description
-----------

Parameters
----------

- ``DEM[Raster]``:
- ``Fuel Model[Raster]``:
- ``Wind Speed[Raster]``:
- ``Wind Direction[Raster]``:
- ``Dead Fuel Moisture 1H[Raster]``:
- ``Dead Fuel Moisture 10H[Raster]``:
- ``Dead Fuel Moisture 100H[Raster]``:
- ``Herbaceous Fuel Moisture[Raster]``:
- ``Wood Fuel Moisture[Raster]``:
- ``Ignition Points[Raster]``:
- ``Update View[Boolean]``:

Outputs
-------

- ``Time[Raster]``:
- ``Flame Length[Raster]``:
- ``Intensity[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:simulation', dem, fuel, windspd, winddir, m1h, m10h, m100h, mherb, mwood, ignition, updateview, time, flame, intensity)
