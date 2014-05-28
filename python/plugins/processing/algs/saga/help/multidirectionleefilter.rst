MULTI DIRECTION LEE FILTER
==========================

Description
-----------

Parameters
----------

- ``Grid[Raster]``:
- ``Estimated Noise (absolute)[Number]``:
- ``Estimated Noise (relative)[Number]``:
- ``Weighted[Boolean]``:
- ``Method[Selection]``:

Outputs
-------

- ``Filtered Grid[Raster]``:
- ``Minimum Standard Deviation[Raster]``:
- ``Direction of Minimum Standard Deviation[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:multidirectionleefilter', input, noise_abs, noise_rel, weighted, method, result, stddev, dir)

	Available options for selection parameters:

	method(Method)
		0 - [0] noise variance given as absolute value
		1 - [1] noise variance given relative to mean standard deviation
		2 - [2] original calculation (Ringeler)
