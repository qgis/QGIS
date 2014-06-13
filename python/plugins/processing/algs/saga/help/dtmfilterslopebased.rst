DTM FILTER (SLOPE-BASED)
========================

Description
-----------

Parameters
----------

- ``Grid to filter[Raster]``:
- ``Search Radius[Number]``:
- ``Approx. Terrain Slope[Number]``:
- ``Use Confidence Interval[Boolean]``:

Outputs
-------

- ``Bare Earth[Raster]``:
- ``Removed Objects[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:dtmfilterslopebased', input, radius, terrainslope, stddev, ground, nonground)
