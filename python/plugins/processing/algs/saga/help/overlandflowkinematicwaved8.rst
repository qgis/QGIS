OVERLAND FLOW - KINEMATIC WAVE D8
=================================

Description
-----------

Parameters
----------

- ``Elevation[Raster]``:
- ``Gauges[Vector]``:
- ``Simulation Time [h][Number]``:
- ``Simulation Time Step [h][Number]``:
- ``Manning's Roughness[Number]``:
- ``Max. Iterations[Number]``:
- ``Epsilon[Number]``:
- ``Precipitation[Selection]``:
- ``Threshold Elevation[Number]``:

Outputs
-------

- ``Runoff[Raster]``:
- ``Flow at Gauges[Table]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:overlandflowkinematicwaved8', dem, gauges, time_span, time_step, roughness, newton_maxiter, newton_epsilon, precip, threshold, flow, gauges_flow)

	Available options for selection parameters:

	precip(Precipitation)
		0 - [0] Homogenous
		1 - [1] Above Elevation
		2 - [2] Left Half
