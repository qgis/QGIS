LS FACTOR
=========

Description
-----------

Parameters
----------

- ``Slope[Raster]``:
- ``Catchment Area[Raster]``:
- ``Area to Length Conversion[Selection]``:
- ``Method (LS)[Selection]``:
- ``Rill/Interrill Erosivity[Number]``:
- ``Stability[Selection]``:

Outputs
-------

- ``LS Factor[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:lsfactor', slope, area, conv, method, erosivity, stability, ls)

	Available options for selection parameters:

	conv(Area to Length Conversion)
		0 - [0] no conversion (areas already given as specific catchment area)
		1 - [1] 1 / cell size (specific catchment area)
		2 - [2] square root (catchment length)

	method(Method (LS))
		0 - [0] Moore et al. 1991
		1 - [1] Desmet & Govers 1996
		2 - [2] Boehner & Selige 2006

	stability(Stability)
		0 - [0] stable
		1 - [1] instable (thawing)
