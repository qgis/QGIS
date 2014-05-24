FIRE RISK ANALYSIS
==================

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
- ``Value[Raster]``:
- ``Base Probability[Raster]``:
- ``Number of Events[Number]``:
- ``Fire Length[Number]``:

Outputs
-------

- ``Danger[Raster]``:
- ``Compound Probability[Raster]``:
- ``Priority Index[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:fireriskanalysis', dem, fuel, windspd, winddir, m1h, m10h, m100h, mherb, mwood, value, baseprob, montecarlo, interval, danger, compprob, priority)
