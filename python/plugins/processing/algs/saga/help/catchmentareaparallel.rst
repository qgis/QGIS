CATCHMENT AREA (PARALLEL)
=========================

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
- ``Linear Flow[Boolean]``:
- ``Linear Flow Threshold[Number]``:
- ``Linear Flow Threshold Grid[Raster]``:
- ``Channel Direction[Raster]``:
- ``Convergence[Number]``:

Outputs
-------

- ``Catchment Area[Raster]``:
- ``Catchment Height[Raster]``:
- ``Catchment Slope[Raster]``:
- ``Total accumulated Material[Raster]``:
- ``Accumulated Material from _left_ side[Raster]``:
- ``Accumulated Material from _right_ side[Raster]``:
- ``Catchment Aspect[Raster]``:
- ``Flow Path Length[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:catchmentareaparallel', elevation, sinkroute, weight, material, target, step, method, dolinear, linearthrs, linearthrs_grid, chdir_grid, convergence, carea, cheight, cslope, accu_tot, accu_left, accu_right, caspect, flwpath)

	Available options for selection parameters:

	method(Method)
		0 - [0] Deterministic 8
		1 - [1] Rho 8
		2 - [2] Braunschweiger Reliefmodell
		3 - [3] Deterministic Infinity
		4 - [4] Multiple Flow Direction
		5 - [5] Multiple Triangular Flow Directon
