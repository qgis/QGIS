BURN STREAM NETWORK INTO DEM
============================

Description
-----------

Parameters
----------

- ``DEM[Raster]``:
- ``Streams[Raster]``:
- ``Method[Selection]``:
- ``Epsilon[Number]``:

Outputs
-------

- ``Processed DEM[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:burnstreamnetworkintodem', dem, stream, method, epsilon, burn)

	Available options for selection parameters:

	method(Method)
		0 - [0] simply decrease cell's value by epsilon
		1 - [1] lower cell's value to neighbours minimum value minus epsilon
