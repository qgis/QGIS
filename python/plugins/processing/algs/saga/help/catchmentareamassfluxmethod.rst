CATCHMENT AREA (MASS-FLUX METHOD)
=================================

Description
-----------

Parameters
----------

- ``Elevation[Raster]``:
- ``Flow Split Method[Selection]``:
- ``Slope[Boolean]``:
- ``Aspect[Boolean]``:
- ``Flow Accumulation[Boolean]``:
- ``Flow Lines[Boolean]``:

Outputs
-------

- ``Flow Accumulation[Raster]``:
- ``Slope[Raster]``:
- ``Aspect[Raster]``:
- ``Flow Accumulation[Raster]``:
- ``Flow Lines[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:catchmentareamassfluxmethod', dem, method, b_slope, b_aspect, b_area, b_flow, area, g_slope, g_aspect, g_area, g_flow)

	Available options for selection parameters:

	method(Flow Split Method)
		0 - [0] flow width (original)
		1 - [1] cell area
