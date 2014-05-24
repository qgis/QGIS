CELL BALANCE
============

Description
-----------

Parameters
----------

- ``Elevation[Raster]``:
- ``Parameter[Raster]``:
- ``Default Weight[Number]``:
- ``Method[Selection]``:

Outputs
-------

- ``Cell Balance[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:cellbalance', dem, weights, weight, method, balance)

	Available options for selection parameters:

	method(Method)
		0 - [0] Deterministic 8
		1 - [1] Multiple Flow Direction
