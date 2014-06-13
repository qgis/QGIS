CATCHMENT AREA (FLOW TRACING)
=============================

Description
-----------

Parameters
----------

- ``Elevation[Raster]``:
- ``Sink Routes[Raster]``:
- ``Weight[Raster]``:
- ``Material[Raster]``:
- ``Target[Raster]``:
- ``Step[Number]``:
- ``Method[Selection]``:
- ``DEMON - Min. DQV[Number]``:
- ``Flow Correction[Boolean]``:

Outputs
-------

- ``Catchment Area[Raster]``:
- ``Catchment Height[Raster]``:
- ``Catchment Slope[Raster]``:
- ``Total accumulated Material[Raster]``:
- ``Accumulated Material from _left_ side[Raster]``:
- ``Accumulated Material from _right_ side[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:catchmentareaflowtracing', elevation, sinkroute, weight, material, target, step, method, mindqv, correct, carea, cheight, cslope, accu_tot, accu_left, accu_right)

	Available options for selection parameters:

	method(Method)
		0 - [0] Rho 8
		1 - [1] Kinematic Routing Algorithm
		2 - [2] DEMON
