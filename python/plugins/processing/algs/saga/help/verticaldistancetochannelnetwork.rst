VERTICAL DISTANCE TO CHANNEL NETWORK
====================================

Description
-----------

Parameters
----------

- ``Elevation[Raster]``:
- ``Channel Network[Raster]``:
- ``Tension Threshold [Percentage of Cell Size][Number]``:
- ``Keep Base Level below Surface[Boolean]``:

Outputs
-------

- ``Vertical Distance to Channel Network[Raster]``:
- ``Channel Network Base Level[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:verticaldistancetochannelnetwork', elevation, channels, threshold, nounderground, distance, baselevel)
