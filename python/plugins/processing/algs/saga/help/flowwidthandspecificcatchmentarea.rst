FLOW WIDTH AND SPECIFIC CATCHMENT AREA
======================================

Description
-----------

Parameters
----------

- ``Elevation[Raster]``:
- ``Total Catchment Area (TCA)[Raster]``:
- ``Method[Selection]``:

Outputs
-------

- ``Flow Width[Raster]``:
- ``Specific Catchment Area (SCA)[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:flowwidthandspecificcatchmentarea', dem, tca, method, width, sca)

	Available options for selection parameters:

	method(Method)
		0 - [0] Deterministic 8
		1 - [1] Multiple Flow Direction (Quinn et al. 1991)
		2 - [2] Aspect
