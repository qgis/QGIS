CATCHMENT AREA (RECURSIVE)
==========================

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
- ``Target Areas[Raster]``:
- ``Method[Selection]``:
- ``Convergence[Number]``:

Outputs
-------

- ``Catchment Area[Raster]``:
- ``Catchment Height[Raster]``:
- ``Catchment Slope[Raster]``:
- ``Total accumulated Material[Raster]``:
- ``Accumulated Material from _left_ side[Raster]``:
- ``Accumulated Material from _right_ side[Raster]``:
- ``Flow Path Length[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:catchmentarearecursive', elevation, sinkroute, weight, material, target, step, targets, method, convergence, carea, cheight, cslope, accu_tot, accu_left, accu_right, flowlen)

	Available options for selection parameters:

	method(Method)
		0 - [0] Deterministic 8
		1 - [1] Rho 8
		2 - [2] Deterministic Infinity
		3 - [3] Multiple Flow Direction
