ANALYTICAL HILLSHADING
======================

Description
-----------

Parameters
----------

- ``Elevation[Raster]``:
- ``Shading Method[Selection]``:
- ``Azimuth [Degree][Number]``:
- ``Declination [Degree][Number]``:
- ``Exaggeration[Number]``:

Outputs
-------

- ``Analytical Hillshading[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:analyticalhillshading', elevation, method, azimuth, declination, exaggeration, shade)

	Available options for selection parameters:

	method(Shading Method)
		0 - [0] Standard
		1 - [1] Standard (max. 90Degree)
		2 - [2] Combined Shading
		3 - [3] Ray Tracing
