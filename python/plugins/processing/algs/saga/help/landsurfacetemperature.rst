LAND SURFACE TEMPERATURE
========================

Description
-----------

Parameters
----------

- ``Elevation [m][Raster]``:
- ``Short Wave Radiation [kW/m2][Raster]``:
- ``Leaf Area Index[Raster]``:
- ``Elevation at Reference Station [m][Number]``:
- ``Temperature at Reference Station [Deg.Celsius][Number]``:
- ``Temperature Gradient [Deg.Celsius/km][Number]``:
- ``C Factor[Number]``:

Outputs
-------

- ``Land Surface Temperature [Deg.Celsius][Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:landsurfacetemperature', dem, swr, lai, z_reference, t_reference, t_gradient, c_factor, lst)
